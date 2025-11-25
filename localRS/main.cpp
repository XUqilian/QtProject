#include <QGuiApplication>
// #include <qqml.h>

#include <core/GlobalConfig.h>
#include <manager/ViewManager.h>
#include <manager/WindowCtrler.h>
#include <view/mainview.h>

// 只有“创建实例”才需要注册类型；“调用已有对象的方法”只需要元对象系统
// “QML 中只要用到某个 C++ 类的 类型名（如 property MyClass obj、函数返回值、参数等），就必须注册该类型（如 QML_ELEMENT）；
// 如果只是 访问一个已存在的实例（通过 setContextProperty 暴露），则不需要注册类型，只需 Q_OBJECT + Q_PROPERTY/Q_INVOKABLE。”

int main(int argc, char *argv[])
{
    // 在创建 QApplication 前设置
    // qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    qputenv("QT_IM_MODULE", "");  // 空字符串 = 禁用输入法

    QGuiApplication app(argc, argv);

    GlobalConfig * appgc = GlobalConfig::instance(&app);        // 全局配置直接挂载到程序上     构造函数主动注册类型
    appgc->viewManager->getEngine().rootContext()->setContextProperty("agc",appgc);       // 对qml引擎注册上下文

    // 窗口包含该对象还是该对象包含窗口呢？
    Mainview window(appgc->viewManager->getEngine(),appgc->viewManager);  // 主动挂载到对象树
    appgc->viewManager->addWindow("main",&window);        // 挂载到管理器的对象树
    window.show();        // 显示窗口

    // QtConcurrent::run([]() { });
    return app.exec();
}


/*
    QQmlContext m_componentContext(engine.rootContext());
    QQmlComponent m_component(&engine);

    QObject::connect(&m_component,&QQmlComponent::statusChanged,[&](){
        if (m_component.status() == QQmlComponent::Ready)
        {
            QObject * m_rootObject = m_component.create(&m_componentContext);
            if (!m_rootObject)  QCoreApplication::exit(-1);


        }
        else if (m_component.status() == QQmlComponent::Error) {
            QList<QQmlError> errors = m_component.errors();
            for (const QQmlError &error : errors) {
                qDebug() << "QML Error:" << error.toString();
            }
            QCoreApplication::exit(-1);
        }
    });

    m_component.loadUrl(QUrl::fromLocalFile("./localRS/UI/Mainview.qml"));
    m_component.loadFromModule("localRS", "Mainview");
    */


/*
QObject::connect(
    &engine,
    &QQmlApplicationEngine::objectCreationFailed,
    &app,
    []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);

engine.loadFromModule("localRS", "Main");
*/

/* // Mainview mainview(appgc->m_viewManager->getEngine());
    // // Mainview mainview(engine);
    // QObject::connect(&mainview,&ViewModel::qmlInitResult,&mainview,
    //     [&](){
    //         auto obj = mainview.rootObj(mainview.parent());
    //         if(obj) {
    //             QQuickWindow * window = qobject_cast<QQuickWindow*>(obj);
    //             if(window)
    //             {
    //                 QObject::connect(window, &QQuickWindow::closing, &app, []() {
    //                     // QCoreApplication::exit(0);  // 正常退出
    //                     QCoreApplication::quit();
    //                 });
    //                 window->show();
    //             }else
    //             {
    //                 QQuickItem * item = qobject_cast<QQuickItem*>(obj);
    //                 if(item)
    //                 {
    //                     window = new QQuickWindow();
    //                     QObject::connect(window, &QQuickWindow::closing, &app, []() {
    //                         // QCoreApplication::exit(0);  // 正常退出
    //                         QCoreApplication::quit();
    //                     });
    //                     item->setParentItem(window->contentItem());
    //                     window->show();
    //                 }
    //                 else  QCoreApplication::exit(-1);
    //             }
    //         }
    //         else  QCoreApplication::exit(-1);
    //     }
    // );
    // mainview.loadFromModule("localRS","Mainview");*/
