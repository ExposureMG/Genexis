#include "adapters/Gxbuild3.hpp"
#include "utils/Files.hpp"
#include "utils/Network.hpp"
#include "utils/Settings.hpp"

#include <KIconTheme>
#include <QApplication>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

namespace {

void checkForUpdatesAsync(QObject *context) {
    auto *client = new genexis::utils::network::NetworkClient(context);
    const QUrl apiBase(QStringLiteral("https://api.github.com"));
    client->getJson(
        apiBase, QStringLiteral("/repos/gx-oss/Genexis/releases/latest"), QUrlQuery{},
        [client](std::expected<QJsonDocument, QString> result) {
            if (!result) {
                qDebug() << "Deferred update check skipped/offline:" << result.error();
            } else {
                qDebug() << "Update check completed.";
            }
            client->deleteLater();
        });
}

} // namespace

int main(int argc, char *argv[]) {
    KIconTheme::initTheme();
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("org.gxoss"));
    QCoreApplication::setApplicationName(QStringLiteral("genexis"));
    QGuiApplication::setDesktopFileName(QStringLiteral("org.gxoss.genexis"));

#ifdef Q_OS_WIN
    QApplication::setStyle(QStringLiteral("breeze"));
#endif

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    genexis::adapters::gxbuild3::initLogging();
    QObject::connect(&app, &QCoreApplication::aboutToQuit, []() {
        genexis::adapters::gxbuild3::shutdownLogging();
    });

    // Initialize SettingsController before initDirectories() so custom paths are loaded first.
    genexis::utils::settings::SettingsController::instance().loadSettings();

    if (const auto dirRes = genexis::utils::files::initDirectories(); !dirRes) {
        qWarning() << "Failed to initialize runtime directories:"
                   << QString::fromStdString(dirRes.error());
    }

    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/gui/Main.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    QTimer::singleShot(0, &app, [&app]() { checkForUpdatesAsync(&app); });

    return app.exec();
}
