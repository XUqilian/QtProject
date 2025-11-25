#ifndef FUTURE_H
#define FUTURE_H

// qDebug()    << "调试信息";      // Debug 消息
// qWarning()  << "警告信息";      // 警告
// qCritical() << "严重错误";      // 致命错误
// qInfo()     << "普通信息";      // 一般信息（Qt 5.5+）

/*
// old design
// inherit ViewModelQ to create custom ViewPage, parent need set qml
class ViewModelQ : public QObject
{
    Q_OBJECT

private:
    const QQmlApplicationEngine & _engine;

    // SQPP aommand can create new attrib code
private:
    QString _name {""};
    Q_PROPERTY(QString name READ name WRITE nameSet NOTIFY nameChanged)
    QString name(){return _name;}
    void nameSet(QString & value){if(_name != value)
    {
        _name = value;
        emit nameChanged();
    }}
signals: void nameChanged();    // QML中绑定的是属性，事件是驱动其更新的

public:
    explicit ViewModelQ(QQmlApplicationEngine & engine, QObject *parent = nullptr) :_engine(engine), QObject(parent) {QQmlEngine::setObjectOwnership(this, QQmlEngine::JavaScriptOwnership);}

    virtual ~ViewModelQ(){}

signals: void Disposed(); // if you manage this memory , you should listen this signal to delete memory ...

};*/

/*
// inherit ViewModelC to create custom ViewPage, you should manage this memoryLife ,need set child qmlPage
class ViewModelC : public QObject
{
    Q_OBJECT

private:
    QQmlApplicationEngine & _engine;
    QPointer<QObject> View = nullptr;
    std::atomic<bool> IsQmlDeleted = false;

public:
    explicit ViewModelC(QQmlApplicationEngine &engine, QUrl qml, QObject *parent = nullptr)
        : QObject(parent), _engine(engine)
    {
        // only use QQmlEngine::CppOwnership

        changeView(qml);

        QWindow *window = qobject_cast<QWindow*>(View);
        if (window) window->show();
    }

    virtual ~ViewModelC()
    {
        deleteQml();
    }

    void changeView(QUrl qml)
    {
        deleteQml();

        QQmlComponent component(&_engine, qml);
        View = component.create();
        if (!View) throw std::runtime_error("Failed to create QML object");

        BindVVM(_engine,View,this);

        connect(View, &QObject::destroyed, this, [this](){
            if(!IsQmlDeleted)
            {
                IsQmlDeleted = true;
                View = nullptr;
                emit Disposed();
            } } );   // in destroying

        IsQmlDeleted = false;
    }


    // Q_PROPERTY 和 Q_INVOKABLE 分别暴露 属性 和 方法 给qml

public:void deleteQml()
    {
        if(!IsQmlDeleted && View)
        {
            IsQmlDeleted = true;
            View->deleteLater();
            View = nullptr;
            emit Disposed();
        }
    }


    signals: void Disposed();

private: void static BindVVM(QQmlApplicationEngine & engine , QObject * view,QObject * viewModel)
    {
        // this func only use QQmlEngine::CppOwnership model

        // if (QQmlEngine::contextForObject(VM) && QQmlEngine::contextForObject(VM)->engine() != &engine)
        // {
        //     throw std::runtime_error("BindVVM: View is managed by a different QML engine!");
        // }

        if(engine.rootObjects().isEmpty() || !view || !viewModel) throw std::runtime_error("engine is empty Or Parameter null pointer!");

        QQmlContext * qmlContext = new QQmlContext(engine.rootContext(), view);    // create a context obj, obj parent is View
        if (! qmlContext) throw std::runtime_error("Failed to create QQmlContext object");

        qmlContext->setContextProperty("vm", viewModel);                                // set context property vm, vm point to this

        QQmlEngine::setObjectOwnership(viewModel, QQmlEngine::CppOwnership);      // tell qml engine, c++ manage this

        QQmlEngine::setContextForObject(view, qmlContext);                         // set View context
    }

};*/

