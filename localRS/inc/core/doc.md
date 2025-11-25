# Qt/C++ 与 QML 开发规范与最佳实践

> 📌 **适用版本**：Qt 6.2+（部分功能需更高版本）  
> 💡 本文档涵盖内存管理、职责划分、类型注册、宏使用、最佳实践等核心内容，适用于团队协作与项目规范制定。

---

## 🧩 内存管理

- **每个使用 `new` 的指针**：
  - 必须绑定父对象（`QObject` 树管理），或
  - 使用智能指针：`std::make_unique<T>()`、`std::make_shared<T>`、`QSharedPointer<T>`、`QScopedPointer<T>` 等。
- **临时堆分配**：使用 RAII 方式管理资源。
- **管理类设计原则**：
  - 不向被管理类提供支持逻辑。
  - 减少反向依赖，解除管理类与被管理类之间的耦合。

---

## 🖼️ QML 与 C++ 职责划分

| 层级 | 职责 |
|------|------|
| **QML** | 仅负责 UI 显示 + 最简单的逻辑判断 |
| **C++** | 提供数据和复杂业务逻辑 |
| **VM 层（可选）** | 仅提供接口 + C++ 数据点集合，作为中间层 |


---

## 🔍 QML 引擎标识符解析顺序

QML 引擎在解析标识符时按以下优先级查找：

1. **本地 `id`**
2. `setContextProperty` 注册的上下文属性
3. `qmlRegisterType` 等注册的类型
4. JavaScript 变量/函数

---

## 🏷️ 命名与注册规范

- **暴露对象名**：全小写（如 `app`, `backend`）
- **类型注册名**：首字母大写（如 `UserModel`, `NetworkService`）
- **大型项目**：可添加特定前后缀区分模块或功能
- **全局上下文暴露**：
  - 通过单一 `app` 对象统一暴露。
  - 每个暴露的 `QProperty` 属性对应一个子对象。
  - 访问方式：`app.ContextName`，全局仅暴露一个 `app` 对象。

---

## 📦 宏注册一览表

| 宏 | 类型 | 功能说明、限制与注意事项 | 支持类/结构 |
|----|------|--------------------------|-------------|
| `QML_ELEMENT` | 非调用式 | 自动注册类型为 QML 可实例化类型，等价于 `qmlRegisterType`。需继承 `QObject`。 | 类 |
| `QML_SINGLETON` | 非调用式 | 自动注册为 QML 单例，等价于 `qmlRegisterSingletonType`。需继承 `QObject`。 | 类 |
| `QML_INTERFACE` | 非调用式 | 声明接口类型，配合 `Q_INTERFACES` 使用，用于 QML 多态。 | 类 |
| `QML_VALUE_TYPE(TypeName)` | 调用式 | 注册值类型（如结构体），可在 QML 使用但不可构造。需 `Q_GADGET` 或 `Q_ENUM`。⚠️ **必须与 `Q_GADGET` 一起使用**，否则编译报错。 | 结构 |
| `QML_NAMED_ELEMENT(name)` | 调用式 | 指定 QML 中使用的类型名，配合 `QML_ELEMENT` 使用。 | 类 |
| `QML_UNCREATABLE(message)` | 调用式 | 标记类型不可在 QML 构造，但可访问成员。需提供原因信息。 | 类 |
| `QML_ANONYMOUS` | 调用式 | 将类型注册为匿名类型，不暴露全局名。用于内部组件。 | 类 |
| `QML_SINGLETON_UNCREATABLE(message)` | 调用式 | 标记单例类型不可在 QML 中手动创建。 | 类 |
| `QML_FOREIGN` | - | 将一个非 `QObject` 派生的类（如 `QPointF`）关联到一个已注册的 QML 类型上，使其可在 QML 中使用。常用于包装标准类型。需配合 `qmlRegisterType` 使用。 | 不一定需继承 `QObject` |
| `QML_ATTACHED(Type)` | 调用式 | 注册附加类型（如 `Drag.active`），允许在 QML 中为任意对象附加属性和行为。需提供 `create()` 静态函数。 | 类 |

---

## 🔧 宏注册通用搭配方案

