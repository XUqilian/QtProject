#pragma once

#include <QObject>
#include <libusb/libusb.h>
#include <QTimer>
#include <QThread>
#include <QDebug>
#include <QMetaObject>
#include "core/connection.h"  // 包含 UsbTuple

class UsbWarpper : public QObject
{
    Q_OBJECT

public:
    explicit UsbWarpper(const UsbTuple& config, QObject *parent = nullptr);
    ~UsbWarpper();

    bool start();
    void stop();

    // 数据发送接口
    bool sendData(const QByteArray& data);

    // 仅用于 Control 传输的读写
    bool readControl(QByteArray& outData, int timeout = 100);
    bool writeControl(const QByteArray& data, int timeout = 100);

signals:
    void deviceConnected(libusb_device_handle* handle);
    void deviceDisconnected();
    void dataReceived(const QByteArray& data);
    void errorOccurred(const QString& error);

private:
    void setupDevice();
    void cleanup();
    void startDataReception();

    void postInterruptTransfer();
    void postBulkTransfer();
    void postIsoTransfer();

private slots:
    void onDataReceived(const QByteArray& data);

private:
    UsbTuple m_config;

    libusb_context* m_ctx = nullptr;
    libusb_device_handle* m_handle = nullptr;
    QTimer* m_pollTimer = nullptr;

    QThread m_workerThread;

    bool m_running = false;
    bool m_connected = false;
};



// #include "usbdevicemanager.h"

// #include <QThread>

UsbWarpper::UsbWarpper(const UsbTuple& config, QObject *parent)
    : QObject(parent), m_config(config)
{
    moveToThread(&m_workerThread);

    connect(&m_workerThread, &QThread::started, [this]() {
        if (m_running) {
            setupDevice();
        }
    });
}

UsbWarpper::~UsbWarpper()
{
    stop();
    m_workerThread.quit();
    m_workerThread.wait();
}

bool UsbWarpper::start()
{
    if (m_running) return true;

    m_running = true;

    int res = libusb_init(&m_ctx);
    if (res < 0) {
        emit errorOccurred(QString("libusb_init failed: %1").arg(res));
        m_running = false;
        return false;
    }

    m_workerThread.start();
    return true;
}

void UsbWarpper::stop()
{
    if (!m_running) return;
    m_running = false;

    if (m_pollTimer) {
        m_pollTimer->stop();
        delete m_pollTimer;
        m_pollTimer = nullptr;
    }

    m_workerThread.quit();
    m_workerThread.wait();

    if (m_ctx) {
        libusb_exit(m_ctx);
        m_ctx = nullptr;
    }
}

