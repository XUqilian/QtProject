#include <manager/ViewManager.h>
#include <core/gsc.h>
#include <view/mainview.h>
#include <view/settings.h>
#include <QQuickWindow>

QQmlApplicationEngine &ViewManager::getEngine()
{
    return m_engine;
}

void ViewManager::addWindow(QString windowName, WindowCtrler * obj)
{
    if(!m_windows.contains(windowName))
    {
        obj->setParent(this);       // 挂载到对象树进行管理
        m_windows.emplace(windowName,obj);
    }
}

void ViewManager::removeWindow(const QString &windowName)
{
    if(m_windows.contains(windowName))
    {
        WindowCtrler * var = m_windows.value(windowName);
        m_windows.remove(windowName);
        var->deleteLater();
    }

}

WindowCtrler * ViewManager::getWindow(const QString &windowName)
{
    return m_windows.value(windowName);
}

QList<QString> ViewManager::windows() const
{
    return m_windows.keys();
}
