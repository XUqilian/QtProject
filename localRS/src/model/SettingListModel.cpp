// settinglistmodel.cpp
#include <model/SettingListModel.h>
#include <interface/ISettingsLoad.h>
#include <SettingItem/SettingItem.h>

// 可选：启用调试输出
// #include <QDebug>

SettingListModel::SettingListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_currentModel(nullptr)
{
}

SettingListModel::~SettingListModel()
{
    m_currentModel = nullptr;
    m_items.clear();
}

QVariant SettingListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) return QVariant();

    SettingItem *item = m_items.at(index.row());
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

int SettingListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())  return 0; // 列表模型无子项

    return m_items.count();
}

ISettingsLoad* SettingListModel::currentModel() const
{
    return m_currentModel;
}

void SettingListModel::setModel(ISettingsLoad *model)
{
    if (model == nullptr || m_currentModel == model) return;

    beginResetModel();
    m_currentModel = model;
    m_items = (m_currentModel) ? m_currentModel->items() : QList<SettingItem*>();
    endResetModel();
}


/*
// 可选的便捷方法实现
SettingItem* SettingListModel::item(const QString &key) const
{
    if (m_currentModel) {
        SettingItem* item = m_currentModel->item(key);
        if (item) {
            return item;
        }
    }
    return SettingItem::defaultItem();
}

QVariant SettingListModel::getData(int row) const
{
    QModelIndex index = this->index(row, 0);
    // qDebug() << "[DEBUG] getData called for row:" << row;
    return data(index, ItemRole);
}

// 重写 roleNames 以支持 QML 中使用 role 名称访问数据
QHash<int, QByteArray> SettingListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ItemRole] = "settingItem";
    return roles;
}

    int columnCount(const QModelIndex &parent = QModelIndex()) const override { }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        // 1. 边界检查
        if (!hasIndex(row, column, parent))  return QModelIndex();

        // 2. 如果 parent 是无效索引，说明是顶层（根） 返回第 row 行、第 column 列的顶层 item
        if (!parent.isValid())  return createIndex(row, column, m_items.at(row));
            // 注意：第三个参数 void* 可以用来存储 item 指针，避免查找

        // 3. 如果 parent 有效，说明在找“孩子的孩子”
        // 但在列表模型中，item 没有孩子，所以返回无效索引
        return QModelIndex();

    }
    QModelIndex parent(const QModelIndex &child) const override{ return QModelIndex(); }
*/