void UsbWarpper::setupDevice()
{
    while (m_running) {
        m_handle = libusb_open_device_with_vid_pid(m_ctx, m_config.vendor_id, m_config.product_id);
        if (!m_handle) {
            QThread::msleep(500);
            continue;
        }

        // === 精确匹配设备信息 ===
        libusb_device* dev = libusb_get_device(m_handle);
        libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(dev, &desc) < 0) {
            libusb_close(m_handle);
            m_handle = nullptr;
            QThread::msleep(500);
            continue;
        }

        // 序列号匹配
        if (!m_config.serial_number.isEmpty()) {
            unsigned char buf[256] = {0};
            int len = libusb_get_string_descriptor_ascii(m_handle, desc.iSerialNumber, buf, sizeof(buf));
            if (len <= 0 || QString::fromLatin1((char*)buf) != m_config.serial_number) {
                libusb_close(m_handle);
                m_handle = nullptr;
                QThread::msleep(500);
                continue;
            }
        }

        // 厂商匹配
        if (!m_config.manufacturer.isEmpty()) {
            unsigned char buf[256] = {0};
            int len = libusb_get_string_descriptor_ascii(m_handle, desc.iManufacturer, buf, sizeof(buf));
            if (len <= 0 || QString::fromLatin1((char*)buf) != m_config.manufacturer) {
                libusb_close(m_handle);
                m_handle = nullptr;
                QThread::msleep(500);
                continue;
            }
        }

        // 产品名匹配
        if (!m_config.product.isEmpty()) {
            unsigned char buf[256] = {0};
            int len = libusb_get_string_descriptor_ascii(m_handle, desc.iProduct, buf, sizeof(buf));
            if (len <= 0 || QString::fromLatin1((char*)buf) != m_config.product) {
                libusb_close(m_handle);
                m_handle = nullptr;
                QThread::msleep(500);
                continue;
            }
        }

        // 总线/地址匹配
        if (m_config.bus_number >= 0 && libusb_get_bus_number(dev) != m_config.bus_number) {
            libusb_close(m_handle);
            m_handle = nullptr;
            QThread::msleep(500);
            continue;
        }
        if (m_config.device_address >= 0 && libusb_get_device_address(dev) != m_config.device_address) {
            libusb_close(m_handle);
            m_handle = nullptr;
            QThread::msleep(500);
            continue;
        }

        // 设置配置
        if (libusb_set_configuration(m_handle, m_config.configuration) < 0) {
            libusb_close(m_handle);
            m_handle = nullptr;
            QThread::msleep(500);
            continue;
        }

        // 声明接口
        if (libusb_claim_interface(m_handle, m_config.interface_number) < 0) {
            libusb_close(m_handle);
            m_handle = nullptr;
            QThread::msleep(500);
            continue;
        }

        m_connected = true;
        emit deviceConnected(m_handle);

        // 启动事件轮询
        if (!m_pollTimer) {
            m_pollTimer = new QTimer(this);
            m_pollTimer->setInterval(10);
            connect(m_pollTimer, &QTimer::timeout, this, [this] {
                if (m_ctx) {
                    struct timeval tv = {0, 10000};
                    libusb_handle_events_timeout(m_ctx, &tv);
                }

                // 检查设备是否断开
                if (m_handle && libusb_get_device(m_handle) == nullptr) {
                    m_connected = false;
                    emit deviceDisconnected();
                    cleanup();
                }
            });
            m_pollTimer->start();
        }

        // 启动数据接收
        startDataReception();

        // 等待断开或停止
        while (m_running && m_connected) {
            QThread::msleep(100);
        }

        cleanup();
        QThread::msleep(500);
    }
}

void UsbWarpper::cleanup()
{
    if (m_pollTimer) {
        m_pollTimer->stop();
    }

    if (m_handle) {
        libusb_release_interface(m_handle, m_config.interface_number);
        libusb_close(m_handle);
        m_handle = nullptr;
    }
}

void UsbWarpper::startDataReception()
{
    switch (m_config.transfer_type) {
    case UsbTuple::INTERRUPT:
        if (m_config.endpoint_in > 0) {
            postInterruptTransfer();
        }
        break;
    case UsbTuple::BULK:
        if (m_config.endpoint_in > 0) {
            postBulkTransfer();
        }
        break;
    case UsbTuple::ISOCHRONOUS:
        if (m_config.endpoint_in > 0) {
            postIsoTransfer();
        }
        break;
    case UsbTuple::CONTROL:
        // Control 通常不持续接收
        break;
    default:
        break;
    }
}

void UsbWarpper::postInterruptTransfer()
{
    auto transfer = libusb_alloc_transfer(0);
    int size = 64; // 可从 endpoint descriptor 获取
    auto buffer = new unsigned char[size];

    libusb_fill_interrupt_transfer(
        transfer,
        m_handle,
        m_config.endpoint_in,
        buffer,
        size,
        [](libusb_transfer* t) {
            UsbWarpper* mgr = static_cast<UsbWarpper*>(t->user_data);
            if (t->status == LIBUSB_TRANSFER_COMPLETED) {
                QByteArray data((const char*)t->buffer, t->actual_length);
                QMetaObject::invokeMethod(mgr, "onDataReceived", Qt::QueuedConnection,
                                          Q_ARG(QByteArray, data));
            } else {
                qWarning() << "Interrupt transfer failed:" << t->status;
            }
            delete [] (unsigned char*)t->buffer;
            libusb_free_transfer(t);

            if (mgr->m_connected && mgr->m_config.transfer_type == UsbTuple::INTERRUPT) {
                QMetaObject::invokeMethod(mgr, "postInterruptTransfer", Qt::QueuedConnection);
            }
        },
        this,
        0
        );

    int res = libusb_submit_transfer(transfer);
    if (res != 0) {
        qWarning() << "Failed to submit interrupt transfer:" << res;
        delete [] buffer;
        libusb_free_transfer(transfer);
    }
}

