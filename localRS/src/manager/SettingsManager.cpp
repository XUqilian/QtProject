// settingsservice.cpp
#include <manager/SettingsManager.h>
#include <SettingItem/SettingItem.h>
#include <service/themeSettings.h>

SettingsManager::SettingsManager(QObject *parent) :QObject(parent)
{
    AddSettings(new ThemeSettings(this));
}

SettingItem *SettingsManager::item(const QString &key)
{
    QStringList parts = key.split('.');
    if(parts.count() < 2) return SettingItem::defaultItem();
    if(!m_map.contains(parts[0])) return SettingItem::defaultItem();
    SettingItem* item = m_map.value(parts[0])->item(parts[1]);
    return item ? item : SettingItem::defaultItem();
}

void SettingsManager::AddSettings(ISettingsLoad *iss)
{
    if(iss == nullptr || m_map.contains(iss->bindName())) return;
    m_map[iss->bindName()] = iss;
    emit mapChanged(m_map.keys());
}

void SettingsManager::RemoveSettings(ISettingsLoad *iss)
{
    if(iss != nullptr && m_map.contains(iss->bindName()))
    {
        m_map.remove(iss->bindName()) ;
        emit mapChanged(m_map.keys());
        // 可能需要对移除项做delete处理 暂时先挂在父子树上即可
    }
}

const QList<QString> SettingsManager::getKeys() const
{
    return m_map.keys();
}

ISettingsLoad *SettingsManager::getISettingsLoad(const QString &key) const
{
    return m_map.value(key);
}

