#pragma once

#include <QVariant>
#include <QtQml/qqmlregistration.h>

// 常见职责角色分类：
// Service：提供通用能力（日志、网络、认证） 功能提供，提供可复用的能力
// Manager：管理资源生命周期（窗口、页面、缓存）
// Model：数据建模与业务逻辑（MVVM）
// Controller：协调流程，处理用户指令（MVC）
// Repository：封装数据访问（数据库、API）
// Provider：被动供应资源（配置、主题）
// Helper / Utility：无状态工具函数
// Factory：创建复杂对象
// Observer / Listener：响应事件
// Adapter：接口转换
// Decorator：动态增强功能
// Gateway：访问外部系统（如 REST 接口封装） 业务层抽象，封装外部服务协议（如 REST API）
// Wrapper：技术层适配，解决接口不兼容、简化调用

// service, gateway, repository：数据流下游，分开放，体现层次。
// controller, manager：控制层，按“单实例”与“全局”分离。
// model 独立，被多方依赖。
// view 独立，对应 UI。

// 数据与显示不要直连，中间需要提供接口，
// 需要设置数据  -》 访问设置接口  -》 调用对应的设置服务获取
// 需要数据库数据 -》 访问数据库接口 -》 调用指定的数据库获取数据       // 接口对象可能是mysql也可能是sqlserver。。。继承接口的实现

struct resultST {
    Q_GADGET
    QML_VALUE_TYPE(resultST)

    Q_PROPERTY(bool success MEMBER success FINAL)
    Q_PROPERTY(int code MEMBER code FINAL)
    Q_PROPERTY(QString message MEMBER message FINAL)
    Q_PROPERTY(QVariant content MEMBER content FINAL)

public:
    bool success = true;
    int code = 0;
    QString message = "";
    QVariant content{};

    template<typename T>
    resultST& operator<<(const T& val) {
        QVariant var = QVariant::fromValue(val);
        if (var.canConvert<QString>()) message += var.toString();
        else  message += QStringLiteral("Unknown!");
        return *this;
    }

};
Q_DECLARE_METATYPE(resultST)

QDebug operator<<(QDebug debug, const resultST &result);

// struct Result{
//     bool success = true;
//     int code = 0;
//     std::string message = "OK";
//     operator QVariant() const {
//         QVariantMap map;
//         map["success"] = success;
//         map["code"] = code;
//         map["message"] = QString::fromStdString(message);
//         return map;
//     }
//
//     // // in qml
//     // // ✅ 构造一个 JS 对象，结构和 Result 一样
//     // var result = {
//     //     success: false,
//     //     code: 404,
//     //     message: "Not Found"
//     // }
//     // // ✅ 直接作为 QVariant 发送（自动转换）
//     // obj.resultSent(result)
//     // // in c++
//     // void onReceived(const QVariant& var) {
//     //     QVariantMap map = var.toMap();
//     //     bool success = map["success"].toBool();
//     //     int code = map["code"].toInt();
//     //     QString message = map["message"].toString();
//     // }
//     // emit onsignal(QVariant(Result))
// };
// Q_DECLARE_METATYPE(Result)