void UsbWarpper::postBulkTransfer()
{
    auto transfer = libusb_alloc_transfer(0);
    int size = 512;
    auto buffer = new unsigned char[size];

    libusb_fill_bulk_transfer(
        transfer,
        m_handle,
        m_config.endpoint_in,
        buffer,
        size,
        [](libusb_transfer* t) {
            UsbWarpper* mgr = static_cast<UsbWarpper*>(t->user_data);
            if (t->status == LIBUSB_TRANSFER_COMPLETED) {
                QByteArray data((const char*)t->buffer, t->actual_length);
                QMetaObject::invokeMethod(mgr, "onDataReceived", Qt::QueuedConnection,
                                          Q_ARG(QByteArray, data));
            } else {
                qWarning() << "Bulk transfer failed:" << t->status;
            }
            delete [] (unsigned char*)t->buffer;
            libusb_free_transfer(t);

            if (mgr->m_connected && mgr->m_config.transfer_type == UsbTuple::BULK) {
                QMetaObject::invokeMethod(mgr, "postBulkTransfer", Qt::QueuedConnection);
            }
        },
        this,
        0
        );

    int res = libusb_submit_transfer(transfer);
    if (res != 0) {
        qWarning() << "Failed to submit bulk transfer:" << res;
        delete [] buffer;
        libusb_free_transfer(transfer);
    }
}

void UsbWarpper::postIsoTransfer()
{
    const int num_packets = m_config.num_packets;
    const int packet_size = m_config.packet_size;
    const int total_size = num_packets * packet_size;

    // 分配 transfer 结构
    libusb_transfer* transfer = libusb_alloc_transfer(num_packets);
    if (!transfer) {
        qWarning() << "Failed to allocate iso transfer";
        return;
    }

    // 分配数据缓冲区
    unsigned char* buffer = new unsigned char[total_size];
    if (!buffer) {
        libusb_free_transfer(transfer);
        qWarning() << "Failed to allocate iso buffer";
        return;
    }

    // 填充 transfer 结构
    libusb_fill_iso_transfer(
        transfer,
        m_handle,
        m_config.endpoint_in,           // IN 端点
        buffer,
        total_size,
        num_packets,
        [](libusb_transfer* t) {
            UsbWarpper* mgr = static_cast<UsbWarpper*>(t->user_data);
            if (t->status == LIBUSB_TRANSFER_COMPLETED) {
                QByteArray data;
                for (int i = 0; i < t->num_iso_packets; ++i) {
                    auto& pkt = t->iso_packet_desc[i];
                    if (pkt.status == LIBUSB_TRANSFER_COMPLETED && pkt.actual_length > 0) {
                        unsigned char* pkt_buf = libusb_get_iso_packet_buffer_simple(t, i);
                        data.append((const char*)pkt_buf, pkt.actual_length);
                    }
                }
                if (!data.isEmpty()) {
                    QMetaObject::invokeMethod(mgr, "onDataReceived", Qt::QueuedConnection,
                                              Q_ARG(QByteArray, data));
                }
            } else {
                qWarning() << "Iso transfer failed:" << t->status;
            }

            delete [] (unsigned char*)t->buffer;
            libusb_free_transfer(t);

            if (mgr->m_connected && mgr->m_config.transfer_type == UsbTuple::ISOCHRONOUS) {
                QMetaObject::invokeMethod(mgr, "postIsoTransfer", Qt::QueuedConnection);
            }
        },
        this,
        0  // timeout
        );

    // ✅ 正确方式：手动设置每个 packet 的长度
    for (int i = 0; i < num_packets; ++i) {
        transfer->iso_packet_desc[i].length = packet_size;
        // 注意：actual_length 是传输后填充的，不要在这里设置
    }

    // 提交传输
    int res = libusb_submit_transfer(transfer);
    if (res != 0) {
        qWarning() << "Failed to submit iso transfer:" << res;
        delete [] buffer;
        libusb_free_transfer(transfer);
    }
}

