#include "StartupManager.hpp"
#include "Log.hpp"
#include "plugins/PluginRegistry.hpp"

#include <QDebug>
#include <QDir>
#include <filesystem>

StartupManager::StartupManager(QObject *parent)
    : QObject(parent)
{
}

StartupManager &StartupManager::instance()
{
    static StartupManager mgr;
    return mgr;
}

void StartupManager::runStartupSequence()
{
    qDebug() << "[StartupManager] Starting Genexis startup sequence...";

    // Initialize gxbuild3 logging system
    Log::Init();

    Q_EMIT startupProgress(0.1, QStringLiteral("Initializing App Data directory..."));
    initAppDataDir();

    Q_EMIT startupProgress(0.4, QStringLiteral("Registering settings..."));
    registerSettings();

    Q_EMIT startupProgress(0.7, QStringLiteral("Registering plugins..."));
    registerPlugins();

    Q_EMIT startupProgress(0.9, QStringLiteral("Checking for updates..."));
    checkForUpdates();

    m_isReady = true;
    Q_EMIT readyStateChanged(m_isReady);
    Q_EMIT startupProgress(1.0, QStringLiteral("Startup complete."));

    qDebug() << "[StartupManager] Initialization complete. Plugins loaded:" << m_loadedPluginsCount;
}

void StartupManager::initAppDataDir()
{
    m_appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir dir(m_appDataPath);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    if (!dir.exists(QStringLiteral("plugins"))) {
        dir.mkdir(QStringLiteral("plugins"));
    }

    Q_EMIT appDataPathChanged();
    qDebug() << "[StartupManager] AppData directory initialized at:" << m_appDataPath;
}

void StartupManager::registerSettings()
{
    m_settingsPath = QDir(m_appDataPath).filePath(QStringLiteral("genexis.ini"));
    m_settings = std::make_unique<QSettings>(m_settingsPath, QSettings::IniFormat);

    Q_EMIT settingsPathChanged();
    qDebug() << "[StartupManager] Settings file registered at:" << m_settingsPath;
}

void StartupManager::registerPlugins()
{
    QString pluginsDirPath = QDir(m_appDataPath).filePath(QStringLiteral("plugins"));
    std::filesystem::path pPath(pluginsDirPath.toStdString());

    size_t count = gxapi::Plugins::PluginRegistry::instance().scanDirectory(pPath);
    m_loadedPluginsCount = static_cast<int>(count);

    Q_EMIT pluginsLoaded(m_loadedPluginsCount);
    qDebug() << "[StartupManager] Registered" << m_loadedPluginsCount << "plugin(s) from" << pluginsDirPath;
}

bool StartupManager::checkForUpdates()
{
    // Stub / Hook for update checks
    qDebug() << "[StartupManager] Update check performed (no updates currently pending).";
    return false;
}

QString StartupManager::appDataPath() const
{
    return m_appDataPath;
}

QString StartupManager::settingsPath() const
{
    return m_settingsPath;
}

int StartupManager::loadedPluginsCount() const
{
    return m_loadedPluginsCount;
}

bool StartupManager::isReady() const
{
    return m_isReady;
}
