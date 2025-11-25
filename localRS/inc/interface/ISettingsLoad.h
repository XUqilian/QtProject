// this file will open sources for everyone
#pragma once

#include <QObject>
#include <SettingItem/SettingItem.h>
// #include <QtGui>
// class SettingItem;

class ISettingsLoad : public QObject
{
    Q_OBJECT

public:
    explicit ISettingsLoad(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ISettingsLoad() override = default;

    /// @brief 返回的 SettingItem* 由本类拥有，调用者不得 delete
    virtual QList<SettingItem*> items() const = 0;
    /// @brief 返回的 SettingItem* 由本类拥有，调用者不得 delete
    virtual SettingItem* item(const QString &key) const = 0;

    // 安全接口：返回共享指针
    // virtual QSharedPointer<SettingItem> item(const QString& key) const = 0;
    // virtual QList<QSharedPointer<SettingItem>> items() const = 0;

    // ========== 持久化 ==========
    virtual bool load() = 0;
    virtual bool save() = 0;

    // ========== 元信息 ==========
    virtual QString id() const = 0;
    virtual QString bindName() const = 0;
    virtual QString version() const { return "1.0"; }
    virtual QString author() const { return "Unknown"; }

    // ========== 生命周期 ==========
    virtual void onActivated() {}
    virtual void onDeactivated() {}

signals:
    void modelChanged();
};
Q_DECLARE_INTERFACE(ISettingsLoad, "ISettingsLoad")
Q_DECLARE_METATYPE(ISettingsLoad*)