/*
// 宏生成 QProperty + QBindable 的解决方案
#define AUTO_PROPERTY(Type, Name) \
private: \
    QProperty<Type> _##Name; \
    public: \
    QBindable<Type> Name() { return &_##Name; } \
    Type Name##Value() const { return _##Name; } \
    Q_PROPERTY(Type Name##Value READ Name##Value BINDABLE Name)

// 宏生成Q_PROPERTY所需所有的解决方案
#define AUTO_Q_PROPERTY(Type, Name) \
private: \
    Type _##Name; \
    public: \
    Type Name() const { return _##Name; } \
    void set##Name(Type value) { \
        if (_##Name != value) { \
            _##Name = value; \
            emit Name##Changed(); \
    } \
} \
    Q_PROPERTY(Type Name READ Name WRITE set##Name NOTIFY Name##Changed) \
    signals: \
    void Name##Changed();

 */

/*
template<typename T>
class PROPERTY : public QObject {
    Q_OBJECT
public:
    using Getter = std::function<T()>;
    using Setter = std::function<void(const T&)>;

    Getter getter;
    Setter setter;

    PROPERTY(Getter g, Setter s , QObject* parent = nullptr) : QObject(parent) , getter(g), setter(s) {}
};
*/

/*
template<typename T>
class BPAdapter : public QObject {
    Q_OBJECT

public:
    explicit BPAdapter(PROPERTY<T> * property , QObject* parent = nullptr) : QObject(parent) , Property(property) {}

    T getter() const {
        return Property->getter();
    }

    void setter(const QString& value) {
        Property->setter(value);
    }

signals:
    void valueChanged();

private:
    PROPERTY<T>* Property;
};
*/

/*
#include <functional>
#include <map>
#include<iostream>

template<typename T>
class Ppty {
private:
    // 存储 getter 和 setter 的 lambda
    std::function<T()> m_getter;
    std::function<void(const T&)> m_setter;

    // 存储变更通知的回调（模拟信号）
    // std::vector<std::function<void(const T&)>> m_notifiers;  // ✅ 多个回调

    // Str_Tag
    std::map<std::string, std::function<void(const T&)>> m_notifiers;

    // Receiver(object*||void*) + function_Tag
    // std::map<std::pair<Receiver, uintptr_t>, std::function<void(const T&)>>> m_notifiers;

    // RAII
    // Connection onValueChanged(std::function<void(const T&)> cb);

    // 存储默认值（如果使用默认存储）
    T m_defaultStorage;

public:
    // 构造函数：接收 getter 和 setter lambda
    // 构造函数支持默认回调
    Ppty(
        std::function<T()> getter = nullptr,
        std::function<void(const T&)> setter = nullptr,
        std::function<void(const T&)> defaultNotifier = nullptr  // ✅ 新增
        )
        : m_getter(std::move(getter))
        , m_setter(std::move(setter))
        , m_defaultStorage(T{})
    {
        if (!m_getter) {
            m_getter = [this]() -> T { return m_defaultStorage; };
        }

        if (!m_setter) {
            m_setter = [this](const T& value) {
                m_defaultStorage = value;
                for (auto& cb : m_notifiers) {
                    cb(value);
                }
            };
        }

        // 如果提供了默认回调，自动添加
        if (defaultNotifier) {
            m_notifiers.push_back("default",std::move(defaultNotifier));
        }
    }

    // 读取值
    T value() const {
        return m_getter();
    }

    // 设置值
    void setValue(const T& value) {
        m_setter(value);
    }

    // 设置变更通知回调（相当于 connect）
    void AddCallback(const std::string &key,std::function<void(const T&)> callback) {
        if (callback )  m_notifiers.push_back("key",std::move(callback));
        // 还要判断是否有相同的项在里面
    }

    void RemoveCallback(const std::string &key)
    {
        // void * t = m_notifiers.at(key);
    }

    // 便捷：支持直接赋值（C++20 可用 operator auto）
    operator T() const { return value(); }
};
*/
#endif // FUTURE_H
