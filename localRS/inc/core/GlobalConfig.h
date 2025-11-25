#pragma once
#include <QObject>
#include <QGuiApplication>
#include <manager/SettingsManager.h>
#include <Notification/Notification.h>
#include <manager/ViewManager.h>
#include <IGlobalManager.h>

class SettingItem;

// QCoreApplication::exit(0); // 立即抛出一个 QtExitException 异常，强制事件循环退出
// qApp->quit();  qApp宏指向 QGuiApplication 的单例实例 调用应用程序的 quit()
// QCoreApplication::quit(); 发送一个 Quit 事件到事件循环。事件循环在处理完当前事件后，会自然退出。

// 管理者不要向下提供服务
class GlobalConfig : public QObject
{
    Q_OBJECT
    // 这种全局类，即服务c++也服务QML 手动控制暴露会更好  而且设置上下文约等于注册单例 不需要注册
public:
    static GlobalConfig * instance(QGuiApplication * app = nullptr)
    {
        static GlobalConfig gc = GlobalConfig(app);
        return &gc;
    }

public:
    void runOnMainThread(std::function<void()> fn)
    {
        if (QThread::currentThread() == qApp->thread()) {
            fn();
        }else{
            QMetaObject::invokeMethod(instance(), fn, Qt::QueuedConnection);
        }
    }

private:
    explicit GlobalConfig(QObject * parent = nullptr) :QObject(parent){

        if (QThread::currentThread() != qApp->thread()) {   // 确保类在主线程执行
            qFatal("GlobalConfig must be constructed in the main thread!");   // 或 std::abort(); qFatal会调用abort
        }
        RegisterType();
        Init();
    }

    // 类型注册协助函数     // 已经使用宏注册的不需要再处理 此处仅用于有手动注册需求的
    void RegisterType();
    // 初始化函数
    void Init();            // 用于创建服务管理器对象

    /// 拓展组件只与数据实例关联，只是一对一，不管理如何组合如何暴露实例给qml
public:

    // for settings
    SettingsManager * setManager;
    Q_INVOKABLE SettingItem* item(const QString & key){return setManager->item(key);}
    // SettingsManager * setManager()const {return setManager;}

    // for windows
    ViewManager * viewManager;
    // ViewManager * viewManager()const{return viewManager;}

};
Q_DECLARE_METATYPE(GlobalConfig*)


// App.h
#include <QObject>
#include <QHash>
#include <QList>
#include <typeindex>
#include <functional>

class App;

class IService {
public:
    virtual ~IService() = default;
    // 服务自己决定如何注册到 QML
    virtual void registerToQml(App* app) = 0;
};

struct AccessToken {
    bool isValid() const { return true; } // 默认通过，后续可扩展
};

class App : public QObject {
    Q_OBJECT

public:
    explicit App(QObject* parent = nullptr);
    ~App() override;

    // ===== 1. 一行注册所有服务到 QML =====
    void registerQmlTypes();

    // ===== 2. 临时服务：类型安全添加/获取 =====
    template<typename T>
    void addTemporaryService(T* service);

    template<typename T>
    T* getTemporaryService() const;

    // ===== 3. 核心服务访问（带验证）=====
    // Settings* getSettings(const AccessToken& token = {});
    // NetworkService* getNetwork(const AccessToken& token = {});

    static AccessToken requestAccessToken();

    // ===== 4. 供服务调用的泛型注册接口 =====
    template<typename T>
    void registerSingleton(const char* uri, const char* typeName, std::function<T*()> factory);

private:
    bool validateAccess(const AccessToken& token) const;

    // 核心服务
    // Settings* m_settings = nullptr;
    // NetworkService* m_network = nullptr;

    // 所有实现 IService 的服务（用于自注册）
    QList<IService*> m_coreServices;

    // 临时服务：按类型索引
    QHash<std::type_index, QObject*> m_tempServices;

    // 防重复注册（可选）
    QHash<QByteArray, bool> m_registeredQmlTypes;
};

// App.cpp
App::App(QObject* parent)
    : QObject(parent)
{
    // 创建核心服务
    // m_settings = new Settings(this);
    // m_network = new NetworkService(m_settings, this);

    // 收集所有 IService（手动注册，或未来用工厂）
    // m_coreServices << m_settings << m_network;

    qDebug() << "App initialized with" << m_coreServices.size() << "core services.";
}

App::~App() = default;

// ===== 验证逻辑 =====
bool App::validateAccess(const AccessToken&) const {
    return true; // TODO: 后续替换为实际验证
}

// ===== 核心服务访问 =====
// Settings* App::getSettings(const AccessToken& token) {
//     return validateAccess(token) ? m_settings : nullptr;
// }

// NetworkService* App::getNetwork(const AccessToken& token) {
//     return validateAccess(token) ? m_network : nullptr;
// }

AccessToken App::requestAccessToken() {
    return AccessToken{};
}

// ===== 临时服务：类型安全 但在load后使用无效 =====
template<typename T>
void App::addTemporaryService(T* service) {
    static_assert(std::is_base_of_v<QObject, T>, "T must inherit QObject");
    if (!service) return;
    service->setParent(this);
    m_tempServices[std::type_index(typeid(T))] = service;
    qDebug() << "Added temporary service:" << typeid(T).name();
}

template<typename T>
T* App::getTemporaryService() const {
    auto it = m_tempServices.find(std::type_index(typeid(T)));
    if (it != m_tempServices.end()) {
        return qobject_cast<T*>(it.value());
    }
    return nullptr;
}

// 显式实例化常用临时服务（或按需在 .cpp 中实例化）
// 例如：template void App::addTemporaryService<DebugTool>(DebugTool*);
//       template DebugTool* App::getTemporaryService<DebugTool>() const;

// ===== 泛型 QML 注册 =====
template<typename T>
void App::registerSingleton(const char* uri, const char* typeName, std::function<T*()> factory) {
    static_assert(std::is_base_of_v<QObject, T>, "T must inherit QObject");

    QByteArray fullType = uri + QByteArrayLiteral(".") + typeName;
    if (m_registeredQmlTypes.contains(fullType)) {
        qWarning() << "QML type already registered:" << fullType;
        return;
    }

    qmlRegisterSingletonType<T>(uri, 1, 0, typeName,
                             [factory](QQmlEngine* engine, QJSEngine*) -> QObject* {
                                 T* obj = factory();
                                 if (obj) {
                                     obj->setParent(engine); // 绑定到 QML 引擎生命周期
                                 }
                                 return obj;
                             });

    m_registeredQmlTypes[fullType] = true;
}

// ===== 一键注册所有服务 =====
void App::registerQmlTypes() {
    // 1. 注册核心服务（通过自注册）
    for (IService* service : m_coreServices) {
        service->registerToQml(this);
    }

    // 2. 注册临时服务（自动暴露为 App.Temp 模块）
    for (auto it = m_tempServices.cbegin(); it != m_tempServices.cend(); ++it) {
        QObject* service = it.value();
        const char* typeName = service->metaObject()->className();
        // 简化类名（去掉命名空间）
        QByteArray simpleName = QByteArray(typeName).split(':').last();

        registerSingleton<QObject>("App.Temp", simpleName.constData(),
                                   [service]() -> QObject* { return service; });
    }
}
