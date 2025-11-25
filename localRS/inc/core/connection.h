#pragma once

// #include <QtNetwork>
#include <QHostAddress>
#include <QSerialPort>

struct NetTuple {

public:
    enum class Protocol {
        Tcp,
        Udp,
    };

    // 成员变量
    QHostAddress localIp;
    std::uint16_t localPort = 0;
    QHostAddress remoteIp;
    std::uint16_t remotePort = 0;
    Protocol protocol()const {return m_protocol;}


    // 构造函数（可选）
    explicit NetTuple(Protocol protocol = NetTuple::Protocol::Tcp) : m_protocol(protocol){}

    NetTuple(const NetTuple &) = default;
    NetTuple(NetTuple &&) = default;
    NetTuple &operator=(const NetTuple &) = default;
    NetTuple &operator=(NetTuple &&) = default;

    // auto operator<=>(const NetTuple&) const = default; 大小无意义
    bool operator==(const NetTuple& other) const = default; // 对类中每一个可访问的非静态成员变量，使用 == 进行比较，按声明顺序
    bool operator!=(const NetTuple& other) const { return !(*this == other); }

    private: Protocol m_protocol;
};



struct SerialTuple {

 private: std::string m_portName;           // "/dev/ttyS0" 或 "COM1"  只允许构造时修改

 public:
    std::uint32_t baudRate = QSerialPort::Baud115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    const std::string& portName(){return m_portName;}

    SerialTuple(const SerialTuple &) = default;
    SerialTuple(SerialTuple &&) = default;
    SerialTuple &operator=(const SerialTuple &) = default;
    SerialTuple &operator=(SerialTuple &&) = default;

    explicit SerialTuple(const std::string &portname) : m_portName(portname) {}


    // 自动生成比较
    bool operator==(const SerialTuple&) const = default;
    bool operator!=(const SerialTuple& other) const { return !(*this == other); }
};

struct UsbTuple
{
    // enum in libusb.h
    enum tType {
        /** Control transfer */
        CONTROL = 0U,

        /** Isochronous transfer */
        ISOCHRONOUS = 1U,

        /** Bulk transfer */
        BULK = 2U,

        /** Interrupt transfer */
        INTERRUPT = 3U,

        /** Bulk stream transfer */
        BULK_STREAM = 4U
    };

    // 完整参数表

        // 🔹 1. 设备识别
        uint16_t vendor_id;
        uint16_t product_id;

        // 🔹 2. 接口配置
        uint8_t  interface_number;
        int      configuration = 1;

        // 🔹 3. 传输类型
        tType transfer_type;
        // 控制传输 (Control)       繁琐	设备配置、命令                 libusb_control_transfer
        // 中断传输 (Interrupt)     简单	键盘、鼠标、状态上报          libusb_interrupt_transfer
        // 批量传输 (Bulk)          简单	大数据传输（如打印机）         libusb_bulk_transfer
        // 等时传输 (Isochronous)	复杂	音视频流                    libusb_iso_transfer

        // 🔹 4. 端点地址（Bulk/Interrupt/Isochronous 使用）
        uint8_t  endpoint_in     = 0;   // IN 端点地址（设备 → 主机）
        uint8_t  endpoint_out    = 0;   // OUT 端点地址（主机 → 设备）

        // 🔹 5. Control 专用参数
        uint8_t  bmRequestType_write = 0;
        uint8_t  bmRequestType_read  = 0;
        uint8_t  bRequest_write = 0;
        uint8_t  bRequest_read  = 0;
        uint16_t wValue_write = 0;
        uint16_t wValue_read  = 0;
        uint16_t wIndex_write = 0;
        uint16_t wIndex_read  = 0;

        // 🔹 6. Isochronous 专用
        int      packet_size = 512;
        int      num_packets = 3;

        // 🔹 7. 精确匹配（多设备）
        QString serial_number;
        QString manufacturer;
        QString product;
        int     bus_number = -1;
        int     device_address = -1;
        std::vector<uint8_t> port_numbers;


    // libusb_control_transfer 调用参数
    // dev_handle	设备句柄	通过 libusb_open_device_with_vid_pid() 获取	handle
    // bmRequestType	请求类型	最重要！ 定义传输方向、类型、接收者	0x40 (主机→设备, 标准)
    // D7: 传输方向
    //     0 = 主机 → 设备（写）
    //     1 = 设备 → 主机（读）
    // D6..D5: 请求类型
    //     00 = 标准 (Standard)
    //     01 = 类别 (Class)
    //     10 = 厂商 (Vendor)
    //     11 = 保留
    // D4..D0: 接收者 (Recipient)
    //     00000 = 设备
    //     00001 = 接口
    //     00010 = 端点
    //     00011 = 其他
    // bRequest	请求码	具体命令（由设备定义）	0x09 (SET_ADDRESS)      bRequest 没有统一标准，必须查设备文档！
    // wValue	值参数	附加参数，含义由 bRequest 决定	0x0200
    // wIndex	索引参数	通常用于接口号、端点号等	0x0000
    // data	数据缓冲区	读写数据的指针	buffer
    // wLength	数据长度	要传输的字节数	64
    // timeout	超时（ms）	0 表示无限等待	100
};



/*
 * 底层四元组
class IpAddress {
public:
    // 禁止默认构造或构造非法地址
    explicit IpAddress(const std::array<uint8_t, 16>& bytes, bool isIpv6);

    // 专用构造函数
    static std::optional<IpAddress> fromV4String(std::string_view ip);
    static std::optional<IpAddress> fromV6String(std::string_view ip);
    static std::optional<IpAddress> fromString(std::string_view ip); // 自动判断

    // 便捷构造
    static IpAddress localhostV4();
    static IpAddress localhostV6();
    static IpAddress anyV4();
    static IpAddress anyV6();

    // 访问器
    const std::array<uint8_t, 16>& bytes() const { return m_bytes; }
    bool isIpv4() const { return m_isIpv4; }
    bool isIpv6() const { return m_isIpv6; }
    bool isLoopback() const;
    bool isAny() const;

    // 转换
    std::string toString() const;
    uint32_t toV4Uint() const; // 仅对 IPv4 有效，否则抛异常或返回 optional

    // 比较
    auto operator<=>(const IpAddress&) const = default;
    bool operator==(const IpAddress&) const = default;

private:
    std::array<uint8_t, 16> m_bytes = {};
    bool m_isIpv4 = false;
    bool m_isIpv6 = false;

    // 私有构造，确保一致性
    IpAddress() = default; // 仅供 friend 或序列化使用
    void validateAndSet(const std::array<uint8_t, 16>& bytes, bool isIpv6);
};

struct NetTuple {
    std::array<uint8_t, 16> fromIp = {};  // IPv6 兼容
    std::uint16_t fromPort = 0;
    std::array<uint8_t, 16> toIp = {};
    std::uint16_t toPort = 0;

    enum class Protocol { Tcp, Udp, Unknown } protocol = Protocol::Unknown;

    auto operator<=>(const NetTuple&) const = default;
};
*/
