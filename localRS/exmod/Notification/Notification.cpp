#include<Notification/Notification.h>
#include <QDebug>

NotificationData::NotificationData(QObject *parent): QObject(parent){}

NotificationData::NotificationData(QString text, bool needret, int infolevel, QObject *parent)
    : QObject(parent) , m_content(text),m_level(infolevel) , m_needRet(needret) {}

void NotificationData::finishedTask(int result){ if(m_needRet) emit finished(result);}



NotificationQueue::NotificationQueue(QObject *parent) : QObject(parent) {}

QWeakPointer<NotificationData> NotificationQueue::addTask(QString text, bool quickTask, bool needret, int infolevel)         // 必须从这里创建，返回的也只是一个const引用，确保不被修改和delete
{
    NotificationData* task = new NotificationData(text,infolevel,needret,this);
    if(quickTask)
    {
        quicktasks.enqueue(task);
        doquickwork();
    }else
    {
        tasks.enqueue(task);
        dowork();
    }
    // auto shared = QSharedPointer<NotificationData>(task);
    return QWeakPointer<NotificationData>(QSharedPointer<NotificationData>(task));
}

void NotificationQueue::uiReady(bool ready)
{
    Ready = ready;
    if(ready)
    {
        doquickwork();
        dowork();
    }
}

void NotificationQueue::clearAll()
{
    Ready = false;

    while(!quicktasks.empty())
    {
        auto var = quicktasks.dequeue();
        qDebug() << var->m_content;
        emit var->finished(0);
        var->deleteLater();
    }
    while(!tasks.empty())
    {
        auto var = tasks.dequeue();
        qDebug() << var->m_content;
        emit var->finished(0);
        var->deleteLater();
    }
}

void NotificationQueue::dowork()
{
    if(!Ready && currentTask && tasks.empty()) return;
    // 是否需要控制队列大小，当队列超过十个的时候，如果ui没有准保好，直接将第一个丢到控制台输出

    currentTask = tasks.dequeue();
    connect(currentTask,&NotificationData::finished,this,&NotificationQueue::taskEnd);
    emit doTask(currentTask);       // 如果没有接收者怎么办，一直阻塞再这里吗？

}

void NotificationQueue::taskEnd()
{
    currentTask->disconnect();
    currentTask->deleteLater();
    currentTask = nullptr;
    dowork();
}

void NotificationQueue::doquickwork()
{
    if(!Ready && currentQuickTask && quicktasks.empty()) return;

    currentQuickTask = tasks.dequeue();
    connect(currentQuickTask,&NotificationData::finished,this,&NotificationQueue::quicktaskEnd);
    emit doQuickTask(currentQuickTask);

}

void NotificationQueue::quicktaskEnd()
{
    currentQuickTask->disconnect();
    currentQuickTask->deleteLater();
    currentQuickTask = nullptr;
    doquickwork();
}
