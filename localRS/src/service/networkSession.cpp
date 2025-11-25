// NetSession.cpp
#include <service/networkSession.h>
#include <QDebug>

NetSession::NetSession(const NetTuple & tup,QObject* parent)
    : ISession(parent)
{
    tuple = tup;

    if(NetTuple::Protocol::Tcp == tup.protocol())
    {
        m_socket = new QTcpSocket(this);
    }else m_socket = new QUdpSocket(this);

    if(tuple.remoteIp.isNull() || 0 == tuple.remotePort)
    {
        qDebug() << "Warning:tuple to/port is invalid.";
    }

    // m_socket 会在自己析构中断开所有的槽链接
    connect(m_socket,&QAbstractSocket::connected,this,[this](){emit statusChanged(SessionStatus::Linked);});
    connect(m_socket,&QAbstractSocket::disconnected,this,[this](){emit statusChanged(SessionStatus::UnLink);});
    connect(m_socket,&QIODevice::readyRead,this,&NetSession::onReadyRead);
    connect(m_socket,&QAbstractSocket::errorOccurred,this,[this](QAbstractSocket::SocketError t)
                {
                    // 将错误信号归并转发
                    switch (t) {
                    // =====================================================================
                    // 🔴 对端引发的错误（Remote Errors）
                    // 由服务器、代理或远端主动拒绝、关闭或认证失败导致
                    // =====================================================================
                    case QAbstractSocket::ConnectionRefusedError:
                    case QAbstractSocket::RemoteHostClosedError:
                    case QAbstractSocket::ProxyAuthenticationRequiredError:
                    case QAbstractSocket::ProxyConnectionRefusedError:
                    case QAbstractSocket::ProxyConnectionClosedError:
                        emit errorOccurred("remoteErr");
                        break;

                    // =====================================================================
                    // 🟢 本地引发的错误（Local Errors）
                    // 与本机资源、权限、配置、网络环境相关
                    // =====================================================================
                    case QAbstractSocket::HostNotFoundError:
                    case QAbstractSocket::SocketAccessError:
                    case QAbstractSocket::SocketResourceError:
                    case QAbstractSocket::DatagramTooLargeError:
                    case QAbstractSocket::NetworkError:
                    case QAbstractSocket::AddressInUseError:
                    case QAbstractSocket::SocketAddressNotAvailableError:
                    case QAbstractSocket::UnsupportedSocketOperationError:
                    case QAbstractSocket::SslInvalidUserDataError:
                        emit errorOccurred("localErr");
                        break;

                    // =====================================================================
                    // 🔵 未知/通用/混合来源错误（Unknown or General Errors）
                    // 来源不明确，可能本地也可能对端，或需重试
                    // 包括超时、协议错误、内部状态错误等
                    // =====================================================================
                    case QAbstractSocket::SocketTimeoutError:
                    case QAbstractSocket::UnfinishedSocketOperationError:
                    case QAbstractSocket::ProxyConnectionTimeoutError:
                    case QAbstractSocket::ProxyNotFoundError:
                    case QAbstractSocket::ProxyProtocolError:
                    case QAbstractSocket::OperationError:
                    case QAbstractSocket::SslHandshakeFailedError:
                    case QAbstractSocket::SslInternalError:
                    case QAbstractSocket::TemporaryError:
                    case QAbstractSocket::UnknownSocketError:
                    default:
                        emit errorOccurred("unknownErr");
                        break;
                    }
                }
            );

}

bool NetSession::link()
{
    if(tuple.remoteIp.isNull() || 0 == tuple.remotePort)
    {
        emit errorOccurred( "Warning:tuple to/port is invalid.");
        qDebug() << "Warning:tuple to/port is invalid.";
        return false;
    }

    // UDP 只收特定 IP:Port 的数据	✅ 调用 QUdpSocket::connectToHost(targetIp, targetPort)
    // 恢复为“广播/多播”模式	✅ 调用 disconnectFromHost()

    bool ok = m_socket->bind(tuple.localIp,tuple.localPort);
    if(!ok)
    {
        emit errorOccurred("socket cant bind ip/port.");
        qDebug() << "Warning:cant bind tuple form/port.";
        return false;
    }

    m_socket->connectToHost(tuple.remoteIp,tuple.remotePort);
    return true;

}

bool NetSession::unlink()
{
    m_socket->close();
    return true;
}

bool NetSession::send(const QByteArray& data)
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "NetSession: Cannot send, not connected";
        return false;
    }

    qint64 result = m_socket->write(data);
    m_socket->flush();
    return result == data.size();
}


void NetSession::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    if (!data.isEmpty()) {
        qDebug() << "NetSession: Received" << data.toHex();
        emit received(data);
    }
}

/* TcpServer 链接申请处理演示

void TcpServerService::onConnected()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* clientSocket = m_server->nextPendingConnection();

        // 添加到客户端列表
        m_clients.append(clientSocket);

        // 连接客户端信号
        // connect(clientSocket, &QTcpSocket::connected, this, &TcpServerService::onConnected);
        connect(clientSocket, &QTcpSocket::disconnected, this, &TcpServerService::onDisconnected);
        connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServerService::onReadyRead);

        qDebug() << "New client connected:" << clientSocket->peerAddress() << ":" << clientSocket->peerPort();

    }

}


*/
