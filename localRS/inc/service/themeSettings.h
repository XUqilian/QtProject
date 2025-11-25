#pragma once

#include<inc/interface/ISettingsLoad.h>
#include <QColor>

class ThemeSettings : public ISettingsLoad
{
    Q_OBJECT

    // ISettingsLoad interface
public:
    virtual QList<SettingItem *> items() const override;
    virtual SettingItem *item(const QString &key) const override;
    virtual bool load() override;
    virtual bool save() override;
    virtual QString id() const override;
    virtual QString bindName() const override;

public:
    explicit ThemeSettings(QObject * parent = nullptr);
    virtual ~ThemeSettings();
private:
    void Init();
    void dispose();
    void changeTheme( const QList<QColor> & colorList);
private slots: void themeChange(const QVariant & t);

private:
    QMap<QString, SettingItem*> m_map;

    SettingItem * theme;
    SettingItem * backColor;
    SettingItem * textColor;
    SettingItem * infoTextColor;
    SettingItem * primaryColor;
    SettingItem * accentColor;
    SettingItem * borderColor;
    SettingItem * cardColor;
};


