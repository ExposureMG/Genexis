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

    /// Run full startup initialization sequence: AppData -> Settings -> Plugins -> Updates
    Q_INVOKABLE void runStartupSequence();

    /// Query or initialize AppData directory location
    QString appDataPath() const;

    /// Path to the settings INI file (AppData/genexis.ini)
    QString settingsPath() const;

    /// Number of registered plugins
    int loadedPluginsCount() const;

    /// Returns true when startup sequence completes
    bool isReady() const;

    /// Stub/hook for update checks
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
    void registerPlugins();

    QString m_appDataPath;
    QString m_settingsPath;
    int m_loadedPluginsCount{0};
    bool m_isReady{false};
    std::unique_ptr<QSettings> m_settings;
};