| 场景 | 推荐宏组合 |
|------|-----------|
| **结构体（QML 使用）** | `Q_GADGET + Q_DECLARE_METATYPE + QML_VALUE_TYPE` |
| **可实例化对象** | `Q_OBJECT + QML_ELEMENT` |
| **单例对象** | `Q_OBJECT + QML_ELEMENT + QML_SINGLETON` |
| **不可实例化类（仅枚举/常量）** | `Q_OBJECT + QML_ELEMENT + QML_UNCREATABLE` |
| **接口类** | `Q_GADGET / Q_OBJECT + QML_INTERFACE` |
| **隐式共享类** | `Q_OBJECT + Q_DECLARE_SHARED + d-pointer` |
| **带私有类的类** | `Q_OBJECT + Q_DECLARE_PRIVATE / Q_DECLARE_PUBLIC` |

> ⚠️ **原理**：所有宏均为**正交设计**：缺少任一宏，对应功能将无法生效。例如，缺少 `Q_GADGET`，`QML_VALUE_TYPE` 将无法注册类型。

---

## 🔧 宏功能详解

### 1. 元对象系统基础

| 宏 | 功能说明 | 限制与注意事项 |
|-----|----------|----------------|
| `Q_OBJECT` | 启用元对象功能（信号槽、属性等） | 仅 `QObject` 派生类可用；需 moc 预处理；必须在头文件中声明 |
| `Q_GADGET` | 为非 `QObject` 类提供部分元功能（属性、枚举） | 不支持信号槽；需 moc 预处理 |
| `Q_DECLARE_METATYPE<T>` | 声明类型可用于元对象系统（配合 `qRegisterMetaType`） | 需在全局作用域；类型需满足 `qRegisterMetaType` 要求 |
| `Q_MOC_RUN` | 标记 moc 生成代码的编译范围 | 内部宏，用户代码一般无需直接使用 |

---

### 2. 属性与成员

| 宏 | 功能说明 | 限制与注意事项 |
|-----|----------|----------------|
| `Q_PROPERTY(...)` | 声明类属性，支持 `READ`/`WRITE`/`NOTIFY` 等 | 格式：`类型 名称 READ 读函数 [WRITE 写函数] [NOTIFY 信号]` |
| `Q_INVOKABLE` | 标记函数可被元对象系统调用（如 QML 调用） | 仅 `Q_OBJECT` 派生类可用 |
| `Q_SIGNAL` / `signals:` | 声明信号函数 | 仅 `Q_OBJECT` 类可用；无需实现，由 moc 生成 |
| `Q_SLOT` / `slots:` | 声明槽函数 | 仅 `Q_OBJECT` 类可用；可被信号触发 |
| `Q_ENUM(枚举名)` | 注册枚举到元系统，支持 QML 访问 | 需在 `Q_OBJECT`/`Q_GADGET` 类中；枚举需在类内定义 |
| `Q_ENUM_NS(枚举名)` | 注册命名空间中的枚举到元系统 | C++11 命名空间内枚举专用；需配合 `Q_DECLARE_METATYPE` |
| `Q_FLAG(标志枚举)` | 注册可作为位运算的枚举（如 `Qt::Alignment`） | 支持 OR 等位操作 |
| `Q_FLAGS(标志列表)` | 注册多个标志枚举（Qt 5 及之前） | Qt 6 推荐使用 `Q_FLAG` |

---

### 3. QML 集成

| 宏 | 功能说明 | 限制与注意事项 |
|-----|----------|----------------|
| `QML_ELEMENT` | 自动注册类为 QML 可用类型（无需 `qmlRegisterType`） | Qt 6.2+；需 `Q_OBJECT`；依赖模块配置（`.qmltypes`） |
| `QML_NAMED_ELEMENT(name)` | 为 QML 类型指定自定义名称（替代类名） | 必须与 `QML_ELEMENT` 配合使用 |
| `QML_UNCREATABLE("reason")` | 标记 QML 类型不可实例化 | 必须与 `QML_ELEMENT`/`QML_NAMED_ELEMENT` 配合 |
| `QML_SINGLETON` | 标记类为 QML 单例 | 需与 `QML_ELEMENT` 配合；类需有静态 `create()` 方法 |
| `QML_VALUE_TYPE` | 标记类为 QML 值类型（值语义，而非引用） | 用于非 `QObject` 派生类型；支持复制而非指针传递 |
| `QML_ADDED_IN_MINOR(minor)` | 指定类型在模块次要版本中添加 | 配合 `QML_ELEMENT`；用于版本控制 |
| `QML_REMOVED_IN_MINOR(minor)` | 指定类型在模块次要版本中移除 | 配合 `QML_ELEMENT`；用于版本控制 |
| `QML_EXTENDED(ExtensionType)` | 为 QML 类型添加扩展方法（通过扩展类） | 扩展类需包含 `qmlExtendedTypes` 静态函数 |
| `QML_IMPLEMENTS_INTERFACE(Interface)` | 声明类实现 QML 接口 | 需与 `QML_ELEMENT` 配合；接口需通过 `qmlRegisterInterface` 注册 |
| `QML_ATTACHED(Type)` | 注册附加类型，允许在 QML 中为任意对象附加属性和行为 | 需提供 `create()` 静态函数 |

