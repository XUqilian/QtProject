// SerialSession.cpp
#include <service/serialSession.h>
#include <QDebug>

SerialSession::SerialSession(const SerialTuple & tup,QObject* parent)
    : ISession(parent)
    ,tuple(tup)
{
    m_serial = new QSerialPort(this);

    // 读写失败 且尝试重连失败 可视为设备离线
    connect(m_serial,&QSerialPort::errorOccurred,this,[this](QSerialPort::SerialPortError t)
            {
            switch (t) {
            // =====================================================================
            // 本地引发的错误（Local Errors）
            // 与本机设备、权限、资源、状态相关
            // =====================================================================
            case QSerialPort::SerialPortError::DeviceNotFoundError:
            case QSerialPort::SerialPortError::PermissionError:
            case QSerialPort::SerialPortError::OpenError:
            case QSerialPort::SerialPortError::ResourceError:
            case QSerialPort::SerialPortError::UnsupportedOperationError:
            case QSerialPort::SerialPortError::NotOpenError:
                emit errorOccurred("localErr");
                break;

            // =====================================================================
            // 🔵 公共/未知/混合来源错误（General/Unknown Errors）
            // 可能由通信过程、超时、硬件稳定性引起
            // =====================================================================
            case QSerialPort::SerialPortError::WriteError:  // 可用于检测设备是否掉线
            case QSerialPort::SerialPortError::ReadError:   // 可用于检测设备是否掉线
            case QSerialPort::SerialPortError::TimeoutError:
            case QSerialPort::SerialPortError::UnknownError:
                emit errorOccurred("unknownErr");
                break;

            // =====================================================================
            // 无错误（可选处理）
            // =====================================================================
            case QSerialPort::SerialPortError::NoError:
                // 可选：emit connected() 或忽略
                break;

            // =====================================================================
            // 默认情况（防御性编程）
            // =====================================================================
            default:
                emit errorOccurred("unknownErr");
                break;
            };
        });
    connect(m_serial, &QSerialPort::readyRead, this, &SerialSession::onReadyRead);
}


ISession::LinkType SerialSession::linkType() const
{
    return LinkType::Serial;
}

bool SerialSession::link()
{
    // m_serial->setFlowControl(QSerialPort::NoFlowControl);     // 可后续拓展，目前不使用

    m_serial->setPortName(QString().append(tuple.portName()));  // 可通过构造参数传入
    m_serial->setBaudRate(tuple.baudRate);
    m_serial->setDataBits(tuple.dataBits);
    m_serial->setParity(tuple.parity);
    m_serial->setStopBits(tuple.stopBits);

    if (m_serial->open(QIODevice::ReadWrite)) {
        qDebug() << "SerialService: Connected to" << m_serial->portName();
        emit statusChanged(SessionStatus::Linked);
        return true;
    } else {
        qWarning() << "SerialService: Failed to open " + tuple.portName() << m_serial->error();
        return false;
    }
}

bool SerialSession::unlink()
{
    if (m_serial->isOpen()) {
        m_serial->close();
        qDebug() << "SerialService: Disconnected";
        emit statusChanged(SessionStatus::UnLink);
    }
    return true;
}

bool SerialSession::send(const QByteArray& data)
{
    if (!m_serial->isOpen()) {
        qWarning() << "SerialService: Cannot send, port not open";
        return false;
    }

    qint64 result = m_serial->write(data);
    if (result == data.size()) {
        m_serial->flush();
        return true;
    } else {
        qWarning() << "SerialService: Send failed:" << result;
        return false;
    }
}

void SerialSession::onReadyRead()
{
    QByteArray data = m_serial->readAll();
    if (!data.isEmpty()) {
        qDebug() << "SerialService: Received" << data.toHex();
        emit ISession::received(data);
    }
}
