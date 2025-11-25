
#include <service/themeSettings.h>
#include <core/gsc.h>
#include <QColor>
#include <QSettings>

ThemeSettings::ThemeSettings(QObject * parent) : ISettingsLoad(parent) { Init();}
ThemeSettings::~ThemeSettings()
{
    dispose();
}

QList<SettingItem *> ThemeSettings::items() const
{
    QList<SettingItem *> t = m_map.values();
    int i = t.indexOf(theme);
    if(i > 0) t.move( i , 0 );
    return t;
}

SettingItem *ThemeSettings::item(const QString &key) const
{
    return m_map.value(key);
}

bool ThemeSettings::load()
{
    return true;
}

bool ThemeSettings::save()
{
    return true;
}

QString ThemeSettings::id() const
{
    return "";
}

QString ThemeSettings::bindName() const
{
    return "theme";
}

void ThemeSettings::dispose()
{
    disconnect(theme);
    qDeleteAll(m_map);  // 自动删除所有 SettingItem 对象
    m_map.clear();
}

void ThemeSettings::Init()
{
    backColor= new SettingItem(
        SettingItem::SettingType::Color,
        QColor::fromRgb(12, 12, 12),
        "背景颜色",
        "设置当前界面背景显示颜色",
        this
        );

    textColor = new SettingItem(
        SettingItem::SettingType::Color,
        QColor::fromRgb(0xE0, 0xE0, 0xE0),
        "主文本颜色",
        "设置当前界面文本显示颜色",
        this
        );

    infoTextColor = new SettingItem(
        SettingItem::SettingType::Color,
        QColor::fromRgb(0xB0, 0xB0, 0xB0),
        "提示文本颜色",
        "设置当前界面提示文本显示颜色",
        this
        );

    primaryColor = new SettingItem(
        SettingItem::SettingType::Color,
        QColor::fromRgb(00, 78, 0xD7),
        "主色调",
        "设置当前界面显示主色调",
        this
        );

    accentColor = new SettingItem(
        SettingItem::SettingType::Color,
        QColor::fromRgb(0xFF, 98, 00),
        "强调色",
        "设置当前界面强调颜色",
        this
        );

    borderColor = new SettingItem(
        SettingItem::SettingType::Color,
        QColor::fromRgb(44, 44, 44),
        "边框颜色",
        "设置当前界面边框显示颜色",
        this
        );

    cardColor = new SettingItem(
        SettingItem::SettingType::Color,
        QColor::fromRgb(0x1E, 0x1E, 0x1E),
        "卡片颜色",
        "设置当前界面卡片显示颜色",
        this
        );

    theme = new SettingItem(
        SettingItem::SettingType::Enum,
        QColor(Qt::black),
        "主题",
        "设置当前界面主题(背景色/主文字/次文字/主色调/强调色/边框色/卡片背景)",
        this
        );

    theme->setOptions(GSC::themeOptions);
    theme->setValue(GSC::themeOptions[1]);  // 预选light
    // theme->setOptions({"auto" ,"light" , "dark" , "highContrast" , "custom"});

    connect(theme,&SettingItem::valueChanged,this,&ThemeSettings::themeChange);

    m_map["theme"] = theme;

    m_map["backColor"] = backColor;

    m_map["textColor"] = textColor;

    m_map["infoTextColor"] = infoTextColor;

    m_map["primaryColor"] = primaryColor;

    m_map["accentColor"] = accentColor;

    m_map["borderColor"] = borderColor;

    m_map["cardColor"] = cardColor;
}

void ThemeSettings::themeChange(const QVariant & t)
{
    switch(GSC::themeOptions.indexOf(t))
    {
        case 1: // light
            changeTheme(GSC::lightMod);
            break;
        case 2: // dark
            changeTheme(GSC::darkMod);
            break;
        case 3: // hightContrast
            changeTheme(GSC::highContrastMod);
            break;
        case 4: // custom
            break;
        case 0: // auto
        default:
            QString mod = GSC::getSystemThemePreference();

            if("light" == mod) changeTheme(GSC::lightMod);
            else if ("dark" == mod) changeTheme(GSC::darkMod);

            break;
        };

}

void ThemeSettings::changeTheme(const QList<QColor> & colorList)
{
    if(colorList.count() < 7)
    {
        qDebug() << "warning: color mod is invalid.";
        return;
    }
    backColor->setValue(colorList[0]);
    textColor->setValue(colorList[1]);
    infoTextColor->setValue(colorList[2]);
    primaryColor->setValue(colorList[3]);
    accentColor->setValue(colorList[4]);
    borderColor->setValue(colorList[5]);
    cardColor->setValue(colorList[6]);
}


