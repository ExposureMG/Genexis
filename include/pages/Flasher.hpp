#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace genexis::pages::flasher {

/// QML singleton shell for the Flasher page (hardware/network backends out of scope in Phase 1).
class FlasherController : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(FlasherController)
    QML_SINGLETON

    Q_PROPERTY(QString connectedFlasherImage READ connectedFlasherImage NOTIFY connectedFlasherImageChanged)
    Q_PROPERTY(QStringList availableFlashers READ availableFlashers NOTIFY availableFlashersChanged)
    Q_PROPERTY(QString selectedFlasher READ selectedFlasher NOTIFY selectedFlasherChanged)
    Q_PROPERTY(bool isUpdClientSelected READ isUpdClientSelected NOTIFY selectedFlasherChanged)
    Q_PROPERTY(QString targetIp READ targetIp NOTIFY targetIpChanged)
    Q_PROPERTY(QStringList detectedNetworkDevices READ detectedNetworkDevices NOTIFY detectedNetworkDevicesChanged)
    Q_PROPERTY(QString detectedHardwareInfo READ detectedHardwareInfo NOTIFY detectedHardwareInfoChanged)
    Q_PROPERTY(bool isBusy READ isBusy NOTIFY isBusyChanged)

public:
    static FlasherController &instance();
    static FlasherController *create(QQmlEngine *engine, QJSEngine *scriptEngine);

    [[nodiscard]] const QString &connectedFlasherImage() const noexcept { return m_connectedFlasherImage; }
    [[nodiscard]] const QStringList &availableFlashers() const noexcept { return m_availableFlashers; }
    [[nodiscard]] const QString &selectedFlasher() const noexcept { return m_selectedFlasher; }
    [[nodiscard]] bool isUpdClientSelected() const noexcept {
        return m_selectedFlasher.startsWith(QStringLiteral("UpdClient"));
    }
    [[nodiscard]] const QString &targetIp() const noexcept { return m_targetIp; }
    [[nodiscard]] const QStringList &detectedNetworkDevices() const noexcept {
        return m_detectedNetworkDevices;
    }
    [[nodiscard]] const QString &detectedHardwareInfo() const noexcept { return m_detectedHardwareInfo; }
    [[nodiscard]] bool isBusy() const noexcept { return m_isBusy; }

    Q_INVOKABLE void setSelectedFlasher(const QString &flasher);
    Q_INVOKABLE void setTargetIp(const QString &ip);
    Q_INVOKABLE void searchNetworkDevices();
    Q_INVOKABLE void detectHardware(const QString &filePath = {});
    Q_INVOKABLE void performOperation(const QString &filePath, const QString &operation,
                                        const QVariantMap &options = {});

Q_SIGNALS:
    void connectedFlasherImageChanged();
    void availableFlashersChanged();
    void selectedFlasherChanged();
    void targetIpChanged();
    void detectedNetworkDevicesChanged();
    void detectedHardwareInfoChanged();
    void isBusyChanged();

    void logOutput(const QString &message);
    void progressUpdated(int percentage, const QString &status);
    void operationFinished(bool success, const QString &message);

private:
    explicit FlasherController(QObject *parent = nullptr);

    QString m_connectedFlasherImage;
    QStringList m_availableFlashers{QStringLiteral("None"), QStringLiteral("UpdClient (Network)")};
    QString m_selectedFlasher{QStringLiteral("None")};
    QString m_targetIp;
    QStringList m_detectedNetworkDevices;
    QString m_detectedHardwareInfo{QStringLiteral("000000")};
    bool m_isBusy{false};
};

} // namespace genexis::pages::flasher
