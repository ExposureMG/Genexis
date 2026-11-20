#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QSettings>
#include <QString>
#include <QStringList>

namespace genexis::utils::settings {

/// Persistent application settings backed by an INI-format QSettings file.
class Settings {
public:
    static constexpr const char *kDefaultBuildBackend = "gxbuild3";
    static constexpr const char *kDefaultFlashBackend = "NandProMax";
    static constexpr const char *kDefaultTimingFlashBackend = "NandProMax";
    static constexpr const char *kDefaultWirelessBackend = "UpdClient";

    /// Constructs a Settings wrapper targeting \p filePath (or defaultFilePath() when empty).
    explicit Settings(QString filePath = {});

    /// Returns the default INI settings file path under AppLocalDataLocation.
    [[nodiscard]] static QString defaultFilePath();

    /// Returns the INI file path targeted by this Settings instance.
    [[nodiscard]] const QString &filePath() const noexcept { return m_filePath; }

    /// Reloads all persisted values from disk.
    void load();

    /// Writes all values to disk and calls QSettings::sync().
    void save();

    [[nodiscard]] const QString &buildBackend() const noexcept { return m_buildBackend; }
    void setBuildBackend(const QString &value) { m_buildBackend = value; }

    [[nodiscard]] const QString &flashBackend() const noexcept { return m_flashBackend; }
    void setFlashBackend(const QString &value) { m_flashBackend = value; }

    [[nodiscard]] const QString &timingFlashBackend() const noexcept { return m_timingFlashBackend; }
    void setTimingFlashBackend(const QString &value) { m_timingFlashBackend = value; }

    [[nodiscard]] const QString &wirelessBackend() const noexcept { return m_wirelessBackend; }
    void setWirelessBackend(const QString &value) { m_wirelessBackend = value; }

    [[nodiscard]] const QString &xebuildDataPath() const noexcept { return m_xebuildDataPath; }
    void setXebuildDataPath(const QString &value) { m_xebuildDataPath = value; }

    [[nodiscard]] const QString &outputDirPath() const noexcept { return m_outputDirPath; }
    void setOutputDirPath(const QString &value) { m_outputDirPath = value; }

private:
    QString m_filePath;
    QString m_buildBackend{QString::fromLatin1(kDefaultBuildBackend)};
    QString m_flashBackend{QString::fromLatin1(kDefaultFlashBackend)};
    QString m_timingFlashBackend{QString::fromLatin1(kDefaultTimingFlashBackend)};
    QString m_wirelessBackend{QString::fromLatin1(kDefaultWirelessBackend)};
    QString m_xebuildDataPath;
    QString m_outputDirPath;
};

/// QML singleton exposing persisted backend settings to Settings.qml.
class SettingsController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(SettingsController)
    QML_SINGLETON

    Q_PROPERTY(QString buildBackend READ buildBackend WRITE setBuildBackend NOTIFY buildBackendChanged)
    Q_PROPERTY(QString flashBackend READ flashBackend WRITE setFlashBackend NOTIFY flashBackendChanged)
    Q_PROPERTY(QString timingFlashBackend READ timingFlashBackend WRITE setTimingFlashBackend NOTIFY timingFlashBackendChanged)
    Q_PROPERTY(QString wirelessBackend READ wirelessBackend WRITE setWirelessBackend NOTIFY wirelessBackendChanged)

    Q_PROPERTY(QStringList availableBuildBackends READ availableBuildBackends CONSTANT)
    Q_PROPERTY(QStringList availableFlashBackends READ availableFlashBackends CONSTANT)
    Q_PROPERTY(QStringList availableTimingFlashBackends READ availableTimingFlashBackends CONSTANT)
    Q_PROPERTY(QStringList availableWirelessBackends READ availableWirelessBackends CONSTANT)

public:
    static SettingsController &instance();
    static SettingsController *create(QQmlEngine *engine, QJSEngine *scriptEngine);

    [[nodiscard]] QString buildBackend() const;
    void setBuildBackend(const QString &value);

    [[nodiscard]] QString flashBackend() const;
    void setFlashBackend(const QString &value);

    [[nodiscard]] QString timingFlashBackend() const;
    void setTimingFlashBackend(const QString &value);

    [[nodiscard]] QString wirelessBackend() const;
    void setWirelessBackend(const QString &value);

    [[nodiscard]] QStringList availableBuildBackends() const;
    [[nodiscard]] QStringList availableFlashBackends() const;
    [[nodiscard]] QStringList availableTimingFlashBackends() const;
    [[nodiscard]] QStringList availableWirelessBackends() const;

    /// Returns the underlying C++ Settings model.
    [[nodiscard]] Settings &settings() noexcept { return m_settings; }
    [[nodiscard]] const Settings &settings() const noexcept { return m_settings; }

    Q_INVOKABLE void loadSettings();
    Q_INVOKABLE void saveSettings();

Q_SIGNALS:
    void buildBackendChanged();
    void flashBackendChanged();
    void timingFlashBackendChanged();
    void wirelessBackendChanged();

private:
    explicit SettingsController(QObject *parent = nullptr);

    Settings m_settings;
};

} // namespace genexis::utils::settings