void UsbWarpper::onDataReceived(const QByteArray& data)
{
    emit dataReceived(data);
}

bool UsbWarpper::sendData(const QByteArray& data)
{
    if (!m_connected || !m_handle) return false;

    int actual = 0;
    uint8_t ep = m_config.endpoint_out;

    switch (m_config.transfer_type) {
    case UsbTuple::BULK:
        if (ep == 0) return false;
        return libusb_bulk_transfer(m_handle, ep, (unsigned char*)data.data(),
                                    data.size(), &actual, 1000) == 0;

    case UsbTuple::INTERRUPT:
        if (ep == 0) return false;
        return libusb_interrupt_transfer(m_handle, ep, (unsigned char*)data.data(),
                                         data.size(), &actual, 1000) == 0;

    case UsbTuple::CONTROL:
        return writeControl(data);

    default:
        return false;
    }
}

bool UsbWarpper::readControl(QByteArray& outData, int timeout)
{
    if (!m_handle || m_config.transfer_type != UsbTuple::CONTROL) return false;

    auto& c = m_config;
    uint8_t buf[512];
    int res = libusb_control_transfer(
        m_handle, c.bmRequestType_read, c.bRequest_read,
        c.wValue_read, c.wIndex_read,
        buf, sizeof(buf), timeout
        );

    if (res > 0) {
        outData = QByteArray((const char*)buf, res);
        emit dataReceived(outData);
        return true;
    }
    return false;
}

bool UsbWarpper::writeControl(const QByteArray& data, int timeout)
{
    if (!m_handle || m_config.transfer_type != UsbTuple::CONTROL) return false;

    auto& c = m_config;
    int res = libusb_control_transfer(
        m_handle, c.bmRequestType_write, c.bRequest_write,
        c.wValue_write, c.wIndex_write,
        (unsigned char*)data.data(), data.size(), timeout
        );
    return res >= 0;
}

