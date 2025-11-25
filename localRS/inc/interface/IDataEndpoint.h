#pragma once
#include <QObject>

// 以数据交换为目标的顶层模型

class IDataEndpoint : public QObject
{
    Q_OBJECT

public:
    virtual ~IDataEndpoint() = default;

    // 1. 能力查询
    virtual bool canRead() const = 0;
    virtual bool canWrite() const = 0;
    virtual bool isReadable() const { return true;};   // 是否有数据可读（可选）
    virtual bool isWritable() const{ return true;};   // 是否可写（可选）

    // 2. 启动/停止
    virtual bool start() = 0;
    virtual void stop() = 0;

    // 3. 数据操作（核心）
    virtual bool write(const QByteArray& data) = 0;
    virtual QByteArray read(){return nullptr;};          // 主动读取（默认返回空）
    virtual qint64 bytesAvailable() const { return 64;}; // 可选：可用字节数

signals:
    void readyRead();                  // 有数据可读（异步通知）
    void bytesWritten(qint64 bytes);   // 数据已写入（可选）
    void errorOccurred(const QString& error);
    void stateChanged();
};
