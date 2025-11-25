#pragma once

// NetworkService.h

#include <core/connection.h>
#include <interface/ISession.h>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>

// OTO  一对一
class NetSession : public ISession
{
    Q_OBJECT
public:
    explicit NetSession(const NetTuple & tup , QObject * parent);

    virtual ~NetSession()
    {
        m_socket->close();
        m_socket->deleteLater();
    }
    // IConnectedSession interface
public:
    virtual bool link() override;
    virtual bool unlink() override;
    virtual bool send(const QByteArray &data) override;
    virtual LinkType linkType() const override{return LinkType::Network;}

private slots:
    void onReadyRead();

private:
    QAbstractSocket * m_socket;
    NetTuple tuple;
};



/* OTM 一对多 的预想方案
 * 借助udp或者tcpServer
class NetSocketManager:QObject
{
    Q_OBJECT

public:

    const QAbstractSocket* create(const NetTuple &)
    {
        // 检查协议
        // 检查是否为服务器
        // 分配套接字(关联销毁到容器移除上)
        return nullptr;
    }

    bool destroy(const QAbstractSocket* socket ,const NetTuple & tup) {
        if (!socket)  return false;
        int index = socketList.indexOf(socket);
        if(-1 == index) return false;

        QAbstractSocket* skt = socketList[index];
        if(tup.protocol() == NetTuple::Protocol::Tcp)
        {
            skt->disconnectFromHost(); // 主动断开
            socketList.removeOne(skt); // 先从列表移除
        }
        skt->deleteLater();  // 再请求删除

        return true;
    }

    bool send(const QAbstractSocket* sender, const NetTuple & tup, const QByteArray& data) {
        if (!sender)  return false;
        int index = socketList.indexOf(sender);
        if(-1 == index) return false;
        QAbstractSocket* socket = socketList[index];

        if(tup.protocol() == NetTuple::Protocol::Udp)
        {
            QUdpSocket* udpSocket = qobject_cast<QUdpSocket*>(socket);
            if(!udpSocket) return false;
            return udpSocket->writeDatagram(data, tup.to, tup.toPort) > 0;

        }else  return socket->write(data) > 0;
    }

    static NetSocketManager * getInstance()
    {
        static NetSocketManager instance;
        return &instance;
    }



public:

    NetSocketManager(const NetSocketManager &) = delete;
    NetSocketManager & operator=(const NetSocketManager&) = delete;
private:
    NetSocketManager() = default;
    ~NetSocketManager() {
        // 程序结束可同步强制删除
        qDeleteAll(socketList); // 立即删除所有套接字  QAbstractSocket会主动关闭链接的
        socketList.clear();     // 清空列表
    }

    QList<QAbstractSocket*> socketList;

    // 双hash存储 服务器角色tcpserver&udp       // 但他俩得管理一个表，当tcp没有链接后要主动关停，udp在最后一个提交注销申请后也关停
    QHash<qintptr, QAbstractSocket*> mainStorage;    // 主表
    QHash<QString, qintptr> indexByAddress;     // 查找表 key = "ip：port"


    // signals:
    //     void onNewConnected(const QAbstractSocket*);

    //     bool enableServer(const NetTuple &);     // server only one,can change
    //     bool disableServer();

    //     QTcpServer* server;

};
*/
