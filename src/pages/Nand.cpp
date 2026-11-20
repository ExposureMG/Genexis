#include "pages/Nand.hpp"

#include "utils/Files.hpp"

#include <QDebug>
#include <QJSEngine>

namespace genexis::pages::nand {

// --- NandSession ---

NandSession::NandSession(QObject *parent) : QObject(parent) {}

NandSession &NandSession::instance() {
    static NandSession inst;
    return inst;
}

NandSession *NandSession::create(QQmlEngine *, QJSEngine *) {
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

void NandSession::openFile(const QUrl &fileUrl, const QString &cpuKey) {
    const auto path = genexis::utils::files::urlToPath(fileUrl);
    qInfo() << "NandSession::openFile (Phase 1 stub):"
            << genexis::utils::files::pathToQString(path)
            << "cpuKeyProvided:" << !cpuKey.trimmed().isEmpty();
}

QString NandSession::detectCpuKey(const QUrl &fileUrl) {
    const auto path = genexis::utils::files::urlToPath(fileUrl);
    return genexis::utils::files::detectSiblingCpuKey(path).value_or(QString{});
}

// --- InfoController ---

InfoController::InfoController(QObject *parent) : QObject(parent) {}

InfoController &InfoController::instance() {
    static InfoController inst;
    return inst;
}

InfoController *InfoController::create(QQmlEngine *, QJSEngine *) {
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

// --- BuilderController ---

BuilderController::BuilderController(QObject *parent) : QObject(parent) {}

BuilderController &BuilderController::instance() {
    static BuilderController inst;
    return inst;
}

BuilderController *BuilderController::create(QQmlEngine *, QJSEngine *) {
    auto *inst = &instance();
    QJSEngine::setObjectOwnership(inst, QJSEngine::CppOwnership);
    return inst;
}

void BuilderController::setSelectedSimpleVersion(const QString &version) {
    qDebug() << "BuilderController::setSelectedSimpleVersion (stub):" << version;
}

void BuilderController::setSelectedSimpleImageType(const QString &imageType) {
    qDebug() << "BuilderController::setSelectedSimpleImageType (stub):" << imageType;
}

void BuilderController::setSelectedSimpleHack(const QString &hack) {
    qDebug() << "BuilderController::setSelectedSimpleHack (stub):" << hack;
}

void BuilderController::setSelectedXellHack(const QString &hack) {
    qDebug() << "BuilderController::setSelectedXellHack (stub):" << hack;
}

void BuilderController::setSelectedXellImage(const QString &image) {
    qDebug() << "BuilderController::setSelectedXellImage (stub):" << image;
}

void BuilderController::setSelectedVersion(const QString &version) {
    qDebug() << "BuilderController::setSelectedVersion (stub):" << version;
}

void BuilderController::setSelectedImageType(const QString &imageType) {
    qDebug() << "BuilderController::setSelectedImageType (stub):" << imageType;
}

void BuilderController::setSelectedConsole(const QString &console) {
    qDebug() << "BuilderController::setSelectedConsole (stub):" << console;
}

void BuilderController::setSelectedSmc(const QString &smc) {
    qDebug() << "BuilderController::setSelectedSmc (stub):" << smc;
}

void BuilderController::buildImage(const QVariantMap &config) {
    qInfo() << "BuilderController::buildImage (Phase 1 stub):" << config;
    Q_EMIT buildStarted();
    Q_EMIT buildFinished(false, {}, QStringLiteral("NAND image building is not implemented in Phase 1."));
}

} // namespace genexis::pages::nand
