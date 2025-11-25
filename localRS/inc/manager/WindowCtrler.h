#pragma once
#include <core/ViewModel.h>
#include <QQuickWindow>
#include <core/ViewModel.h>
#include <Notification/Notification.h>

// 使用方法
// 构建对象 => init初始化某些必须的内容（无操作时可忽略） => ViewModel::loadqml加载窗口蓝图（这里或上一步需要监听自己的ViewModel::statusChanged信号） => show显示主窗口
class WindowCtrler : public ViewModel{
    Q_OBJECT
    // QML_ELEMENT
    // QML_UNCREATABLE("Cannot create in QML because UNCREATABLE!")

    Q_PROPERTY(NotificationQueue* notify MEMBER m_notifyQueue CONSTANT FINAL)

public:
    explicit WindowCtrler(QQmlEngine &engine,QObject * parent);
    virtual ~WindowCtrler(){}

    void show();

    // 预加载某个页面 - 根据参数创建ViewModel并监听加载情况 - 触发加载情况信号loadQmlResult - 成功则添加到缓存m_vmscache 同步更新m_pages并触发pagesChanged
    void addPage(const QString& componentName , ViewModel* vm = nullptr);
    void addPage(const QString& componentName , const QUrl &url , ViewModel* vm = nullptr);
    void addPage(const QString& componentName , const QString& qmlText , ViewModel* vm = nullptr);
    void addModulePage(const QString& componentName , const QAnyStringView& moduleUri, ViewModel* vm = nullptr,
                       QQmlComponent::CompilationMode INmode = QQmlComponent::CompilationMode::PreferSynchronous);
    // 释放页面缓存 - 移除缓存m_vmscache的指定项 同步更新m_pages并触发pagesChanged
    void releasePage(const QString &componentName);
    ViewModel* getpage(const QString &pageName);
    QList<QString> pages() const { return m_vmscache.keys(); }

signals:
    void loadQmlResult(const QString &name, resultST ret);
    void pagesChanged();

    // virtual function
public:
    virtual void init();        // 在构造结束后会执行的内容
protected:
    // —————— 可重写行为 ——————      用信号槽传递事件，用虚函数定义策略。
    virtual void whenWindowReady(QQuickWindow* window);           // 窗口创建好了
    virtual void whenWindowLoadFailed(const QString& message);    // 窗口加载失败的处理
    virtual void whenWindowAboutToDestroy(QQuickWindow* window);  // 窗口注销的处理
    virtual bool shouldAutoShow() const ;                       // 决定窗口是否加载完自动显示
    virtual bool shouldQuitOnClose() const;                     // 是否将窗口关闭关联到程序结束

private:
    bool addPageHelper(const QString &componentName, ViewModel *vm);
    // 绑定notify的使能信号
    void notifyEnable();
    // 处理创建窗口
    bool handleCreateResult(const resultST& res);

    // —————— 私有槽 策略控制 ——————
    Q_SLOT void onStatusChanged(resultST ret);

private:
    // Q_PROPERTY
    NotificationQueue * m_notifyQueue;

    // Member
    QQuickWindow * m_window;
    QHash<QString,ViewModel*> m_vmscache{};    // 缓存创建的页面vm

    // Sign
    bool m_shouldShow = false;
};
