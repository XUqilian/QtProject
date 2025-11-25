#ifndef VIEWMODEL_H
#define VIEWMODEL_H

#include <QtQmlCore/QtQmlCore>
#include <core/core.h>
// #include <QPair>

// QML_SINGLETON 宏只留给真正无状态、无依赖、纯计算的工具类
// 且宏需要搭配 qmlRegisterUncreatableType<Settings>(uri, 1, 0, name, "...");  才能正常在qml中使用
// 核心服务（Settings, Network...） → 用 单例类型（类型安全、模块化）；   运行前使用单例，运行后使用上下文；   因为单例注册必须在 engine.load() 前注册才有用
// 插件/临时/用户态服务 → 用 局部上下文属性（隔离、灵活、无污染）；

// 善于使用临时上下文，当需要的时候继承根上下文创建新的子上下文，在子上下文中注入一个个服务，然后在创建的时候注入到页面，当页面使用结束后释放子上下文对象即可

///////
// QML 组件在 create() 时若未指定上下文，则自动使用引擎的根上下文；
// 上下文一经创建即永久绑定、不可更改
// 即使后续被添加到其他页面或组件中，也无法访问父容器上下文中的任何内容，只能访问自己创建时上下文（及其继承链）中暴露的数据。

// 程序设计中尽量使用 全局单例/setProperty/暴漏对象到ui，全局单例提供公共服务，设置属性需要qml配合，多用于动态注入
// localContext局部上下文要保证各自内容不受干扰且自己的组件上下文应继承自己的，也就是隔离性要强，一般用于窗口级别的隔离（因为这个窗口一般不会访问那个窗口的内容）

// QObject::setProperty("propName", QVariant::fromValue(obj))       QQmlContext::setContextProperty("name", obj)




// 目前架构支持单component单实例的加载形式，要不无法实现数据隔离，也就是你创建一个viewmodel1 就只能为单个实例提供数据，有第二个时你需要创建viewmodel2，有大量的重复的蓝图缓存浪费
// 为qml组件提供模板和上下文对象管理 加载和创建实例的功能，为组件提供以vm为访问名的上下文设置
class ViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Cannot create in QML because UNCREATABLE!")

public:
    enum LoadSourceType{Null,Url,Modul};
    struct LoadSource {
        LoadSourceType type;
        QUrl url;
        QString qmlName, modulName;
        QQmlComponent::CompilationMode mode = QQmlComponent::PreferSynchronous;

        LoadSource(const LoadSource &) = default;
        LoadSource(LoadSource &&) = default;
        LoadSource &operator=(const LoadSource &) = default;
        LoadSource &operator=(LoadSource &&) = default;
        explicit LoadSource(const QUrl &qurl, QString qmlname)
            : url(qurl), qmlName(qmlname) {
            type = LoadSourceType::Url;
        }
        explicit LoadSource(QAnyStringView modulname, QAnyStringView qmlname, QQmlComponent::CompilationMode INmode = QQmlComponent::CompilationMode::PreferSynchronous)
            :modulName(modulname.toString()),qmlName(qmlname.toString()),mode(INmode){type = LoadSourceType::Modul;}

        LoadSource() : type(Null) {}

        static LoadSource defaultLoadSource(){
            LoadSource t("","");
            t.type = LoadSourceType::Null;
            return t;
        }
    };

    enum Status{ Error = -1 , Active, Checking, Loading, Ready, Destroying};    // 构造 检查 加载 准备 销毁       // 该标识只能用于向外部传递消息，不能与内部做判定
    Q_ENUM(Status)
