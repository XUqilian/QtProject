#include <SettingItem/SettingItem.h>
#include <QDebug>
#include <QColor>
#include <QFont>
#include <QJsonArray>

// 构造函数：必须传入类型
SettingItem::SettingItem(SettingType type, QObject *parent)
    : QObject(parent)
    , m_type(type)
{
    // 根据类型设置默认值和默认范围
    switch (m_type) {
    case Boolean:
        m_value = false;
        break;
    case Integer:
        m_value = 0;
        break;
    case Number:
        m_value = 0.0;
        break;
    case String:
    case TextArea:
    case FilePath:
    case DirPath:
        m_value = QString();
        break;
    case List:
        m_value = QStringList();
        break;
    case Color:
        m_value = QColor(Qt::white);
        break;
    case Font:
        m_value = QFont("Arial", 14);
        break;
    case Enum:
        m_value = QString(); // 可以是空，或 options 第一项
        break;
    case Flag:
        m_value = 0;
        break;
    case Action:
    case Separator:
    case Group:
        m_value = QVariant(); // 无意义值
        break;
    default:
        m_value = QVariant();
        break;
    }
}

// 便捷构造：带标题和信息
SettingItem::SettingItem(SettingType type, QVariant value, const QString &title, const QString &info, QObject *parent)
    : SettingItem(type, parent)  // 委托构造
{
    m_value = value;
    m_title = title;
    m_info = info;
}

// keep value type safe
QVariant SettingItem::convertToType (const QVariant& value, SettingType type) const
{
    // 如果 value 无效，直接使用默认值
    if (!value.isValid() || value.isNull()) {
        switch (type) {
        case Boolean:  return false;
        case Integer:  return 0;
        case Number:   return 0.0;
        case String:
        case FilePath:
        case DirPath:
        case TextArea: return QString();
        case Color:    return QColor(Qt::white);  // 或 Qt::transparent
        case Font:     return QFont("Arial", 10); // 默认字体
        default:       return QVariant();
        }
    }

    // 检查是否已经是目标类型，避免无谓转换
    switch (type) {
    case Boolean:
        return value.toBool();
    case Integer:
        return value.canConvert<int>() ? value.toInt() : 0;
    case Number:
        return value.canConvert<double>() ? value.toDouble() : 0.0;
    case String:
    case FilePath:
    case DirPath:
    case TextArea:
        return value.toString(); // toStr 会处理 null → ""
    case Color:
        if (value.canConvert<QColor>()) {
            return value.value<QColor>();
        }
        return QColor(Qt::white); // 默认颜色
    case Font:
        if (value.canConvert<QFont>()) {
            return value.value<QFont>();
        }
        return QFont("Arial", 14); // 默认字体
    default:
        return value;
    }
}

// ====== Saves ======
QJsonObject SettingItem::toJson() const
{
    QJsonObject obj;

    obj["type"] = m_type;  // 自动转为 int
    if (!m_title.isEmpty())
        obj["title"] = m_title;
    if (!m_info.isEmpty())
        obj["info"] = m_info;

    // 序列化 value（支持多种类型）
    if (m_value.isValid()) {
        switch (m_value.userType()) {
        case QMetaType::Bool:
            obj["value"] = m_value.toBool();
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
            obj["value"] = m_value.toInt();
            break;
        case QMetaType::Double:
            obj["value"] = m_value.toDouble();
            break;
        case QMetaType::QString:
            obj["value"] = m_value.toString();
            break;
        case QMetaType::QStringList: {
            QStringList valueArr = m_value.toStringList();
            QJsonArray arr;
            for (const QString &s : std::as_const(valueArr))
                arr.append(s);
            obj["value"] = arr;
            break;
        }
        case QMetaType::QColor:
            obj["value"] = m_value.value<QColor>().name(QColor::HexArgb);  // #AARRGGBB
            break;
        case QMetaType::QFont: {
            QFont f = m_value.value<QFont>();
            obj["value"] = f.toString();  // "Arial,10,-1,5,50,0,0,0,0,0"
            break;
        }
        default:
            // 其他类型尝试 toString()
            if (m_value.canConvert<QString>())
                obj["value"] = m_value.toString();
            else
                obj["value"] = m_value.toString();  // fallback
            break;
        }
    }

    // 其他元数据
    if (!m_options.isEmpty()) {
        QJsonArray opts;
        for (const QString &opt : m_options)
            opts.append(opt);
        obj["options"] = opts;
    }

    return obj;
}