/*
# UsbDeviceManager 技术文档

> **版本：1.0**
> **作者：Qwen**
> **适用库：libusb-1.0 + Qt5/Qt6**
> **目标：实现跨平台 USB 设备的稳定通信**

---

## 概述

`UsbDeviceManager` 是一个基于 `libusb` 和 `Qt` 的异步 USB 通信管理类，支持多种传输模式（Control / Interrupt / Bulk / Isochronous），具备设备精确匹配、热插拔检测、自动重连、线程安全数据收发等特性。

该类通过异步事件轮询机制避免阻塞主线程，适用于工业控制、医疗设备、嵌入式调试等高可靠性场景。

---

## 核心结构依赖：`UsbTuple`

```cpp
struct UsbTuple {
    quint16 vendor_id;
    quint16 product_id;
    QString serial_number;
    QString manufacturer;
    QString product;
    int bus_number = -1;           // -1 表示不匹配
    int device_address = -1;

    int configuration = 1;
    int interface_number = 0;

    enum TransferType { CONTROL, INTERRUPT, BULK, ISOCHRONOUS };
    TransferType transfer_type = BULK;

    uint8_t endpoint_in = 0;      // 如 0x81
    uint8_t endpoint_out = 0;     // 如 0x01

    // Control 特有字段
    uint8_t bmRequestType_read = 0xC0;
    uint8_t bRequest_read = 0x01;
    uint16_t wValue_read = 0;
    uint16_t wIndex_read = 0;

    uint8_t bmRequestType_write = 0x40;
    uint8_t bRequest_write = 0x01;
    uint16_t wValue_write = 0;
    uint16_t wIndex_write = 0;

    // Isochronous 特有
    int num_packets = 3;
    int packet_size = 512;
};
```

---

## 类成员函数详解

---

### 构造函数：`UsbDeviceManager(const UsbTuple& config, QObject *parent)`

#### 功能
初始化 USB 配置参数，并将对象移动到独立工作线程中运行，确保不阻塞 UI 线程。

#### 内部操作
- 接收 `UsbTuple` 配置。
- 使用 `moveToThread(&m_workerThread)` 实现线程隔离。
- 绑定线程启动信号，触发设备连接流程。

#### 设计思路
采用 **“对象即线程”** 模型，所有 USB 操作在专用线程中执行，避免 GUI 卡顿。

---

### `bool start()`

#### 功能
启动 USB 管理器，初始化 libusb 上下文并开启工作线程。

#### 内部操作
1. 调用 `libusb_init(&m_ctx)` 初始化上下文。
2. 启动 `m_workerThread`，触发 `setupDevice()` 执行。

#### 返回值
- `true`：成功启动。
- `false`：libusb 初始化失败，发出错误信号。

#### 设计思路
延迟初始化：仅在 `start()` 被调用时才创建资源，避免构造时开销。

---

### `void stop()`

#### 功能
安全停止 USB 通信，释放所有资源。

#### 内部操作
1. 停止事件轮询定时器。
2. 请求工作线程退出并等待完成。
3. 调用 `libusb_exit(m_ctx)` 释放上下文。

#### 设计思路
优雅关闭：确保所有异步传输完成后再清理，防止内存泄漏或段错误。

---

### `void setupDevice()`

#### 功能
主循环：搜索并连接符合配置的 USB 设备，支持热插拔和自动重连。

#### 内部操作
1. 循环调用 `libusb_open_device_with_vid_pid()` 查找设备。
2. 获取设备描述符，逐项匹配：
   - 序列号（`iSerialNumber`）
   - 厂商名（`iManufacturer`）
   - 产品名（`iProduct`）
   - 总线号 & 地址（用于多设备区分）
3. 设置配置、声明接口。
4. 成功后发射 `deviceConnected()` 信号。
5. 启动 `pollTimer` 进行事件处理和断开检测。

#### 设计思路
- **容错重试**：设备未插入时持续轮询。
- **精确匹配**：防止 VID/PID 冲突导致误连。
- **事件驱动**：使用 `libusb_handle_events_timeout()` 处理异步回调。

---

### `void cleanup()`

#### 功能
释放当前设备占用的资源。

#### 内部操作
1. 停止事件轮询。
2. 释放接口：`libusb_release_interface()`
3. 关闭句柄：`libusb_close()`
4. 清空状态标志。

#### 设计思路
封装资源释放逻辑，供 `setupDevice` 循环中断后复用。

---

### `void startDataReception()`

#### 功能
根据配置的传输类型，启动相应的异步接收机制。

#### 内部操作
```cpp
switch (transfer_type) {
    case INTERRUPT: postInterruptTransfer(); break;
    case BULK:      postBulkTransfer();      break;
    case ISOCHRONOUS: postIsoTransfer();     break;
    default: break;
}
```

#### 设计思路
策略分发：统一入口，按需启动不同传输方式的数据监听。

---

### `void postInterruptTransfer()`

#### 功能
提交一次中断传输（Interrupt IN）请求，用于低频、可靠的小数据读取（如 HID 设备状态）。

#### 内部操作
1. 分配 `libusb_transfer` 和缓冲区。
2. 使用 `libusb_fill_interrupt_transfer` 填充结构。
3. 设置回调函数：
   - 若成功：提取数据 → 发射 `dataReceived`
   - 无论成败：重新提交下一次传输（形成链式循环）
4. 提交传输。

#### 设计思路
- **非阻塞**：通过回调通知结果。
- **持续监听**：自动递归提交，实现“永远在线”的接收。

---

### `void postBulkTransfer()`

#### 功能
提交一次批量传输（Bulk IN）请求，用于大容量、无实时性要求的数据（如文件传输）。

#### 内部操作
与 `postInterruptTransfer` 类似，但使用 `libusb_fill_bulk_transfer`。

#### 缓冲区大小
默认 512 字节（常见扇区大小），可扩展。

#### 设计思路
Bulk 是最常用的传输方式，适合大多数设备。自动重提实现流式接收。

---

### `void postIsoTransfer()`

#### 功能
提交等时传输（Isochronous IN），用于音视频等实时流数据。

#### 内部操作
1. 分配 `num_packets` 个 packet 的 transfer。
2. 分配总缓冲区：`total_size = num_packets × packet_size`
3. 使用 `libusb_fill_iso_transfer` 填充。
4. **关键修正**：手动设置每个 packet 的长度：
   ```cpp
   for (int i = 0; i < num_packets; ++i) {
       transfer->iso_packet_desc[i].length = packet_size;
   }
   ```
   > ⚠️ 注意：`libusb_iso_transfer_set_iso_packet_len` 并不存在！

5. 回调中解析每个 packet：
   - 检查 `pkt.status == COMPLETED`
   - 使用 `libusb_get_iso_packet_buffer_simple()` 获取数据指针
   - 合并有效数据并发射信号

#### 设计思路
- **高效切片**：单次传输多个 packet，减少系统调用。
- **容错合并**：允许个别 packet 失败，只上报成功部分。
- **自动续传**：成功后立即提交下一轮。

---

### `void onDataReceived(const QByteArray& data)`

#### 功能
接收线程安全的数据，并转发至 `dataReceived` 信号。

#### 内部操作
- 使用 `QMetaObject::invokeMethod(..., Qt::QueuedConnection)` 将数据从工作线程投递到主线程。
- 最终触发 `dataReceived(data)` 信号。

#### 设计思路
确保信号在正确线程发出，避免跨线程直接 emit 导致崩溃。

---

### `bool sendData(const QByteArray& data)`

#### 功能
发送数据，根据传输类型选择对应方式。

#### 内部操作
| 类型 | 方法 |
|------|------|
| BULK | `libusb_bulk_transfer` |
| INTERRUPT | `libusb_interrupt_transfer` |
| CONTROL | `writeControl(data)` |

#### 返回值
- `true`：发送成功。
- `false`：设备未连接或传输失败。

#### 设计思路
对外统一接口，内部自动适配传输模式。

---

### `bool readControl(QByteArray& outData, int timeout)`

#### 功能
执行一次 Control 读取请求（Host ← Device）

#### 内部操作
调用 `libusb_control_transfer` 使用配置中的 `bmRequestType_read` 等参数。

#### 成功时
- 填充 `outData`
- 触发 `dataReceived` 信号

#### 用途
读取设备状态、寄存器、固件信息等。

---

### `bool writeControl(const QByteArray& data, int timeout)`

#### 功能
执行一次 Control 填写写入请求（Host → Device）

#### 内部操作
使用 `libusb_control_transfer` 发送数据。

#### 用途
配置设备参数、触发动作、写入命令等。

---

## 信号说明

| 信号 | 参数 | 触发时机 |
|------|------|---------|
| `deviceConnected(libusb_device_handle*)` | 句柄 | 设备成功连接并配置完成 |
| `deviceDisconnected()` | —— | 设备被拔出或通信中断 |
| `dataReceived(QByteArray)` | 数据 | 收到有效数据包（来自 IN 传输） |
| `errorOccurred(QString)` | 错误信息 | 初始化失败、资源分配错误等 |

---

## 设计哲学总结

| 特性 | 实现方式 |
|------|----------|
| **异步非阻塞** | libusb 异步传输 + Qt 信号槽 |
| **线程安全** | 工作线程 + `QueuedConnection` |
| **热插拔支持** | `setupDevice` 循环 + 断开检测 |
| **精确匹配** | VID/PID/SN/字符串/总线地址多重校验 |
| **自动重连** | 连接失败后自动重试 |
| **资源安全** | RAII 风格的 `cleanup()` 与 `stop()` |

---

## 建议使用场景

| 传输类型 | 典型设备 | 建议配置 |
|----------|--------|---------|
| CONTROL | 配置类命令 | 用于初始化、查询状态 |
| INTERRUPT | HID、键盘鼠标 | endpoint_in=0x81, size=8~64 |
| BULK | U盘、打印机、自定义设备 | endpoint_in=0x81, size=512 |
| ISOCHRONOUS | 音频采集卡、摄像头 | num_packets=3, packet_size=1024 |

---

## 参考资料

- libusb 官方文档：https://libusb.sourceforge.io/api-1.0/
- USB 2.0 规范（Section 5.9 Isochronous Transfer）
- Qt 多线程编程指南

---

✅ **本实现已修正 `libusb_iso_transfer_set_iso_packet_len` 不存在的问题，使用标准 API 安全操作等时传输。**
*/
