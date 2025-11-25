// UsbSession.cpp
#include <service/usbSession.h>
#include <QDebug>
#include <QTimer>

UsbSession::UsbSession(const UsbTuple & tub,QObject* parent)
    : ISession(parent)
{    
    // 初始化 libusb
    if (libusb_init(nullptr) < 0) {
        qCritical() << "Failed to initialize libusb";
    }

}

UsbSession::~UsbSession() {
    if (handle) {
        libusb_release_interface(handle, 0);
        libusb_close(handle);
        libusb_exit(nullptr);
        handle = nullptr;
        qDebug() << "Device closed";
    }

    emit statusChanged(ISession::UnLink);
}

bool UsbSession::link()
{
    // 打开设备
    handle = libusb_open_device_with_vid_pid(nullptr, tuple.vendor_id, tuple.product_id);
    if (!handle) {
        qCritical() << "Device not found" << QString("VID:%1 PID:%2").arg(tuple.vendor_id, 0, 16).arg(tuple.product_id, 0, 16);
        libusb_exit(nullptr);
        return false;
    }

    qDebug() << "Device opened successfully";

    // 声明接口（通常为 0）                      // 需要监测并指定链接的接口
    int err = libusb_claim_interface(handle, tuple.interface_number);        // 独占接口
    if (err != 0) {
        qCritical() << "Cannot claim interface:" << libusb_error_name(err);
        libusb_close(handle);
        libusb_exit(nullptr);
        return false;
    }

    owner = true;

    emit statusChanged(ISession::Linked);
    qDebug() << "Interface claimed";

    // 在几秒后执行某个对象的某个函数
    QTimer::singleShot(0, this, &UsbSession::recvLoop);

    return true;


    /*精确匹配设备
bool UsbDevice::open()
{
    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(m_context, &list);
    if (cnt < 0) {
        setError("libusb_get_device_list", (int)cnt);
        return false;
    }

    libusb_device *target_dev = nullptr;

    for (ssize_t i = 0; i < cnt; ++i) {
        libusb_device *dev = list[i];
        libusb_device_descriptor desc;

        // 获取设备描述符
        int result = libusb_get_device_descriptor(dev, &desc);
        if (result < 0) continue;

        // 1. 先匹配 VID/PID
        if (desc.idVendor != m_config.vendor_id ||
            desc.idProduct != m_config.product_id) {
            continue;
        }

        // 2. 检查是否需要更精确匹配
        bool match = true;

        if (!m_config.serial_number.isEmpty()) {
            libusb_device_handle *temp_handle;
            if (libusb_open(dev, &temp_handle) == 0) {
                char serial[256] = {0};
                libusb_get_string_descriptor_ascii(temp_handle, desc.iSerialNumber,
                                                  (unsigned char*)serial, sizeof(serial));
                if (QString(serial) != m_config.serial_number) {
                    match = false;
                }
                libusb_close(temp_handle);
            } else {
                match = false;
            }
        }

        // 3. 匹配总线号/设备地址
        if (m_config.bus_number >= 0) {
            if (libusb_get_bus_number(dev) != m_config.bus_number) {
                match = false;
            }
        }
        if (m_config.device_address >= 0) {
            if (libusb_get_device_address(dev) != m_config.device_address) {
                match = false;
            }
        }

        // 4. 匹配端口路径（最精确！）
        if (!m_config.port_numbers.empty()) {
            uint8_t ports[8];
            int port_count = libusb_get_port_numbers(dev, ports, 8);
            if (port_count != (int)m_config.port_numbers.size()) {
                match = false;
            } else {
                for (int j = 0; j < port_count; ++j) {
                    if (ports[j] != m_config.port_numbers[j]) {
                        match = false;
                        break;
                    }
                }
            }
        }

        if (match) {
            target_dev = dev;
            break;
        }
    }

    libusb_free_device_list(list, 1);

    if (!target_dev) {
        emit errorOccurred("未找到匹配的设备");
        return false;
    }

    // 打开目标设备
    int result = libusb_open(target_dev, &m_handle);
    if (result < 0) {
        setError("libusb_open", result);
        return false;
    }

    // 继续设置配置、claim 接口...
    // （后续代码同前）
    return true;
}
     */
}

