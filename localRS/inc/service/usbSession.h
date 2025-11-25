// UsbService.h
#pragma once
#include <interface/ISession.h>
#include <core/connection.h>
#include <libusb/libusb.h>

// [[deprecated("该类未开发完备请勿使用")]]
/*!
 *  \fn UsbSession
    \deprecated 该类未开发完备请勿使用
*/
class UsbSession : public ISession
{
    Q_OBJECT

public:
    explicit UsbSession(const UsbTuple & tup,QObject* parent = nullptr);
    ~ UsbSession();// = default;

    LinkType linkType() const override { return LinkType::USB; }

    bool link() override;
    bool unlink() override;
    bool send(const QByteArray& data) override;

private slots:
    void recvLoop();

// private:
//     int write(const QByteArray &data, int timeout = 100);
//     int read(QByteArray &data, int length = -1, int timeout = 100);

//     int controlRead(QByteArray &data, int length, int timeout);
//     int bulkRead(QByteArray &data, int timeout);
//     int interruptRead(QByteArray &data, int timeout);
//     int isoRead(QByteArray &data, int timeout);

//     int controlWrite(const QByteArray &data, int timeout);
//     int bulkWrite(const QByteArray &data, int timeout);
//     int interruptWrite(const QByteArray &data, int timeout);
//     int isoWrite(const QByteArray &data, int timeout);
private:
    bool owner = false;
    libusb_device_handle * handle;
    UsbTuple tuple;
};


/*  区分是否为usb转serial设备
    // 常见 USB 转串口芯片的 VID/PID 和 Class
    芯片                       VID     PID     bInterfaceClass             说明
    CP210x (Silicon Labs)	0x10C4	0xEA60      0xFF (Vendor Specific)	常见于开发板
    CH340/CH341 (WCH)        0x1A86	0x7523      0xFF                    国产，便宜
    FTDI FT232               0x0403	0x6001      0xFF                    高质量，驱动稳定
    Prolific PL2303          0x067B	0x2303      0xFF                    较老
    CDC-ACM (标准)             任意	任意        0x02 (Communications)	标准虚拟串口
*/

/* libusb使用示例
#include <libusb-1.0/libusb.h>

class UsbDevice {
    libusb_device_handle* handle = nullptr;

public:
    bool open(uint16_t vid, uint16_t pid) {
        // 初始化 libusb
        if (libusb_init(nullptr) < 0) {
            qCritical() << "Failed to initialize libusb";
            return false;
        }

        // 打开设备
        handle = libusb_open_device_with_vid_pid(nullptr, vid, pid);
        if (!handle) {
            qCritical() << "Device not found" << QString("VID:%1 PID:%2").arg(vid, 0, 16).arg(pid, 0, 16);
            libusb_exit(nullptr);
            return false;
        }

        qDebug() << "Device opened successfully";

        // 声明接口（通常为 0）
        int interface = 0;
        int err = libusb_claim_interface(handle, interface);
        if (err != 0) {
            qCritical() << "Cannot claim interface:" << libusb_error_name(err);
            libusb_close(handle);
            libusb_exit(nullptr);
            return false;
        }

        qDebug() << "Interface claimed";
        return true;
    }

    // 发送控制传输（示例：设置请求）
    bool controlWrite(uint8_t requestType, uint8_t request, uint16_t value, uint16_t index, const uint8_t* data, uint16_t length) {
        int transferred;
        int err = libusb_control_transfer(
            handle,
            requestType,    // bmRequestType
            request,        // bRequest
            value,          // wValue
            index,          // wIndex
            (uint8_t*)data, // data
            length,         // wLength
            1000            // timeout (ms)
            );
        return err >= 0;
    }

    // 从中断端点读取数据
    bool readInterrupt(uint8_t endpoint, uint8_t* data, int length, int timeout = 1000) {
        int transferred;
        int err = libusb_interrupt_transfer(
            handle,
            endpoint,       // 端点地址，如 0x81 (IN)
            data,
            length,
            &transferred,
            timeout
            );
        if (err == 0) {
            qDebug() << "Read" << transferred << "bytes";
            return true;
        } else {
            qWarning() << "Read error:" << libusb_error_name(err);
            return false;
        }
    }

    // 向中断端点写入数据
    bool writeInterrupt(uint8_t endpoint, const uint8_t* data, int length, int timeout = 1000) {
        int transferred;
        int err = libusb_interrupt_transfer(
            handle,
            endpoint,       // 端点地址，如 0x01 (OUT)
            (uint8_t*)data,
            length,
            &transferred,
            timeout
            );
        return err == 0;
    }

    // 批量传输（高速数据）
    bool readBulk(uint8_t endpoint, uint8_t* data, int length, int timeout = 1000) {
        int transferred;
        return libusb_bulk_transfer(handle, endpoint, data, length, &transferred, timeout) == 0;
    }

    void close() {
        if (handle) {
            libusb_release_interface(handle, 0);
            libusb_close(handle);
            libusb_exit(nullptr);
            handle = nullptr;
            qDebug() << "Device closed";
        }
    }

    ~UsbDevice() {
        close();
    }
};
*/

/*  libusb自实现的检测事件循环
// 1. 分配 transfer 结构
struct libusb_transfer *transfer = libusb_alloc_transfer(0);
unsigned char *buffer = (unsigned char*)malloc(64);

// 2. 填写任务单（指定谁、从哪、到哪、完成后找谁）
libusb_fill_interrupt_transfer(
    transfer,
    handle,         // 设备
    0x81,           // IN 端点
    buffer, 64,     // 缓冲区
    on_data_ready,  // 回调函数
    this,           // 回调时传回 this
    1000            // 超时
);

// 3. 提交任务（交给操作系统）
int result = libusb_submit_transfer(transfer);
if (result != 0) {
    // 提交失败
}

// 4. 事件循环（在另一个线程或主循环中）
//    当数据到达时，on_data_ready 会被自动调用
libusb_handle_events(ctx);

// 在独立线程中运行 或添加到可靠的事件循环中（QGui）
void event_loop() {
    while (running) {
        libusb_handle_events(ctx);  // 阻塞在这里，直到有事件
    }
}

✅ 回调函数 on_data_ready 会被这样调用
void on_data_ready(struct libusb_transfer *t) {
    switch (t->status) {
        case LIBUSB_TRANSFER_COMPLETED:
            printf("收到 %d 字节: %s\n", t->actual_length, t->buffer);
            // ✅ 重要：如果你想持续接收，重新提交
            libusb_submit_transfer(t);
            break;
        case LIBUSB_TRANSFER_TIMED_OUT:
            printf("超时\n");
            break;
        default:
            printf("错误: %s\n", libusb_error_name(t->status));
    }
}
 */