public:

    /// @brief 需手动调用load进行初始化
    explicit ViewModel(QQmlEngine &engine, QObject *parent = nullptr)
        :QObject(parent),m_engine(engine)
    {
        Q_ASSERT_X(QThread::currentThread() == qApp->thread(),
                   "ViewModel::ViewModel", "Must be constructed on GUI thread!");

        m_componentContext = new QQmlContext(m_engine.rootContext(),this);
        m_status = Active;
        // QMetaObject::invokeMethod(this, &ViewModel::exposeToQmlContext, Qt::QueuedConnection);
        // QTimer::singleShot(0, this, [this]() { exposeToQmlContext(); });
        // exposeToQmlContext();
    };
    /// @brief 不提倡，因为可能程序运行快而导致QML构建结果信号无法接收
    explicit ViewModel(QQmlEngine &engine, const QUrl& url , QString& componentName, QObject *parent = nullptr)
        :QObject(parent),m_engine(engine)
    {
        Q_ASSERT_X(QThread::currentThread() == qApp->thread(),
                   "ViewModel::ViewModel", "Must be constructed on GUI thread!");

        // 创建专属上下文
        m_componentContext = new QQmlContext(m_engine.rootContext(),this);
        m_status = Active;

        loadUrl(url,componentName);
    };

    /// @brief 重新加载 QML源URL
    void loadUrl(const QUrl &url , QString componentName)
    {
        m_status = Checking;
        emit statusChanged(resultST{.success = true,.code = m_status});

        if (!url.isValid()) {
            m_status = Error;
            emit statusChanged(resultST{.success = false,.code = m_status,.message = "Checking:url isValid!"});
            return;
        }

        if (url.scheme().isEmpty())
        {
            m_status = Error;
            emit statusChanged(resultST{.success = false,.code = m_status,.message = "Checking:url isEmpty!"});
            return;
        }

        Init(LoadSource(url,componentName));
    }
    void loadQmlText(const QString& qmlText ,QString componentName)
    {
        m_status = Checking;
        emit statusChanged(resultST{.success = true,.code = m_status});

        if (qmlText.trimmed().isEmpty())
        {
            m_status = Error;
            emit statusChanged(resultST{.success = false,.code = m_status,.message = "Checking:qml Text isEmpty!"});
            return;
        }

        QByteArray data = qmlText.toUtf8();
        QUrl url = QUrl("data:text/plain;charset=utf-8;base64," + data.toBase64());

        Init(LoadSource(url,componentName));
    }
    void loadFromModule(QAnyStringView moduleUri, QString componentName, QQmlComponent::CompilationMode INmode = QQmlComponent::CompilationMode::PreferSynchronous)
    {
        m_status = Checking;
        emit statusChanged(resultST{.success = true,.code = m_status});

        Init(LoadSource(moduleUri,componentName,INmode));
    }
    void reLoad()
    {
        Init(m_src);
    }

    // 已值返回的形式处理可能的问题 该对象需要自行管理生命周期，会自动加入传入指针的对象树
    // 调用者必须传入 parent，否则行为未定义
    Q_INVOKABLE resultST createObj(QObject * parent )const
    {
        // std::unique_ptr<QObject> createObj(QObject * parent )const
        // return std::unique_ptr<QObject>(obj);

        resultST ret;
        if(getStatus() != Ready)    // m_component 可能还没有创建
        {
            ret.success = false;
            return ret << "Component not ready";
        }

        if (m_component->status() != QQmlComponent::Ready) {
            ret.success = false;
            return ret << "Component not ready, status:" << m_component->status();
        }

        QObject* obj = m_component->create(m_componentContext);

        if(obj)
        {
            if(parent) obj->setParent(parent);
            ret.success = true;
            ret.content = QVariant::fromValue(obj);
        }
        else
        {
            // 检测动态绑定时出现的错误
            ret.success = false;
            if (m_component->isError())
            {
                const QList<QQmlError> errors = m_component->errors();
                for (const QQmlError &error : errors) {
                    ret << "QML Error:" << error.toString();
                }
            } else {
                ret << "Unknown creation error (no error info)";
            }
        }

        return ret;
    }

    /// @brief qml对象模板访问接口 必须 QML 创建信号后才能访问 否则为空  该方法自定义上下文无效 将会继承控件父级的上下文
     Q_INVOKABLE const QQmlComponent * rootComponent() const { return m_component; }

    /// @brief qml对象模板上下文访问接口
     Q_INVOKABLE const QQmlContext * rootContext() const { return m_componentContext;}

    /// @brief qml组件名称 加载后访问 否则为空
    const QString & qmlName()const {return m_src.qmlName;}

    /// 返回当前状态
    Status getStatus() const{return m_status;}

    virtual ~ViewModel()
    {
        emit Disposed();

        // 2. 销毁组件上下文
        if (m_componentContext) {
            delete m_componentContext; // QQmlContext 建议直接 delete
            m_componentContext = nullptr;
        }

        cleanup(false);
    }

signals:    // const 信号约束 发送者（sender）在 emit 信号时，是否允许修改自身状态
    /// @brief VM对象注销
    void Disposed();

    /// @brief 状态
    void statusChanged(resultST ret);