---

### 4. 其他功能

| 宏 | 功能说明 | 限制与注意事项 |
|-----|----------|----------------|
| `Q_DECLARE_PRIVATE(Class)` | 声明 PIMPL 模式的私有实现类 | 需在类中定义 `d_ptr`；配合 `Q_D` 宏使用 |
| `Q_DECLARE_PUBLIC(Class)` | 声明 PIMPL 模式中私有类引用的公有类 | 需在私有类中定义 `q_ptr`；配合 `Q_Q` 宏使用 |
| `Q_DISABLE_COPY(Class)` | 禁用拷贝构造和赋值运算符 | 放在类的私有成员区域 |
| `Q_REQUIRED_RESULT` | 强制函数返回值必须被使用（否则警告） | 用于强调返回值重要性（如错误码） |
| `Q_OVERRIDE` | 显式标记函数重写基类虚函数（Qt 5） | Qt 6 推荐使用 C++11 的 `override` 关键字 |
| `Q_DECL_DEPRECATED` | 标记函数/类为已过时 | 编译器会对使用过时接口的代码警告 |
| `Q_DECLARE_INTERFACE` | 声明接口类型，用于 `Q_INTERFACES`，实现多态 | - |
| `Q_DECLARE_METATYPE` | 使自定义类型可被元对象系统识别，用于信号槽传参 | - |
| `Q_DECLARE_FLAGS` | 声明标志位类型，组合 `QFlags` 使用 | - |
| `Q_DECLARE_NON_MOVABLE` | 禁止类的移动构造和移动赋值 | - |
| `Q_DECLARE_SHARED` | 启用隐式共享（写时复制），需有 d 指针 | - |
| `Q_DECLARE_WEAK_POINTER` | 为类型生成 `QWeakPointer` 支持 | - |

---

## 🔄 函数注册表（类型 / 组件注册）

| 函数 | 功能说明、限制与注意事项 | 支持类/结构 |
|------|--------------------------|-------------|
| `qmlRegisterType(uri, major, minor, qmlName)` | 注册可在 QML 中实例化的 C++ 类型。需继承 `QObject`，含 `Q_OBJECT`，有默认构造函数。 | 类 |
| `qmlRegisterUncreatableType(uri, major, minor, qmlName, reason)` | 注册不可实例化的类型，用于暴露枚举或静态属性。需继承 `QObject`，指定无法创建的原因。 | 类 |
| `qmlRegisterSingletonType(uri, major, minor, typeName, callback)` | 注册单例类型，通过回调函数创建唯一实例。**实例由回调函数创建，生命周期由 QML 引擎管理**。 | 类 |
| `qmlRegisterSingletonInstance(uri, major, minor, typeName, instance)` | 注册已存在的 C++ 对象为 QML 单例。**对象必须在引擎销毁前保持有效，否则导致未定义行为**。 | 类 |
| `qmlRegisterAnonymousType(uri, major, minor)` | 注册匿名类型，不暴露全局名，用于内部属性或列表。仅 C++/元系统使用。 | 类 |
| `qmlRegisterInterface(interfaceName)` | 注册接口类型，用于 QML 中多态和类型转换。实现类需使用 `Q_INTERFACES`。 | 类 |
| `qmlRegisterModule(uri, version)` | 注册模块元信息（设计时数据），不注册具体类型。Qt 6.2+。 | 模块 |
| `qRegisterMetaType<T>()` | 向元系统注册类型，支持信号槽传递。需 `Q_DECLARE_METATYPE`，类型需有公有默认构造、拷贝构造。 | 类、结构 |
| `qRegisterMetaTypeStreamOperators<T>()` | 注册支持 `QDataStream` 序列化的类型。需实现 `operator<<` 和 `operator>>`。 | 类、结构 |
| `qmlRegisterRevision(uri, major, minor, revision)` | 为类型注册版本修订号，支持 QML 版本检查。需配合 `qmlRegisterType` 使用。 | 类 |
| `qmlRegisterTypeNotAvailable(uri, major, minor, qmlName, reason)` | 注册占位类型，标记该类型不可用。用于版本兼容或禁用类型。 | 类 |
| `qmlRegisterValueTypeEnabler(enable)` | 启用值类型在 QML 中的值语义（如复制而非引用）。用于非 `QObject` 派生的值类型。 | 类、结构 |
| `qmlRegisterUncreatableMetaObject(metaObject, uri, major, minor, qualifier, reason)` | 注册不可实例化的元对象，暴露命名空间中的枚举/常量。禁止构造，仅访问。 | 针对命名空间、静态成员或元对象，非类实例 |

