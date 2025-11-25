#pragma once

#include <interface/ISettingsLoad.h>

// 不传递 一般不被qml访问
class SettingsManager : public QObject
{
    Q_OBJECT
    // QML_ELEMENT
    // QML_UNCREATABLE("Cannot create in QML because UNCREATABLE!")

public:
    explicit SettingsManager(QObject *parent = nullptr);//: QObject(parent) {}
    virtual ~SettingsManager() override {} //= default;

    // 获取指定组的指定项 格式为GroupName.ItemName
    Q_INVOKABLE SettingItem* item(const QString & key);
    // 添加删除设置组
    void AddSettings(ISettingsLoad* iss);
    void RemoveSettings(ISettingsLoad* iss);

    const QList<QString> getKeys() const;
    // 返回设置组 返回对象不可管理其生命周期
    ISettingsLoad* getISettingsLoad(const QString & key)const;

signals:
    // 添加或删除组会触发，传递一个组名列表
    void mapChanged(const QList<QString> newkeylist);
private:
    QMap<QString, ISettingsLoad*> m_map{};

};

// Q_DECLARE_METATYPE(SettingsManager*)