protected:

    /// @brief 加载时默认将调用者指针通过setContextProperty绑定到独立Content上下文，创建时可通过设置上下文暴露给绑定的qml对象
    void exposeToQmlContext()
    {
        if (m_exposedToContext || !m_componentContext) return;
        m_componentContext->setContextProperty("vm", this);
        m_exposedToContext = true;
    }

    /// @brief 引擎访问接口
    QQmlEngine& engine() { return m_engine; }

    /// @brief 上下文属性管理接口，供子类添加自定义对象
    void addToContextProperty(const QString& name, QObject* object) {
        if (name.isEmpty() || !object) return;
        if (m_componentContext) {
            m_componentContext->setContextProperty(name, object);
        }
    }

    /// @brief QML 对象被注销时的处理事件，可在子类中重写 不能在构造和析构流程调用
    // virtual void onQmlBeforeDestroyed(QObject* object) = 0;

private:

    /// @brief 通过参数加载QML并初始化类内私有成员
    void Init(const LoadSource &src)
    {
        cleanup();

        if(src.type == Null) {
            m_status = Error;
            emit statusChanged(resultST{.success = false,.code = m_status,.message  = "Checking: argment err, LoadSource type is Null!"});
            return;
        }

        m_src = src;

        exposeToQmlContext();

        m_status = Loading;
        emit statusChanged(resultST{.success=true,.code = m_status});

        m_component = new QQmlComponent(&m_engine);

        QObject::connect(m_component, &QQmlComponent::statusChanged, this, [this](){
            if (m_component->status() == QQmlComponent::Ready)
            {
                m_status = Ready;
                emit statusChanged(resultST{.success = true,.code = m_status});
            }
            else if (m_component->status() == QQmlComponent::Error)
            {
                m_status = Error;
                emit statusChanged(resultST{.success = false,.code = m_status,.message  = "QML Component Error:" + m_component->errorString()});
                // qWarning() << "QML Component Error:" << m_component->errorString();
                cleanup();
            }

        });

        // 统一使用 QQmlComponent 加载
        if(src.type == LoadSourceType::Url) m_component->loadUrl( src.url);
        else if(src.type == LoadSourceType::Modul) m_component->loadFromModule(src.modulName,src.qmlName,src.mode);
        else {emit statusChanged(resultST{.success = false,.message = "unknow type src.type"}); return;}

    }

    /// @brief 清理所有加载过的对象
    void cleanup(bool emitSignals = true)
    {
        if (cleaning) return;
        cleaning = true;

        if(emitSignals){
            m_status =Destroying;
            emit statusChanged(resultST{.success = true,.code = m_status});
        }

        // 有一个问题是如果使用过程中通过addToContextProperty添加了其他的单属于某一次创建的属性，可能会一直延续创建，直到当前对象被销毁或者上下文被重建

        // 1. 断开连接
        if (m_component) {
            m_component->disconnect(this);
        }

        // 3. 销毁组件
        if (m_component) {
            m_component->deleteLater();
            m_component = nullptr;
        }

        m_src = LoadSource::defaultLoadSource();

        if(emitSignals){
            m_status =Active;
            emit statusChanged(resultST{.success = true,.code = m_status});
        }

        cleaning = false;
    }


    bool cleaning  = false;
    bool m_exposedToContext = false;
private:
    Status m_status = Error;
    LoadSource m_src = LoadSource();//LoadSource::defaultLoadSource();

    QQmlEngine& m_engine;       // QML应用引擎引用

    QPointer<QQmlComponent> m_component = nullptr;  // QML组件
    QPointer<QQmlContext> m_componentContext = nullptr; // 组件专属上下文

};

// 程序ui完全有c++持有window，每个组件在后台通过代码添加进主页面，后台代码持有该窗口对象/所有的后台组件对象都持有组件ui

