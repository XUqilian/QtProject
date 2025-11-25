// settings.cpp
#include <core/GlobalConfig.h>
#include <manager/SettingsManager.h>
#include <view/settings.h>
#include <service/themeSettings.h>
#include <model/SettingListModel.h> // SettingListModel 定义
#include <SettingItem/SettingItem.h>       // SettingItem 定义


Settings::Settings(QQmlEngine &engine, QObject *parent)
    : ViewModel(engine, parent)
{
    Init();
}

Settings::Settings(QQmlEngine &engine, const QUrl& url, QString& componentName,QObject *parent)
    : ViewModel(engine, url,componentName, parent)
{
    Init();
}

// Settings::Settings(QQmlApplicationEngine &engine, const QString & qmlText, QObject *parent)
//     : ViewModel(engine, qmlText, parent)
// {
//     Init();
// }

// Settings::Settings(QQmlApplicationEngine &engine, const QString& module, const QString& component,
//                    QQmlComponent::CompilationMode mode, QObject *parent)
//     : ViewModel(engine, module, component, mode, parent)
// {
//     Init();
// }

// Settings::Settings(QQmlApplicationEngine &engine, const LoadSource & src, QObject *parent)
//     : ViewModel(engine, src, parent)
// {
//     Init();
// }

// std::string Settings::className(){ return std::string("Settings"); }

void Settings::Init()
{
    m_settings = GlobalConfig::instance()->setManager;       // 这里引用了尚未创建的对象 GlobalConfig创建会创建ViewManager触发Settings创建，Settings创建又会从GlobalConfig获取页面管理器ViewManager
    m_currentModel = new SettingListModel(this);
    m_serviceNames = m_settings->getKeys();
    connect(m_settings,&SettingsManager::mapChanged,this,
            [this](const QList<QString> newkeylist)
            {
                m_serviceNames = newkeylist;
                emit serviceNamesChanged();
            });
    if(m_serviceNames.count() > 0) currentChanged(m_serviceNames[0]);
}

void Settings::currentChanged(const QString & key)
{
    ISettingsLoad * var = m_settings->getISettingsLoad(key);
    if(var) m_currentModel->setModel(var);
    emit currentModelChanged();
}

SettingListModel* Settings::currentModel() const
{
    return m_currentModel;
}


