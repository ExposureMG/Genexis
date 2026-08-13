#include "StartupManager.hpp"
#include "pages/Flasher.hpp"
#include "pages/Nand.hpp"
#include "pages/NandBuilderController.hpp"
#include "pages/Settings.hpp"

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QCoreApplication::setOrganizationName(QStringLiteral("org.gxoss"));
  QCoreApplication::setApplicationName(QStringLiteral("genexis"));
  QGuiApplication::setDesktopFileName(QStringLiteral("org.gxoss.genexis"));

  if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
  }

  // Execute Genexis startup tasks (AppData, Settings, Plugins, Updates)
  StartupManager::instance().runStartupSequence();

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty(QStringLiteral("startupManager"),
                                           &StartupManager::instance());
  engine.rootContext()->setContextProperty(QStringLiteral("settingsController"),
                                           &Settings::instance());
  engine.rootContext()->setContextProperty(QStringLiteral("flasherController"),
                                           &Flasher::instance());
  engine.rootContext()->setContextProperty(QStringLiteral("nandController"),
                                           &Nand::instance());
  engine.rootContext()->setContextProperty(
      QStringLiteral("nandBuilderController"),
      &NandBuilderController::instance());

  const QUrl url(QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/QML/Main.qml"));
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app,
      [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
          QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);

  engine.load(url);

  return app.exec();
}
