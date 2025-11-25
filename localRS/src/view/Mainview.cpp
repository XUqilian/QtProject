#include <view/mainview.h>
#include <view/settings.h>
#include <QQuickWindow>
#include <core/GlobalConfig.h>

Mainview::Mainview(QQmlEngine &engine, QObject *parent):WindowCtrler(engine,parent){}
Mainview::Mainview(QQmlEngine &engine, const QUrl& url ,QString& componentName, QObject *parent):WindowCtrler(engine,parent)
{
    loadUrl(url,componentName);
}

void Mainview::selectPage(const QString &pageName)
{
    ViewModel* ptr = getpage(pageName);
    if(ptr) emit currentPageChanged();  // (ptr);
    else qDebug() << "cannot find page!";
}

void Mainview::init()
{
    // Settings* settings = new Settings(*m_engine,this);  // (getEngine(),this);
    addPage("settings",new Settings(*m_engine,this));
}

bool Mainview::shouldAutoShow() const
{
    return true;
}

bool Mainview::shouldQuitOnClose() const
{
    return true;
}

// void Mainview::showSettingsDialog()
// {
//     QObject* settingsObj = engine().rootContext()->contextProperty("Gss").value<QObject*>();
//     Settings * settings = qobject_cast<Settings*>(settingsObj);
//     QObject * obj = settings->rootObj();
//     if(!obj) qDebug() << "obj was null!";
//     QQuickWindow* window = qobject_cast<QQuickWindow*>(settings->rootObj());
//     if(window)
//     {
//         window->setModality(Qt::ApplicationModal); // 设置模态
//         window->show();
//     }else qDebug() << "window was null!";
// }


