#pragma once

// #pragma once
#include <interface/ISession.h>
#include<core/connection.h>
#include <QSerialPort>

class SerialSession : public ISession
{
    Q_OBJECT

public:
    explicit SerialSession(const SerialTuple & tup, QObject* parent = nullptr);
    ~SerialSession() override = default;

    LinkType linkType() const override;

    bool link() override;
    bool unlink() override;
    bool send(const QByteArray& data) override;
    // QObject * object() override;


private slots:
    void onReadyRead();  // 接收数据

private:
    QSerialPort* m_serial;
    SerialTuple tuple;
};