bool UsbSession::unlink()
{
    if (handle) {
        if(owner)
        {
            owner = false;
            libusb_release_interface(handle, 0);
        }

        libusb_close(handle);
        libusb_exit(nullptr);
        handle = nullptr;
        qDebug() << "Device closed";
    }

    emit statusChanged(ISession::UnLink);

    return true;
}

bool UsbSession::send(const QByteArray& data)
{
    int timeout = 100;
    int transferred;
    int err = libusb_interrupt_transfer(
        handle,
        tuple.endpoint_out,       // 端点地址，如 0x01 (OUT)
        (uint8_t*)data.constData(),
        data.size(),
        &transferred,
        timeout
        );

    return err >= 0;

    qDebug() << "UsbService: Sending data:" << data.toHex();
    // 实际发送逻辑（如 libusb_bulk_transfer）
    return true;
}

void UsbSession::recvLoop()
{

    if (!handle && !owner)
    {
        emit statusChanged(SessionStatus::UnLink);
        return;
    }

    const int BUFFER_SIZE = 64;
    uint8_t buffer[BUFFER_SIZE];
    int actual_length = 0;      // 接收长度

    // 阻塞读取，超时 100ms
    int result = libusb_interrupt_transfer(
        handle,
        tuple.endpoint_in,           // 端点地址（如 0x81）
        buffer,
        BUFFER_SIZE,
        &actual_length,
        100                   // 超时（ms）
        );

    if (result == 0) {
        // 成功收到数据！发出信号
        QByteArray data((char*)buffer, actual_length);
        emit received(data);
    }
    else if (result != LIBUSB_ERROR_TIMEOUT) {
        qDebug() << QString("USB 错误: %1").arg(libusb_error_name(result));
        emit errorOccurred(libusb_error_name(result));
    }


    // 继续下一次读取（非阻塞调度）
    if (handle && owner)  QTimer::singleShot(1, this, &UsbSession::recvLoop);

}

// int UsbService::write(const QByteArray &data, int timeout)
// {
//     if (!owner) return LIBUSB_ERROR_NO_DEVICE;

//     switch ((libusb_transfer_type)tuple.transfer_type) {
//     case LIBUSB_TRANSFER_TYPE_CONTROL:
//         return controlWrite(data, timeout);

//     case LIBUSB_TRANSFER_TYPE_BULK:
//         return bulkWrite(data, timeout);

//     case LIBUSB_TRANSFER_TYPE_INTERRUPT:
//         return interruptWrite(data, timeout);

//     case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
//         return isoWrite(data, timeout);

//     default:
//         emit errorOccurred(localErr);
//         return LIBUSB_ERROR_INVALID_PARAM;
//     }
// }

// int UsbService::read(QByteArray &data, int length, int timeout)
// {
//     if (!owner) return LIBUSB_ERROR_NO_DEVICE;

//     // 如果 length < 0，使用默认缓冲区大小（如 512）
//     if (length < 0) length = 512;

//     switch ((libusb_transfer_type)tuple.transfer_type) {
//     case LIBUSB_TRANSFER_TYPE_CONTROL:
//         return controlRead(data, length, timeout);

//     case LIBUSB_TRANSFER_TYPE_BULK:
//         return bulkRead(data, timeout);

//     case LIBUSB_TRANSFER_TYPE_INTERRUPT:
//         return interruptRead(data, timeout);

//     case LIBUSB_TRANSFER_TYPE_ISOCHRONOUS:
//         return isoRead(data, timeout);

//     default:
//         emit errorOccurred(localErr);
//         return LIBUSB_ERROR_INVALID_PARAM;
//     }
// }
