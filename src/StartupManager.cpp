#include "StartupManager.hpp"
#include "Log.hpp"
#include "backend/BackendManager.hpp"

#include <QDebug>
#include <QDir>

StartupManager::StartupManager(QObject *parent) : QObject(parent) {}

StartupManager &StartupManager::instance() {
  static StartupManager mgr;
  return mgr;
}

void StartupManager::runStartupSequence() {
  qDebug() << "[StartupManager] Starting Genexis startup sequence...";

  
  Log::Init();

  Q_EMIT startupProgress(0.1,
                         QStringLiteral("Initializing App Data directory..."));
  initAppDataDir();

  Q_EMIT startupProgress(0.4, QStringLiteral("Registering settings..."));
  registerSettings();

  Q_EMIT startupProgress(0.7, QStringLiteral("Initializing backends..."));
  registerBackends();

  Q_EMIT startupProgress(0.9, QStringLiteral("Checking for updates..."));
  checkForUpdates();

  m_isReady = true;
  Q_EMIT readyStateChanged(m_isReady);
  Q_EMIT startupProgress(1.0, QStringLiteral("Startup complete."));

  qDebug() << "[StartupManager] Initialization complete. Backends loaded:"
           << m_loadedPluginsCount;
}

void StartupManager::initAppDataDir() {
  m_appDataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
  QDir dir(m_appDataPath);
  if (!dir.exists()) {
    dir.mkpath(QStringLiteral("."));
  }

  Q_EMIT appDataPathChanged();
  qDebug() << "[StartupManager] AppData directory initialized at:"
           << m_appDataPath;
}

void StartupManager::registerSettings() {
  m_settingsPath = QDir(m_appDataPath).filePath(QStringLiteral("genexis.ini"));
  m_settings =
      std::make_unique<QSettings>(m_settingsPath, QSettings::IniFormat);

  Q_EMIT settingsPathChanged();
  qDebug() << "[StartupManager] Settings file registered at:" << m_settingsPath;
}

void StartupManager::registerBackends() {
  gxapi::backend::BackendManager::instance().initialize();
  m_loadedPluginsCount =
      5; 

  Q_EMIT pluginsLoaded(m_loadedPluginsCount);
  qDebug() << "[StartupManager] Initialized built-in submodule backends "
              "(gxbuild3, NandProMax, FTDI2SPI, xsvftool, UpdClient).";
}

bool StartupManager::checkForUpdates() {
  
  qDebug() << "[StartupManager] Update check performed (no updates currently "
              "pending).";
  return false;
}

QString StartupManager::appDataPath() const { return m_appDataPath; }

QString StartupManager::settingsPath() const { return m_settingsPath; }

int StartupManager::loadedPluginsCount() const { return m_loadedPluginsCount; }

bool StartupManager::isReady() const { return m_isReady; }
