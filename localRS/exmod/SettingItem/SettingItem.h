#ifndef SETTINGITEM_H
#define SETTINGITEM_H

#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <QJsonObject>
#include <QVariant>
#include <QMetaType>
#include <QStringList>
#include <QColor>

class theme
{
public:
    QColor Background;
    QColor TextPrimary;
    QColor TextSecondary;
    QColor PrimaryColor;
    QColor Accent;
    QColor Border;
    QColor Card;
};

class SettingItem : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Cannot create in QML because UNCREATABLE")

    // 核心属性
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)
    Q_PROPERTY(QString info READ info WRITE setInfo NOTIFY infoChanged FINAL)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged FINAL)
    Q_PROPERTY(SettingType type READ type CONSTANT)  // 改为 CONSTANT：构造后不可变

    // 静态扩展元数据
    Q_PROPERTY(QMap<QString, QVariant> configs READ configs WRITE configsSet NOTIFY configsChanged FINAL)


    // 动态扩展元数据
    // 选项
    Q_PROPERTY(QStringList options READ options WRITE setOptions NOTIFY optionsChanged FINAL)

    // Q_PROPERTY(QMap<QString,QString> options READ options WRITE setOptions NOTIFY optionsChanged  FINAL)

    // 该方法只会实现单次，需慎重
    // Q_INVOKABLE QVariant configValue(const QString& key, const QVariant& defaultValue = QVariant())
    //     {    return m_configs.contains(key) ? m_configs[key] : defaultValue;    }

public:
    enum SettingType {
        Unknown,

        // 基础数据
        Boolean,
        Integer,
        Number,
        String,
        TextArea,

        // 拓展

        // 路径与选择
        FilePath,
        DirPath,

        // 专用编辑器
        Color,
        Font,

        // 选择
        Enum,       // 下拉选择
        Flag,       // 多选标志
        List,       // 可编辑字符串列表

        // 动作
        Action,

        // 分隔
        Separator,
        Group
    };
    Q_ENUM(SettingType)

public:
    // 构造函数：必须传入类型，不允许 Unknown（除非显式指定）
    explicit SettingItem(SettingType type = SettingType::Unknown, QObject *parent = nullptr);

    // 便捷构造：带标题和类型的构造
    explicit SettingItem(SettingType type, QVariant value, const QString &title, const QString &info = "", QObject *parent = nullptr);

    // 更建议采用 QtJsonSerializer
    // 新增：序列化与反序列化接口
    QJsonObject toJson() const;
    bool fromJsonObject(const QJsonObject &json);
    // 静态工具：批量序列化/反序列化
    static QJsonArray toJsonArray(const QList<SettingItem*> &items);
    static QList<SettingItem*> fromJsonArray(const QJsonArray &array, QObject *parent = nullptr);

    static SettingItem* defaultItem()
    {
        static SettingItem intance(SettingType::Unknown);
        return &intance;
    }

public:
    // Getters
    QString title() const;
    QString info() const;
    QVariant value() const;
    SettingType type() const;  //只读

    QMap<QString, QVariant> configs() const;
    QStringList options() const;


    // Setters（value 仍可变，但类型锁定）
    void setTitle(const QString &title);
    void setInfo(const QString &info);
    void setValue(const QVariant &value);

    void configsSet(const QMap<QString, QVariant>& newConfigs);
    void setOptions(const QStringList &options);


signals:
    void titleChanged();
    void infoChanged();
    void valueChanged(const QVariant& value);  // 值可变，但类型不变

    void configsChanged();
    void optionsChanged();



private:
    QVariant convertToType(const QVariant& value, SettingType type) const;

    QString m_title = "Title";
    QString m_info = "Info";

    const SettingType m_type;  // 不再有 setter
    QVariant m_value = 0;

    // 动态扩展配置 提供无法使用QVariant承载的 function等 以及需要单独列出的
    QStringList m_options = {};             // 当前无选项
    // QMap<QString,QString> m_options;
    std::function<void()> m_todo = [](){};   // Action 的调用 可升级 std::function<int()> m_todo = [](){return 0;};

    //静态配置项（只会在初始化时读取一次），有变动也是内部从C++改动并且必须使用赋值且触发emit；QML直接使用键访问，如果没有也能使用默认值
    QMap<QString,QVariant> m_configs = {};

    // m_configs["nameFilters"] = "*";             // 全文件匹配
    // m_configs["placeholderText"] = "";         // 提示为空
    // m_configs["validator"] = "";              // "/(?:)/";  // 无验证,在qml中已做处理
    // m_configs["m_echoMode"] =  0;            // 不隐藏输入（非密码）
    // m_configs["from"] = 0;
    // m_configs["to"] = 0;
};

// Q_DECLARE_METATYPE(SettingItem*)

#endif // SETTINGITEM_H
