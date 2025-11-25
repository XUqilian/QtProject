#pragma once

#include <QObject>

// 以建立链接为导向，数据交换为目的 的顶层模型

// 特定链接对象必要的参数
// 创建控制         // qt采用非失败构建，此处与构造函数集成
// 链接控制
// 交互控制


class ISession : public QObject
{
    Q_OBJECT

public:

    enum SessionStatus{
        UnLink,
        Linking,
        Linked
    };

    enum class LinkType { USB, Serial, Network, Bluetooth };
    Q_ENUM(LinkType)

    explicit ISession(QObject* parent = nullptr) : QObject(parent) {}

    virtual bool link() = 0;
    virtual bool unlink() = 0;
    virtual bool send(const QByteArray& data) = 0;
    // virtual QByteArray recv() = 0;           //主动接收	调用 recv() 手动从缓冲区复制数据	NFC、调试器、DMA 设备、内存映射设备
    virtual LinkType linkType() const = 0;   // 类型由子类定义
    // virtual void dispose();  // { unlink();}
    // virtual QObject* object() const = 0;
    virtual ~ISession() = default;

signals:
    void received(const QByteArray& data);     // 被动接收信号
    void statusChanged(SessionStatus);       // link/unlink 状态变化  delete交给QObject
    void errorOccurred(QString);
};

    // UART / 串口              QSerialPort           工控、嵌入式、调试
    // USB (CDC/ACM)            QSerialPort         （虚拟串口）或 libusb	设备升级、调试、数据采集
    // TCP / Ethernet           QTcpSocket          工业设备、服务器通信
    // UDP                      QUdpSocket          广播、低延迟、实时数据
    // WebSocket                QWebSocket          Web 通信、跨平台、浏览器交互
    // Bluetooth (RFCOMM)       QBluetoothSocket	移动设备、无线外设
    // Bluetooth (BLE)          QLowEnergyController	低功耗设备（传感器、穿戴）
    // CAN Bus                  QCanBus             汽车、工业控制
    // I²C / SPI	通常通过串口或专用库（如 wiringPi）封装	嵌入式、树莓派、传感器
    // Named Pipe / LocalSocket	QLocalSocket        同机进程间通信（IPC）
    // Shared Memory            QSharedMemory       高频数据共享（图像、音频）
    // MQTT                 第三方库（如 QMqtt）       IoT、远程设备管理


