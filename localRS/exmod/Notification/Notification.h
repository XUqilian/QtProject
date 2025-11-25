#pragma once

#include <QObject>
// #include <QProperty>
#include <QtQml/qqmlregistration.h>
#include <QWeakPointer>
#include <QSharedPointer>
#include <QString>
#include <QQueue>

//  qml 不允许构建 只能调用
class NotificationData : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Cannot create in QML because UNCREATABLE")
public:
    explicit NotificationData(QObject * parent = nullptr);
    explicit NotificationData(QString text , bool needret = false , int infolevel = 0 , QObject * parent = nullptr);

    // CONSTANT 限制qml只读  不加 使qml可读写
    Q_PROPERTY(QString content MEMBER m_content CONSTANT)
    Q_PROPERTY(int level MEMBER m_level CONSTANT)
    Q_PROPERTY(bool needRet MEMBER m_needRet CONSTANT)
    QString m_content;
    int m_level;
    bool m_needRet;

    Q_INVOKABLE void finishedTask(int result = 0);
signals:
    void finished(int result);
};

// 窗口单例，手动构建 因为需要在c++访问对象
class NotificationQueue : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Cannot create in QML because UNCREATABLE!")
public:
    explicit NotificationQueue(QObject * parent);

    QWeakPointer<NotificationData> addTask(QString text ,bool quickTask = false , bool needret = false , int infolevel = 0 );

    void uiReady(bool ready);             // 告诉当前类ui是否准备好，可以显示内容了
    void clearAll();                        // 销毁清理消息队列，未回复的直接置否并输出到qDebug
signals:
    void doTask(NotificationData*);         // 执行普通任务
    void doQuickTask(NotificationData*);    // 中断当前任务 执行紧急任务  可选择将旧任务重新放回队列

    // 有一个问题，如果这时候UI还没有显化或使用，那发出的信号是无人接收的，这样就永远不会结束，队列将一直阻塞

    Q_SLOT void taskEnd();
    Q_SLOT void quicktaskEnd();

private:
    void dowork();
    void doquickwork();

    // remove & delete task

private:
    bool Ready = false;

    QQueue<NotificationData*> tasks = {};  
    NotificationData* currentTask;

    QQueue<NotificationData*> quicktasks = {};
    NotificationData* currentQuickTask;

};



/* // 优化代码 但会增加阅读难度
    // const NotificationData& addTask(bool quickTask, const QString& text, bool needRet = false, int infoLevel = 0) {
    //     auto task = new NotificationData(text, infoLevel, needRet, this);
    //     if (quickTask) {
    //         enqueue(&m_quickTasks, &m_currentQuickTask, &NotificationManager::doQuickTask, task);
    //     } else {
    //         enqueue(&m_tasks, &m_currentTask, &NotificationManager::doTask, task);
    //     }
    //     return *task;
    // }
    // // 通用入队并触发处理函数
    // void enqueue(
    //     QQueue<NotificationData*>* queue,
    //     NotificationData** currentTask,
    //     void (NotificationManager::*emitSignal)(NotificationData*),
    //     NotificationData* task
    //     ) {
    //     queue->enqueue(task);
    //     processQueue(queue, currentTask, emitSignal);
    // }

    // // 通用出队并发射信号
    // void processQueue(
    //     QQueue<NotificationData*>* queue,
    //     NotificationData** currentTask,
    //     void (NotificationManager::*emitSignal)(NotificationData*)
    //     ) {
    //     if (*currentTask || queue->isEmpty()) return;

    //     *currentTask = queue->dequeue();
    //     connect(*currentTask, &NotificationData::finished, this, [this, currentTask, queue, emitSignal]() {
    //         taskFinished(currentTask, queue, emitSignal);
    //     });
    //     (this->*emitSignal)(*currentTask);
    // }

    // // 通用任务结束处理
    // template<typename QueuePtr, typename TaskPtr, typename SignalPtr>
    // void taskFinished(QueuePtr currentTaskPtr, QueuePtr queuePtr, SignalPtr signalPtr) {
    //     NotificationData* task = *currentTaskPtr;
    //     task->disconnect(this);
    //     task->deleteLater();
    //     *currentTaskPtr = nullptr;
    //     processQueue(queuePtr, currentTaskPtr, signalPtr);
    // }*/

/*
struct NotificationMsg
{
    Q_GADGET

public:
    using FinishedCallback  = std::function<void(int)>;

    explicit NotificationMsg(FinishedCallback  cb = nullptr) : m_callback(std::move(cb)) {}

    QProperty<int> level{0};
    QProperty<QString> content{"text"};

    Q_INVOKABLE void finishTask(int result) { if (m_callback) m_callback(result); }

private:
    FinishedCallback  m_callback;
};
*/
