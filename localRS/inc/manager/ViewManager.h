#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <QQuickItem>
#include <core/ViewModel.h>
#include <core/core.h>
#include <interface/IGlobalManager.h>
#include <manager/WindowCtrler.h>
// class ItemManager;      // manage viewmodel create item

class ViewManager : public QObject //, public IGlobalManager
{
    Q_OBJECT
    // QML_ELEMENT
    // QML_UNCREATABLE("Cannot create in QML because UNCREATABLE!")

public:
    explicit ViewManager( QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ViewManager() override {} //= default;
    // ViewManager * Instance();
public:
    static QQmlApplicationEngine & getEngine();

    void addWindow(QString,WindowCtrler*);
    void removeWindow(const QString &componentName);
    WindowCtrler* getWindow(const QString &pageName);
public:
    QList<QString> windows() const ;

signals:
    void doResult(resultST ret);

private:
    static QQmlApplicationEngine m_engine;        // 可直接在栈上分配
    QHash<QString,WindowCtrler*> m_windows;      // 窗口管理器

};


// Q_DECLARE_METATYPE(ViewManager*)
