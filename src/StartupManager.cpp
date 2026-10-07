#include "StartupManager.hpp"
#include "utils/Log.hpp"
#include "backend/BackendManager.hpp"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QStringList>

#include <set>
#include <string>

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

  Q_EMIT settingsPathChanged();
  qDebug() << "[StartupManager] Settings file registered at:" << m_settingsPath;
}

void StartupManager::registerBackends() {
  const auto &backends = gxapi::backend::BackendManager::instance();

  // A backend can serve several roles (NandProMax does flash and JTAG).
  std::set<std::string> names;
  for (const auto &list :
       {backends.getAvailableBuilderBackends(),
        backends.getAvailableFlashBackends(),
        backends.getAvailableJtagBackends(),
        backends.getAvailableNetworkBackends()}) {
    names.insert(list.begin(), list.end());
  }
  m_loadedPluginsCount = static_cast<int>(names.size());

  QStringList nameList;
  for (const auto &name : names) {
    nameList.append(QString::fromStdString(name));
  }

  Q_EMIT pluginsLoaded(m_loadedPluginsCount);
  qDebug() << "[StartupManager] Initialized built-in submodule backends:"
           << nameList.join(QStringLiteral(", "));
}

bool StartupManager::checkForUpdates() {
  qDebug() << "[StartupManager] Update check is not implemented; skipping.";
  return false;
}

QString StartupManager::appDataPath() const { return m_appDataPath; }

QString StartupManager::settingsPath() const { return m_settingsPath; }

int StartupManager::loadedPluginsCount() const { return m_loadedPluginsCount; }

bool StartupManager::isReady() const { return m_isReady; }
