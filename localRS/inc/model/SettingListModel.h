#pragma once

#include <QAbstractListModel>
#include <interface/ISettingsLoad.h>

class SettingListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // enum IRole { ItemRole = Qt::UserRole + 1 };
    // Q_ENUMS(IRole)

    explicit SettingListModel(QObject *parent = nullptr);
    ~SettingListModel() override;

    // QAbstractItemModel 接口重写
    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // 自定义管理接口
    ISettingsLoad* currentModel() const;
    void setModel(ISettingsLoad *model);

    // 可选的便捷方法（Q_INVOKABLE 供 QML 调用）
    // Q_INVOKABLE SettingItem* item(const QString &key) const;
    // Q_INVOKABLE QVariant getData(int row) const;

// protected:
    // 为支持 QML 数据绑定，提供 roleNames
    // QHash<int, QByteArray> roleNames() const override;

private:
    ISettingsLoad *m_currentModel = nullptr;
    QList<SettingItem*> m_items; // 缓存当前模型的 items
};

