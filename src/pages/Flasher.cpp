#include "pages/Flasher.hpp"

#include <QDebug>
#include <QJSEngine>

namespace genexis::pages::flasher {

FlasherController::FlasherController(QObject *parent) : QObject(parent) {}

FlasherController &FlasherController::instance() {
    static FlasherController inst;
    return inst;
}

FlasherController *FlasherController::create(QQmlEngine *, QJSEngine *) {
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

void FlasherController::setSelectedFlasher(const QString &flasher) {
    if (m_selectedFlasher == flasher) {
        return;
    }
    m_selectedFlasher = flasher;
    Q_EMIT selectedFlasherChanged();
}

void FlasherController::setTargetIp(const QString &ip) {
    if (m_targetIp == ip) {
        return;
    }
    m_targetIp = ip;
    Q_EMIT targetIpChanged();
}

void FlasherController::searchNetworkDevices() {
    qInfo() << "FlasherController::searchNetworkDevices (Phase 1 stub)";
}

void FlasherController::detectHardware(const QString &filePath) {
    qInfo() << "FlasherController::detectHardware (Phase 1 stub):" << filePath;
}

void FlasherController::performOperation(const QString &filePath, const QString &operation,
                                         const QVariantMap &options) {
    qInfo() << "FlasherController::performOperation (Phase 1 stub):" << operation << filePath
            << options;
    Q_EMIT operationFinished(false, QStringLiteral("Flasher backend is not implemented in Phase 1."));
}

} // namespace genexis::pages::flasher