bool SettingItem::fromJsonObject(const QJsonObject &json)
{
    bool changed = false;

    if (json.contains("title")) {
        setTitle(json["title"].toString());
        changed = true;
    }
    if (json.contains("info")) {
        setInfo(json["info"].toString());
        changed = true;
    }

    // 恢复 value（注意类型匹配）
    if (json.contains("value")) {
        const QJsonValue &val = json["value"];

        QVariant newValue;
        switch (m_type) {
        case Boolean:
            newValue = val.toBool(false);
            break;
        case Integer:
            newValue = val.toInt(0);
            break;
        case Number:
            newValue = val.toDouble(0.0);
            break;
        case String:
        case TextArea:
        case FilePath:
        case DirPath:
            newValue = val.toString();
            break;
        case List:{
            if (val.isArray()) {
                QJsonArray arr = val.toArray();
                QStringList lst;
                for (const QJsonValue &v : std::as_const(arr))
                    lst.append(v.toString());
                newValue = lst;
            }
            break;
        }
        case Color:{
            newValue = QColor(val.toString());
            if (!newValue.value<QColor>().isValid())
                newValue = QColor(Qt::white);
            break;
        }
        case Font:{
            QFont f;
            f.fromString(val.toString());
            newValue = f;
            break;
        }
        case Enum: {
            // 可以是字符串或索引
            if (val.isString())
                newValue = val.toString();
            else if (val.isDouble()) // JSON 数字都是 double
                newValue = static_cast<int>(val.toDouble());
            break;
        }
        case Unknown:
        default:
            // 其他类型尝试字符串
            newValue = val.toVariant();
            break;
        }

        if (newValue.isValid() && m_value != newValue) {
            m_value = newValue;
            emit valueChanged(m_value);
            changed = true;
        }
    }

    if (json.contains("options") && json["options"].isArray()) {
        QJsonArray arr = json["options"].toArray();
        QStringList opts;
        for (const QJsonValue &v : std::as_const(arr)) {
            opts.append(v.toString());
        }
        setOptions(opts);
        changed = true;
    }

    return changed;
}

QJsonArray SettingItem::toJsonArray(const QList<SettingItem*> &items)
{
    QJsonArray array;
    for (SettingItem *item : items) {
        if (item)
            array.append(item->toJson());
    }
    return array;
}

QList<SettingItem*> SettingItem::fromJsonArray(const QJsonArray &array, QObject *parent)
{
    QList<SettingItem*> items;

    for (const QJsonValue &value : array) {
        if (!value.isObject()) continue;

        QJsonObject obj = value.toObject();
        if (!obj.contains("type")) continue;

        SettingItem::SettingType type = static_cast<SettingItem::SettingType>(obj["type"].toInt(0));

        SettingItem *item = new SettingItem(type, parent);
        item->fromJsonObject(obj);  // 恢复所有属性
        items.append(item);
    }

    return items;
}

// ====== Getters ======

QString SettingItem::title() const {    return m_title; }

QString SettingItem::info() const {     return m_info; }

QVariant SettingItem::value() const {   return convertToType(m_value,m_type); }

SettingItem::SettingType SettingItem::type() const {    return m_type; }


QMap<QString, QVariant> SettingItem::configs() const {  return m_configs;  }

QStringList SettingItem::options() const {  return m_options;}



// ====== Setters ======

void SettingItem::setTitle(const QString &title)
{
    if (m_title == title) return;
    m_title = title;
    emit titleChanged();
}

void SettingItem::setInfo(const QString &info)
{
    if (m_info == info) return;
    m_info = info;
    emit infoChanged();
}

void SettingItem::setValue(const QVariant &value)
{
    if(m_type == SettingType::Unknown) return;  // 不允许操作未知和默认

    // 可选：类型检查（防止非法赋值）
    if (!value.isValid()) { }        // 允许无效值，但可以警告

    if (m_value == value) return;

    m_value = convertToType(value,m_type);
    emit valueChanged(m_value);
}


void SettingItem::configsSet(const QMap<QString, QVariant>& newConfigs)
{
    if (m_configs != newConfigs) {
        m_configs = newConfigs;
        emit configsChanged();
    }
}

void SettingItem::setOptions(const QStringList &options)
{
    if (m_options == options) return;
    m_options = options;
    emit optionsChanged();
}