---

## 🔗 关键关联说明

- `qmlRegisterUncreatableMetaObject` 与命名空间枚举：
  - 需配合 `Q_ENUM_NS`，将命名空间中的枚举暴露给 QML，但禁止实例化。
- `QML_VALUE_TYPE` 与值语义：
  - 用于类似 `QPoint`、`QRect` 的非 `QObject` 类型，在 QML 中赋值时会**复制而非引用**。
- **所有 QML 相关宏**（如 `QML_UNCREATABLE`、`QML_SINGLETON`）均依赖 `QML_ELEMENT` 或 `QML_NAMED_ELEMENT`，因为它们需要通过元对象系统完成 QML 类型注册。
- `Q_DECLARE_METATYPE` 与 `qRegisterMetaType<T>()`：
  - `Q_DECLARE_METATYPE(T)`：声明类型可被 `QVariant` 存储。
  - `qRegisterMetaType<T>()`：将类型注册到 Qt 元对象系统，支持跨线程信号传递。
  - ✅ **两者常配合使用**：先 `Q_DECLARE_METATYPE`，再调用 `qRegisterMetaType<T>()`。

---

## 💡 单例获取方式

```cpp
// 通过引擎获取单例指针
// 需确保类型已通过 qmlRegisterSingletonType 或 QML_SINGLETON 正确注册。
TypeName* singleton = engine->singletonInstance<TypeName*>("TypeName");
```

---

## 🎯 如何选择类型注册方式？

| 需求 | 推荐方案 |
|------|----------|
| 简单数据结构（如 `Point`、`Rect`） | `Q_GADGET + QML_VALUE_TYPE` |
| 可实例化的业务对象 | `Q_OBJECT + QML_ELEMENT` |
| 全局配置/服务 | `Q_OBJECT + QML_SINGLETON` |
| 仅暴露枚举/常量 | `Q_OBJECT + QML_UNCREATABLE` |
| 解耦数据传递（C++ ↔ QML） | `operator QVariant()` + `QVariantMap` |

---

## ❌ 常见错误与解决方案

| 错误现象 | 可能原因 | 解决方案 |
|---------|---------|----------|
| QML 报错 `Unknown type` | 类型未注册 | 使用 `qmlRegisterType` 或 `QML_ELEMENT` |
| QML 无法访问属性 | 缺少 `Q_PROPERTY` | 添加 `Q_PROPERTY(... MEMBER xxx)` |
| 信号传参失败 | 类型未注册到元系统 | 添加 `Q_DECLARE_METATYPE` + `qRegisterMetaType<T>()` |
| 单例无法访问 | 未正确注册或名称错误 | 检查 `qmlRegisterSingletonType` 参数 |

---

## 📌 Qt 版本支持说明

| 功能 | 最低 Qt 版本 |
|------|-------------|
| `QML_ELEMENT`, `QML_SINGLETON` | 6.2 |
| `QML_VALUE_TYPE` | 6.4 |
| `QML_ADDED_IN_MINOR` | 6.5 |
| 函数式注册（如 `qmlRegisterType`） | 所有版本 |