class VMhelper
{
    /// ====== 将组件文件转为qrl方式 ====== ///
    // 1. 模块 + 组件名 → qrc 路径
    static QUrl toComponentUrl(const QString& module, const QString& componentName, QQmlEngine* engine)
    {
        if (module.isEmpty() || componentName.isEmpty() || !engine)
            return {};

        // 获取 import 路径列表（含默认路径）
        const QStringList importPaths = engine->importPathList();

        // 遍历每个 import 路径
        for (const QString& basePath : importPaths) {
            QDir moduleDir(basePath + "/" + module);
            if (!moduleDir.exists())
                continue;

            QFile qmldirFile(moduleDir.filePath("qmldir"));
            if (!qmldirFile.exists())
                continue;

            if (!qmldirFile.open(QIODevice::ReadOnly | QIODevice::Text))
                continue;

            // 解析 qmldir
            QTextStream in(&qmldirFile);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (line.isEmpty() || line.startsWith('#'))
                    continue;

                // 匹配: TypeName Major.Minor FileName.qml
                // 例如: LineChart 1.0 LineChart.qml
                static const QRegularExpression re(R"(^(\w+)\s+(\d+)\.(\d+)\s+(.+)$)");
                QRegularExpressionMatch match = re.match(line);
                if (match.hasMatch() && match.captured(1) == componentName) {
                    QString qmlFileName = match.captured(4);
                    QFileInfo qmlFile(moduleDir, qmlFileName);
                    if (qmlFile.exists()) {
                        return QUrl::fromLocalFile(qmlFile.absoluteFilePath());
                    }
                }
            }
        }

        // 未找到
        return {};
    }

    // 2. QML 源码文本 → data URL（base64 编码）
    static QUrl toComponentUrl(const QByteArray& qmlText)
    {
        if (qmlText.isEmpty())
            return {};
        // data:text/qml;charset=utf-8;base64,...
        QByteArray base64 = qmlText.toBase64();
        QString urlStr = "data:text/qml;charset=utf-8;base64," + QString::fromLatin1(base64);
        return QUrl(urlStr);
    }

    // 3. 文件路径（QString）→ file:// URL
    static QUrl toComponentUrl(const QString& fileName)
    {
        if (fileName.isEmpty())
            return {};
        QFileInfo fi(fileName);
        return QUrl::fromLocalFile(fi.absoluteFilePath());
    }


    // 从引擎获取根上下文创建子上下文,默认挂载到引擎上下文对象树中
    static QQmlContext * createChildContext(QQmlEngine* eg,QObject * parent = nullptr)
    {
        // 引擎与上下文能够互相访问 eg->rootContext / ct->engine
        if(parent)  return new QQmlContext(eg,parent);
        else        return new QQmlContext(eg,eg->rootContext());
    }

    // QQmlComponent(QQmlEngine *, QObject *parent = nullptr);
};


// component加载      ： 后台创建 - 前台ui显示 / 前台需要创建 - 后台创建 - 前台ui显示
// object加载         ： 后台创建 - 前台ui显示
// 新架构为解决之前 一模板对一实例 问题
class IModel:public QObject
{
    Q_OBJECT
public:
    explicit IModel(QQmlComponent * cp,QObject * parent);        // 初始化一个ui对象，并设置setProperty这个ui对象的某个属性
    // QQmlEngine* engine = qmlEngine(QObject);

    virtual ~IModel(){};


    // 首先创建本类的实例，然后再通过component创建ui实例，最后再注入；    就算是前台创建也需要先将需求转至后台；     除非不需要输出的component（仅显示某些内容不需要后台数据支撑）
    QQmlComponent * component;
    // QObject* view;          // 交由前台展示

    // stackview cp + vm + qmlpropertyname
    // obj + vm
    QVariantList getComponent();  // QPair<QQmlComponent*,IModel*> getcm();
    QVariantList getVModel();  // QPair<QObject*,IModel*> getom();

    // 属于组件的页面配置，可空，继承类自己实现即可   // 蓝图需要使用懒加载的方式，必须的常用的缓存，不常用的用到再加载，每个类根据自己的策略决定哪些缓存，
    // QMap<QString,QUrl>  urlMap{};     // 懒加载      使用QUrl url = QQmlMetaType::qmlTypeUrl("QtQuick", "Rectangle");获取唯一url
    // QMap<QUrl,QQmlComponent> cacheMap{};   // 预加载


    // virtual Q_INVOKABLE void event(int eventRole);   // do callback
    // virtual QStrinngList roles();                    // options
    // QMap<int,std::function<void>> rolesMap();        // callback map

    // QStringList err;
    // QStringList popAllErr() { return std::move(err);}  // 右值移动即可减少拷贝损耗，清理已存在的列表

    // 这个后续可以添加到蓝图管理中去
    // setComponentShared(QVariant value);  用于共享蓝图的某些属性，
    // QMap<int, QVariant> sharedMap;
};


// ui模板需要什么  蓝图类component + 数据类model  (连接器context必须默认从蓝图引擎构建)
// 最终对象是什么  QObject + 数据类实例





#endif // VIEWMODEL_H
