#pragma once

#include <SessionMessage/SessionMessage.h>

#include <QAbstractListModel>

class SessionMsgListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit SessionMsgListModel(QObject *parent = nullptr): QAbstractListModel(parent){}
    ~SessionMsgListModel() override = default;

    // for this
public:
    void addItem(SessionMessage* item);
    void removeItem(SessionMessage* item);
    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

private:
    QList<SessionMessage*> m_msgs = {};

};

void SessionMsgListModel::addItem(SessionMessage* item)
{
    if(m_msgs.contains(item)) return;
    qsizetype index = m_msgs.count();
    beginInsertRows(QModelIndex(),index,index);
    m_msgs.append(item);
    endInsertRows();
}

void SessionMsgListModel::removeItem(SessionMessage* item)
{
    for (int i = m_msgs.size() - 1; i >= 0; --i) {
        if (m_msgs.at(i) == item) {
            beginRemoveRows(QModelIndex(), i, i);
            m_msgs.removeAt(i);
            endRemoveRows();
        }
    }

    // while(m_msgs.contains(item))
    // {
    //     qsizetype index = m_msgs.lastIndexOf(item);     // 后向删除，减少损耗
    //     beginRemoveRows(QModelIndex(),index,index);
    //     m_msgs.remove(index);
    //     endRemoveRows();
    // }

    // m_msgs.removeAll(item);
    // beginResetModel();   // 重置整个模型
    // endResetModel();     // QML 会完全重建视图，无动画
}



int SessionMsgListModel::rowCount(const QModelIndex &parent) const
{
    return m_msgs.count();
}

QVariant SessionMsgListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_msgs.size()) return QVariant();

    SessionMessage *item = m_msgs.at(index.row());
    if (!item) return QVariant();

    // switch (role) {
    //     case Qt::DisplayRole:
    //         return QVariant::fromValue(item);
    // }

    if (role == Qt::DisplayRole) {
        return QVariant::fromValue(item);
    }

    return QVariant();
}
