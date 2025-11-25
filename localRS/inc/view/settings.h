#pragma once

#include <core/ViewModel.h>

class SettingItem;
class ISettingsLoad;
class SettingListModel;
class SettingsManager;

class Settings : public ViewModel
{
    Q_OBJECT

    // 左侧选项
    Q_PROPERTY(QList<QString> serviceNames MEMBER m_serviceNames NOTIFY serviceNamesChanged FINAL)
    // 右侧内容
    Q_PROPERTY(SettingListModel* currentModel READ currentModel NOTIFY currentModelChanged FINAL)

public:
    explicit Settings(QQmlEngine &engine, QObject *parent = nullptr);
    explicit Settings(QQmlEngine &engine, const QUrl& url,QString& componentName, QObject *parent = nullptr);
    // explicit Settings(QQmlApplicationEngine &engine, const QString & qmlText, QObject *parent = nullptr);
    // explicit Settings(QQmlApplicationEngine &engine, const QString& module, const QString& component,
    //                   QQmlComponent::CompilationMode mode = QQmlComponent::CompilationMode::PreferSynchronous, QObject *parent = nullptr);
    // explicit Settings(QQmlApplicationEngine &engine, const LoadSource & src, QObject *parent = nullptr);

    // std::string className() override;

    // void injection();    // 接收外部注入的核心组件 SettingsManager

    Q_INVOKABLE void currentChanged(const QString & key);
    // Q_INVOKABLE SettingItem* item(const QString & key);
    // void AddSettings(ISettingsLoad* iss);
    // void RemoveSettings(ISettingsLoad* iss);

    SettingListModel* currentModel() const;

signals:
    void serviceNamesChanged();
    void currentModelChanged();

private:
    void Init();  // 初始化函数

private:
    // QMap<QString, ISettingsLoad*> m_map;
    SettingListModel* m_currentModel = nullptr;
    QList<QString> m_serviceNames;
    SettingsManager * m_settings;

};

