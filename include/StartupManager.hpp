#pragma once

#include <QDir>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <qqmlregistration.h>

#include <memory>

class StartupManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString appDataPath READ appDataPath NOTIFY appDataPathChanged)
    Q_PROPERTY(QString settingsPath READ settingsPath NOTIFY settingsPathChanged)
    Q_PROPERTY(int loadedPluginsCount READ loadedPluginsCount NOTIFY pluginsLoaded)
    Q_PROPERTY(bool isReady READ isReady NOTIFY readyStateChanged)

public:
    explicit StartupManager(QObject *parent = nullptr);
    ~StartupManager() override = default;

    static StartupManager &instance();

    
    Q_INVOKABLE void runStartupSequence();

    
    QString appDataPath() const;

    
    QString settingsPath() const;

    
    int loadedPluginsCount() const;

    
    bool isReady() const;

    
    Q_INVOKABLE bool checkForUpdates();

Q_SIGNALS:
    void startupProgress(qreal progress, const QString &statusMessage);
    void appDataPathChanged();
    void settingsPathChanged();
    void pluginsLoaded(int count);
    void readyStateChanged(bool ready);

private:
    void initAppDataDir();
    void registerSettings();
    void registerBackends();

    QString m_appDataPath;
    QString m_settingsPath;
    int m_loadedPluginsCount{0};
    bool m_isReady{false};
    std::unique_ptr<QSettings> m_settings;
};
