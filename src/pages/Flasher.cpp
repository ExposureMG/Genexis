#include "pages/Flasher.hpp"
#include "pages/Settings.hpp"
#include "api/Flasher.hpp"

#include <QDateTime>
#include <QDebug>
#include <QFileInfo>
#include <QUrl>
#include <thread>

Flasher::Flasher(QObject *parent)
    : QObject(parent)
{
}

Flasher &Flasher::instance()
{
    static Flasher inst;
    return inst;
}

bool Flasher::isBusy() const
{
    return m_isBusy;
}

void Flasher::performOperation(const QString &filePath, const QString &operation, const QVariantMap &options)
{
    if (m_isBusy) {
        Q_EMIT logOutput(QStringLiteral("[WARN] Flasher operation already in progress."));
        return;
    }

    // Clean file path if URL syntax (e.g. file:///path/to/file)
    QString cleanPath = filePath;
    if (cleanPath.startsWith(QStringLiteral("file://"))) {
        cleanPath = QUrl(filePath).toLocalFile();
    }

    if (cleanPath.isEmpty()) {
        Q_EMIT logOutput(QStringLiteral("[ERROR] No file selected for operation. Please browse and select a file."));
        Q_EMIT operationFinished(false, QStringLiteral("No file selected"));
        return;
    }

    QFileInfo fileInfo(cleanPath);
    QString ext = fileInfo.suffix().toLower();
    QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss"));

    m_isBusy = true;
    Q_EMIT busyStateChanged(m_isBusy);

    if (ext == QStringLiteral("bin") || ext == QStringLiteral("ecc")) {
        // Query listed Flash backend from genexis.ini
        QString flashBackend = Settings::getSetting(QStringLiteral("flashBackend"), QStringLiteral("NandProMax"));

        Q_EMIT logOutput(QStringLiteral("[%1] Extension .%2 detected -> Routing to Flash Backend: %3")
                             .arg(timestamp, ext, flashBackend));
        Q_EMIT logOutput(QStringLiteral("[%1] Performing '%2' operation on target file: %3")
                             .arg(timestamp, operation, cleanPath));

        gxapi::Flasher::DeviceConfig config;
        if (flashBackend == QStringLiteral("FTDI2SPI")) {
            config.adapter = gxapi::Flasher::AdapterType::FtdiNative;
        } else {
            config.adapter = gxapi::Flasher::AdapterType::NandProMax;
        }

        bool verify = options.value(QStringLiteral("verify"), true).toBool();

        std::thread worker([this, cleanPath, operation, config, verify]() {
            auto progressCb = [this](const gxapi::Flasher::ProgressInfo &info) {
                Q_EMIT progressUpdated(static_cast<qreal>(info.percentage / 100.0f),
                                       QString::fromStdString(info.statusMessage));
            };

            std::expected<void, std::string> res;
            if (operation.contains(QStringLiteral("read"), Qt::CaseInsensitive)) {
                res = gxapi::Flasher::readNandToFile(cleanPath.toStdString(), 0, 0, config, progressCb);
            } else {
                res = gxapi::Flasher::writeNandFromFile(cleanPath.toStdString(), 0, true, verify, config, progressCb);
            }

            m_isBusy = false;
            Q_EMIT busyStateChanged(m_isBusy);

            if (res) {
                Q_EMIT logOutput(QStringLiteral("[SUCCESS] Operation completed successfully."));
                Q_EMIT operationFinished(true, QStringLiteral("Success"));
            } else {
                QString err = QString::fromStdString(res.error());
                Q_EMIT logOutput(QStringLiteral("[ERROR] Operation failed: ") + err);
                Q_EMIT operationFinished(false, err);
            }
        });
        worker.detach();

    } else if (ext == QStringLiteral("svf") || ext == QStringLiteral("xsvf")) {
        // Query listed Timing Flash backend from genexis.ini
        QString timingBackend = Settings::getSetting(QStringLiteral("timingFlashBackend"), QStringLiteral("NandProMax"));

        Q_EMIT logOutput(QStringLiteral("[%1] Extension .%2 detected -> Routing to Timing Flash Backend: %3")
                             .arg(timestamp, ext, timingBackend));
        Q_EMIT logOutput(QStringLiteral("[%1] Flashing CPLD timing file: %2")
                             .arg(timestamp, cleanPath));

        gxapi::Flasher::DeviceConfig config;
        if (timingBackend == QStringLiteral("xsvftool")) {
            config.adapter = gxapi::Flasher::AdapterType::XsvfTool;
        } else {
            config.adapter = gxapi::Flasher::AdapterType::NandProMax;
        }

        std::thread worker([this, cleanPath, config]() {
            auto progressCb = [this](const gxapi::Flasher::ProgressInfo &info) {
                Q_EMIT progressUpdated(static_cast<qreal>(info.percentage / 100.0f),
                                       QString::fromStdString(info.statusMessage));
            };

            auto res = gxapi::Flasher::flashCpld(cleanPath.toStdString(), config, progressCb);

            m_isBusy = false;
            Q_EMIT busyStateChanged(m_isBusy);

            if (res) {
                Q_EMIT logOutput(QStringLiteral("[SUCCESS] CPLD timing flashed successfully."));
                Q_EMIT operationFinished(true, QStringLiteral("Success"));
            } else {
                QString err = QString::fromStdString(res.error());
                Q_EMIT logOutput(QStringLiteral("[ERROR] CPLD flash failed: ") + err);
                Q_EMIT operationFinished(false, err);
            }
        });
        worker.detach();

    } else {
        m_isBusy = false;
        Q_EMIT busyStateChanged(m_isBusy);
        QString msg = QStringLiteral("[%1] [ERROR] Unsupported file format '.%2'. Expected .bin/.ecc for NAND operations or .svf/.xsvf for CPLD timing operations.")
                          .arg(timestamp, ext);
        Q_EMIT logOutput(msg);
        Q_EMIT operationFinished(false, QStringLiteral("Unsupported file extension"));
    }
}
