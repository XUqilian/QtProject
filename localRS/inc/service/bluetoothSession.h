#pragma once
/*
#include <interface/IConnectedChannel.h>
#include<QBluetoothSocket>

class BluetoothSppCommunicator : public IConnectedChannel
{
    Q_OBJECT
    QBluetoothSocket* m_socket;

public:
    LinkType linkType() const override { return LinkType::Bluetooth; }

    bool link() override {
        m_socket->connectToService(QBluetoothAddress("XX:XX:XX:XX:XX:XX"),
                                   QBluetoothUuid(QBluetoothUuid::SerialPort));
        return m_socket->waitForConnected(3000);
    }

    bool send(const QByteArray& data) override {
        return m_socket->write(data) == data.size();
    }

private slots:
    void onReadyRead() {
        emit received(m_socket->readAll());
    }
};
*/

/*
// 低功耗蓝牙（BLE）—— 基于 GATT

class BleCommunicator : public ICommunicator
{
    Q_OBJECT
    QLowEnergyController* m_controller;
    QLowEnergyService* m_service;

public:
    LinkType linkType() const override { return LinkType::Bluetooth; }

    bool link() override {
        m_controller = QLowEnergyController::createCentral(...);
        connect(m_controller, &QLowEnergyController::connected, this, [this]{
            m_controller->discoverServices();
        });
    }

    bool send(const QByteArray& data) override {
        m_service->writeCharacteristic(m_char, data);
        return true;
    }
};
 */
