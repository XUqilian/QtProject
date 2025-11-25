#pragma once

#include <manager/WindowCtrler.h>

class Mainview : public WindowCtrler
{
    Q_OBJECT
public:
    explicit Mainview(QQmlEngine &engine, QObject *parent = nullptr);
    explicit Mainview(QQmlEngine &engine, const QUrl& url ,QString& componentName, QObject *parent = nullptr);
    // explicit Mainview(QQmlApplicationEngine &engine, const QString & qmlText , QObject *parent = nullptr);
    // explicit Mainview(QQmlApplicationEngine &engine, const QString& module, const QString& component ,
    //                   QQmlComponent::CompilationMode mode = QQmlComponent::CompilationMode::PreferSynchronous, QObject *parent = nullptr);
    // explicit Mainview(QQmlApplicationEngine &engine,const LoadSource & src, QObject *parent = nullptr);

    // void injection();    // 接收外部注入的核心组件 ViewManager


    // QML 调用：选择页面 - 从缓存m_vmscache获取 - 触发currentPageChanged - qml加载
    Q_INVOKABLE void selectPage(const QString &pageName);

public:
    Q_PROPERTY(QList<QString> pagelist READ pages NOTIFY pagesChanged)
    // Q_SIGNAL void pagesChanged();    using WindowCtrler::pagesChanged()

private:
    ViewModel* m_currentPage = {};
public:
    Q_PROPERTY(ViewModel* currentPage READ currentPage NOTIFY currentPageChanged FINAL)
    ViewModel* currentPage() const { return m_currentPage; }
    Q_SIGNAL void currentPageChanged();
    // Q_SIGNAL void currentPageChanged(ViewModel*);


    // WindowCtrler interface
public:
    virtual void init() override;

protected:
    virtual bool shouldAutoShow() const override;
    virtual bool shouldQuitOnClose() const override;

private:
    QQmlEngine * m_engine;
};


