#include "StartupManager.hpp"
#include "pages/Flasher.hpp"
#include "pages/Nand.hpp"
#include "pages/NandBuilderController.hpp"
#include "pages/Settings.hpp"

#include <KIconTheme>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include <utility>

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

  StartupManager::instance().runStartupSequence();

  // The controllers reach QML only as these context properties.
  const std::pair<QString, QObject *> controllers[] = {
      {QStringLiteral("startupManager"), &StartupManager::instance()},
      {QStringLiteral("settingsController"), &Settings::instance()},
      {QStringLiteral("flasherController"), &Flasher::instance()},
      {QStringLiteral("nandController"), &Nand::instance()},
      {QStringLiteral("nandBuilderController"),
       &NandBuilderController::instance()},
  };

  QQmlApplicationEngine engine;
  for (const auto &[name, controller] : controllers) {
    engine.rootContext()->setContextProperty(name, controller);
  }

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
