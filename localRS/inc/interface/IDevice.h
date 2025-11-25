#pragma once

#include <interface/ISession.h>

// 与设备交互的包装
// 命令控制
// 报文解析


class IDevice : public QObject{

Q_OBJECT

public:
    enum IDevStatus{ err = -1 , free , linking , useing };
    enum reciveType{unknow, str};

    explicit IDevice(ISession * ics ,QObject * parent = nullptr);
    virtual ~IDevice() = default;

    // for IConnectedSession
    virtual bool link() = 0;
    virtual bool unlink() = 0;

    // for this
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool send(const QVariant & data) = 0;
    virtual bool doCmd(int cmd,const QVariant * data = nullptr) = 0;    // 简化固定数据发送的调用，底层调用send
    // virtual void recv() = 0;
    virtual bool changeIC(ISession * icc) = 0;

    inline IDevStatus getStatus(){return status;}
    /// @brief Caller(You) cant delete return point
    inline const ISession * getIC()const {return ic;}

signals:
    void recived(const reciveType & type, const QVariant & data);

private:
    IDevStatus status;
    ISession * ic;

};


