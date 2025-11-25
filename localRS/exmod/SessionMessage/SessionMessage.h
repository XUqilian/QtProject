#pragma once
#include <QObject>
#include <QtQml/qqmlregistration.h>
#include <QString>
#include <QUrl>
#include <QFile>
#include <QDebug>

 // qRegisterMetaType<SessionMessage>("SessionMessage")     注册到元对象系统，QML 才能识别类型名
class SessionMessage : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Cannot create in QML because UNCREATABLE!")
    // QML_VALUE_TYPE(SessionMessage)          // 自动注册到QML 不允许QML创建实例
public:
    enum class MessageType {
        Text,
        Image,
        Music,
        Video,
        File
    };
    Q_ENUM(MessageType)

    enum class Status {
        Unsent,
        Sending,
        Sent,
        Failed
    };
    Q_ENUM(Status)

    // 不可变字段：直接暴露
    Q_PROPERTY(bool isSender MEMBER m_isSender CONSTANT)
    Q_PROPERTY(MessageType type MEMBER m_type CONSTANT)
    Q_PROPERTY(QString text MEMBER m_text CONSTANT)
    Q_PROPERTY(QUrl contentUrl MEMBER m_contentUrl CONSTANT)
    Q_PROPERTY(qint64 timestamp MEMBER m_timestamp CONSTANT)
    bool m_isSender = false;
    MessageType m_type = MessageType::Text;
    QString m_text = "text";
    QUrl m_contentUrl;
    qint64 m_timestamp = 0;


private:
    Status m_status = {};
public:
    Q_PROPERTY(Status status READ status WRITE statusSet NOTIFY statusChanged FINAL)
    Status status() const { return m_status; }
    void statusSet(const Status & obj) {
        if(m_status != obj) {
            m_status = obj;
            emit statusChanged();
        }}
signals: void statusChanged();

};

// 专属于 SessionMessage 的操作工具类  单例模式 因为当前类不需要在c++进行操作
class SessionMessageTools : public QObject {
    Q_OBJECT
    QML_SINGLETON
    QML_UNCREATABLE("Cannot create in QML because SINGLETON")
public:
    explicit SessionMessageTools(QObject *parent = nullptr);

    // 静态函数：保存消息中的文件
    Q_INVOKABLE static bool copyFile(const QUrl &sourceUrl, const QUrl &targetUrl);
    // 可扩展其他操作
};
