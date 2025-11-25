#include <manager/WindowCtrler.h>
#include <core/gsc.h>
#include <core/GlobalConfig.h>
#include <view/mainview.h>
#include <view/settings.h>
#include <QQuickWindow>

WindowCtrler::WindowCtrler(QQmlEngine& engine,QObject * parent):ViewModel(engine,parent)
{
    m_notifyQueue = new NotificationQueue(this);
    connect(this, &ViewModel::statusChanged, this, &WindowCtrler::onStatusChanged);
    QTimer::singleShot(0, this, &WindowCtrler::init);
}

void WindowCtrler::onStatusChanged(resultST ret)
{
    if (!ret.success) {
        whenWindowLoadFailed(ret.message);
        return;
    }

    handleCreateResult(createObj(this));

}

// 私有辅助函数：安全地从 resultST 创建并设置窗口
bool WindowCtrler::handleCreateResult(const resultST& res)
{
    if (!res.success) {
        whenWindowLoadFailed(res.message);
        return false;
    }

    QObject* obj = res.content.value<QObject*>();
    if (!obj) {
        whenWindowLoadFailed("Pointer lost in QVariant conversion!");
        return false;
    }

    QQuickWindow* window = qobject_cast<QQuickWindow*>(obj);
    if (!window) {
        whenWindowLoadFailed("Loaded root is not a QQuickWindow!");
        return false;
    }

    // 替换旧窗口（防御性）
    if (m_window) {
        whenWindowAboutToDestroy(m_window);
        m_window->setVisible(false);
        m_window->disconnect();
        m_window->deleteLater();
    }

    m_window = window;
    whenWindowReady(window);

    if (m_shouldShow || shouldAutoShow()) {
        window->show();
    }

    return true;
}

void WindowCtrler::show()
{
    // 这里应该判定状态是否正常，如果是active就等于还没有加载主页内容，如果error则是加载出现问题
    m_shouldShow = true;

    if(m_window) m_window->show();
    else{
        auto status = getStatus();
        if (getStatus() == ViewModel::Ready)
        {
            handleCreateResult(createObj(this));
        }
        else if (status == ViewModel::Error)
        {
            reLoad();   // 重新尝试一下
        }
        else if (status == ViewModel::Active)
        {
            whenWindowLoadFailed("You need call Load before show()!");
        }
        // loading checking 等着就好了
    }

}

// —————— 可重写默认行为 ——————
void WindowCtrler::init()
{

    // 加载默认的设置页
    // ViewModel * loading = new ViewModel(m_engine,this);
    // addPage("loading",GSC::DefaultLoadingPage);
    // ViewModel * loadfailed = new ViewModel(m_engine,this);
    // addPage("loadfailed",GSC::DefaultLoadErrPage);


}

void WindowCtrler::whenWindowReady(QQuickWindow* window)
{
    notifyEnable();

    if (shouldQuitOnClose()) {
        connect(window, &QQuickWindow::closing, []() {
            QCoreApplication::quit();
        });
    }
}

void WindowCtrler::notifyEnable()
{
    // 设置一个标识位，如果没有启用，就将每一个内容推送到控制台输出，并将反馈设为取消
    connect(m_window, &QQuickWindow::visibilityChanged, this,
            [this](QWindow::Visibility visibility) {
                m_notifyQueue->uiReady(visibility != QWindow::Hidden);
            });

    connect(m_window, &QQuickWindow::destroyed, this, [this]() {
        m_notifyQueue->clearAll();
        m_window = nullptr;
    });
}

bool WindowCtrler::shouldQuitOnClose() const
{
    return false; // 默认不退出
}

void WindowCtrler::whenWindowLoadFailed(const QString& message)
{
    qWarning() << "Window load failed:" << message;
}

void WindowCtrler::whenWindowAboutToDestroy(QQuickWindow* /*window*/)
{
    // 默认无操作
}

bool WindowCtrler::shouldAutoShow() const
{
    return m_shouldShow;   // 默认不显示
}


// —————— 页面管理 ——————
void WindowCtrler::addPage(const QString &componentName, ViewModel *vm)
{
    if(vm == nullptr)
    {
        emit loadQmlResult(componentName,resultST{.success=false,.message = "vm not ready!"});
        return;
    }

    if(m_vmscache.contains(componentName))
    {
        emit loadQmlResult(componentName,resultST{.success=false,.message = "componentName Repeat!"});
        return;
    }

    m_vmscache.emplace(componentName,vm);
    emit pagesChanged();
}

void WindowCtrler::addPage(const QString &componentName, const QUrl &url, ViewModel *vm)
{
    if(addPageHelper(componentName,vm))
    {
        vm->loadUrl(url,componentName);
    }
}

void WindowCtrler::addPage(const QString &componentName, const QString &qmlText, ViewModel *vm)
{
    if(addPageHelper(componentName,vm))
    {
        vm->loadQmlText(qmlText,componentName);
    }
}

void WindowCtrler::addModulePage(const QString &componentName, const QAnyStringView &moduleUri, ViewModel *vm, QQmlComponent::CompilationMode INmode)
{
    if(addPageHelper(componentName,vm))
    {
        vm->loadFromModule(moduleUri,componentName,INmode);
    }
}

void WindowCtrler::releasePage(const QString &componentName)
{
    ViewModel * var = m_vmscache.value(componentName);
    if(var)
    {
        m_vmscache.remove(componentName);
        emit pagesChanged();
        var->deleteLater();
    }
}

bool WindowCtrler::addPageHelper(const QString &componentName, ViewModel *vm)
{
    if(m_vmscache.contains(componentName))
    {
        vm->deleteLater();
        emit loadQmlResult(componentName,resultST{.success=false,.message = "componentName Repeat!"});
        return false;
    }

    if(vm) vm->setParent(this);
    else vm = new ViewModel(engine(),this);

    connect(vm,&ViewModel::statusChanged,this,[&](resultST ret){
        if(ret.success)
        {
            emit loadQmlResult(componentName,resultST{.success = true,.message = ret.message});
            m_vmscache.emplace(componentName,vm);
            emit pagesChanged();

        }else
        {
            emit loadQmlResult(componentName,resultST{.success = false,.message = ret.message});
            vm->deleteLater();
        }
    });

    return true;
}

ViewModel* WindowCtrler::getpage(const QString &pageName)
{
    if(m_vmscache.contains(pageName))
        return m_vmscache.value(pageName);
    else {
        qDebug() << "page get failed:" + pageName;
        return nullptr;
    }
}

