#include "utils/Settings.hpp"

#include <QDir>
#include <QFileInfo>
#include <QJSEngine>
#include <QStandardPaths>

namespace genexis::utils::settings {

namespace {
constexpr QLatin1StringView kKeyBuildBackend("backends/build");
constexpr QLatin1StringView kKeyFlashBackend("backends/flash");
constexpr QLatin1StringView kKeyTimingFlashBackend("backends/timingFlash");
constexpr QLatin1StringView kKeyWirelessBackend("backends/wireless");
constexpr QLatin1StringView kKeyXebuildDataPath("paths/xebuildData");
constexpr QLatin1StringView kKeyOutputDirPath("paths/outputDir");
} // namespace

Settings::Settings(QString filePath)
    : m_filePath(filePath.isEmpty() ? defaultFilePath() : std::move(filePath)) {
    load();
}

QString Settings::defaultFilePath() {
    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    return QDir(appData).filePath(QStringLiteral("genexis.ini"));
}

void Settings::load() {
    QSettings ini(m_filePath, QSettings::IniFormat);
    m_buildBackend = ini.value(kKeyBuildBackend, QString::fromLatin1(kDefaultBuildBackend)).toString();
    m_flashBackend = ini.value(kKeyFlashBackend, QString::fromLatin1(kDefaultFlashBackend)).toString();
    m_timingFlashBackend =
        ini.value(kKeyTimingFlashBackend, QString::fromLatin1(kDefaultTimingFlashBackend)).toString();
    m_wirelessBackend =
        ini.value(kKeyWirelessBackend, QString::fromLatin1(kDefaultWirelessBackend)).toString();
    m_xebuildDataPath = ini.value(kKeyXebuildDataPath, QString{}).toString();
    m_outputDirPath = ini.value(kKeyOutputDirPath, QString{}).toString();
}

void Settings::save() {
    const QFileInfo info(m_filePath);
    QDir().mkpath(info.absolutePath());

    QSettings ini(m_filePath, QSettings::IniFormat);
    ini.setValue(kKeyBuildBackend, m_buildBackend);
    ini.setValue(kKeyFlashBackend, m_flashBackend);
    ini.setValue(kKeyTimingFlashBackend, m_timingFlashBackend);
    ini.setValue(kKeyWirelessBackend, m_wirelessBackend);
    ini.setValue(kKeyXebuildDataPath, m_xebuildDataPath);
    ini.setValue(kKeyOutputDirPath, m_outputDirPath);
    ini.sync();
}

SettingsController::SettingsController(QObject *parent) : QObject(parent) {}

SettingsController &SettingsController::instance() {
    static SettingsController inst;
    return inst;
}

SettingsController *SettingsController::create(QQmlEngine *, QJSEngine *) {
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

QString SettingsController::buildBackend() const {
    return m_settings.buildBackend();
}

void SettingsController::setBuildBackend(const QString &value) {
    if (m_settings.buildBackend() == value) {
        return;
    }
    m_settings.setBuildBackend(value);
    Q_EMIT buildBackendChanged();
}

QString SettingsController::flashBackend() const {
    return m_settings.flashBackend();
}

void SettingsController::setFlashBackend(const QString &value) {
    if (m_settings.flashBackend() == value) {
        return;
    }
    m_settings.setFlashBackend(value);
    Q_EMIT flashBackendChanged();
}

QString SettingsController::timingFlashBackend() const {
    return m_settings.timingFlashBackend();
}

void SettingsController::setTimingFlashBackend(const QString &value) {
    if (m_settings.timingFlashBackend() == value) {
        return;
    }
    m_settings.setTimingFlashBackend(value);
    Q_EMIT timingFlashBackendChanged();
}

QString SettingsController::wirelessBackend() const {
    return m_settings.wirelessBackend();
}

void SettingsController::setWirelessBackend(const QString &value) {
    if (m_settings.wirelessBackend() == value) {
        return;
    }
    m_settings.setWirelessBackend(value);
    Q_EMIT wirelessBackendChanged();
}

QStringList SettingsController::availableBuildBackends() const {
    return {QStringLiteral("gxbuild3")};
}

QStringList SettingsController::availableFlashBackends() const {
    return {QStringLiteral("NandProMax"), QStringLiteral("FTDI2SPI"), QStringLiteral("UpdClient")};
}

QStringList SettingsController::availableTimingFlashBackends() const {
    return {QStringLiteral("NandProMax"), QStringLiteral("xsvftool")};
}

QStringList SettingsController::availableWirelessBackends() const {
    return {QStringLiteral("UpdClient")};
}

void SettingsController::loadSettings() {
    const QString prevBuild = m_settings.buildBackend();
    const QString prevFlash = m_settings.flashBackend();
    const QString prevTiming = m_settings.timingFlashBackend();
    const QString prevWireless = m_settings.wirelessBackend();

    m_settings.load();

    if (prevBuild != m_settings.buildBackend()) {
        Q_EMIT buildBackendChanged();
    }
    if (prevFlash != m_settings.flashBackend()) {
        Q_EMIT flashBackendChanged();
    }
    if (prevTiming != m_settings.timingFlashBackend()) {
        Q_EMIT timingFlashBackendChanged();
    }
    if (prevWireless != m_settings.wirelessBackend()) {
        Q_EMIT wirelessBackendChanged();
    }
}

void SettingsController::saveSettings() {
    m_settings.save();
}

} // namespace genexis::utils::settings
