#include "utils/Settings.hpp"
#include "utils/Network.hpp"

#include "pages/Builder.hpp"
#include "pages/Info.hpp"

#include <KIconTheme>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

// Check github releases and support files commits
int updater() {
};

int main(int argc, char *argv[]) {
  KIconTheme::initTheme();
  QApplication app(argc, argv);

  QCoreApplication::setOrganizationName(QStringLiteral("org.gxoss"));
  QCoreApplication::setApplicationName(QStringLiteral("genexis"));
  QGuiApplication::setDesktopFileName(QStringLiteral("org.gxoss.genexis"));
  #ifdef Q_OS_WIN
    QApplication::setStyle("breeze");
  #endif

  if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
  }

  
  StartupManager::instance().runStartupSequence();

  QQmlApplicationEngine engine;
  
  const QUrl url(QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/gui/Main.qml"));
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