#include "pages/Nand.hpp"

#include "NandSnapshotFixture.hpp"

#include <QGuiApplication>
#include <QHash>
#include <QJSValue>
#include <QMetaProperty>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlPropertyMap>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSet>
#include <QtTest/QTest>

#include <memory>

// Binds the real Nand controller into the NAND-related QML files (loaded from
// the source tree) and fails on any QML warning, so a stale access path in QML
// cannot pass silently.

namespace {

class TestNand final : public Nand {
public:
  using Nand::applySnapshot;
};

QStringList g_warnings;
QtMessageHandler g_previousHandler = nullptr;

void captureWarnings(QtMsgType type, const QMessageLogContext &context,
                     const QString &message) {
  if (type == QtWarningMsg || type == QtCriticalMsg) {
    g_warnings.append(message);
  }
  if (g_previousHandler) {
    g_previousHandler(type, context, message);
  }
}

// A QML var property reads back as a QJSValue when it holds a JS array.
QVariantList toVariantList(const QVariant &value) {
  if (value.metaType() == QMetaType::fromType<QJSValue>()) {
    return value.value<QJSValue>().toVariant().toList();
  }
  return value.toList();
}

// heading -> summary of every ComponentCard below root.
QHash<QString, QString> cardSummaries(QQuickItem *root) {
  QHash<QString, QString> cards;
  QList<QQuickItem *> pending{root};
  while (!pending.isEmpty()) {
    QQuickItem *item = pending.takeLast();
    const QVariant heading = item->property("heading");
    if (heading.isValid()) {
      cards.insert(heading.toString(), item->property("summary").toString());
    }
    pending.append(item->childItems());
  }
  return cards;
}

QUrl qmlFile(const QString &relativePath) {
  return QUrl::fromLocalFile(QStringLiteral(GENEXIS_QML_DIR "/") +
                             relativePath);
}

struct ExpectedField {
  const char *property;
  QString NandInfoSnapshot::*field;
};

// Every string property NandMetadata.qml exposes and the snapshot field it
// must show.
constexpr ExpectedField kMetadataFields[] = {
    {"consoleTarget", &NandInfoSnapshot::consoleTarget},
    {"imageSize", &NandInfoSnapshot::imageSize},
    {"blockType", &NandInfoSnapshot::blockType},
    {"kernelVerOrType", &NandInfoSnapshot::ceVersion},
    {"smcVersion", &NandInfoSnapshot::smcVersion},
    {"smcType", &NandInfoSnapshot::smcType},
    {"smcSize", &NandInfoSnapshot::smcSize},
    {"smcConfigOffset", &NandInfoSnapshot::smcConfigOffset},
    {"cbVersion", &NandInfoSnapshot::cbVersion},
    {"cbSize", &NandInfoSnapshot::cbSize},
    {"cbMagic", &NandInfoSnapshot::cbMagic},
    {"cbLdv", &NandInfoSnapshot::cbLdv},
    {"cbPairing", &NandInfoSnapshot::cbPairing},
    {"cbAVersion", &NandInfoSnapshot::cbAVersion},
    {"cbALdv", &NandInfoSnapshot::cbALdv},
    {"cbAPairing", &NandInfoSnapshot::cbAPairing},
    {"cbBVersion", &NandInfoSnapshot::cbBVersion},
    {"scVersion", &NandInfoSnapshot::scVersion},
    {"ccVersion", &NandInfoSnapshot::ccVersion},
    {"cdVersion", &NandInfoSnapshot::cdVersion},
    {"ceVersion", &NandInfoSnapshot::ceVersion},
    {"cf0Version", &NandInfoSnapshot::cf0Version},
    {"cg0Version", &NandInfoSnapshot::cg0Version},
    {"cf0Ldv", &NandInfoSnapshot::cf0Ldv},
    {"cf0Pairing", &NandInfoSnapshot::cf0Pairing},
    {"cf1Version", &NandInfoSnapshot::cf1Version},
    {"cg1Version", &NandInfoSnapshot::cg1Version},
    {"cf1Ldv", &NandInfoSnapshot::cf1Ldv},
    {"cf1Pairing", &NandInfoSnapshot::cf1Pairing},
    {"serialNumber", &NandInfoSnapshot::serialNumber},
    {"consoleId", &NandInfoSnapshot::consoleId},
    {"dvdKey", &NandInfoSnapshot::dvdKey},
    {"gameRegion", &NandInfoSnapshot::gameRegion},
    {"consoleType", &NandInfoSnapshot::consoleType},
};

} // namespace

class NandQmlTests final : public QObject {
  Q_OBJECT

public:
  static void initMain() {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    // org.kde.desktop needs a session bus; the binding paths under test do not
    // depend on the style.
    qputenv("QT_QUICK_CONTROLS_STYLE", "Fusion");
  }

private Q_SLOTS:
  void initTestCase();
  void init();
  void cleanup();
  void cleanupTestCase();

  void metadataFollowsController();
  void componentCardsFollowController();
  void viewsBindWithoutWarnings_data();
  void viewsBindWithoutWarnings();

private:
  std::unique_ptr<QObject> create(const QString &relativePath);

  TestNand m_nand;
  QQmlPropertyMap m_builderStub;
  std::unique_ptr<QQuickWindow> m_window;
  std::unique_ptr<QQmlEngine> m_engine;
};

void NandQmlTests::initTestCase() {
  g_previousHandler = qInstallMessageHandler(captureWarnings);

  // NandBuilderDonor.qml also reads these list properties at creation.
  for (const char *name : {"availablePatches", "buildVersions", "imageTypes",
                           "consoles", "smcFiles"}) {
    m_builderStub.insert(QLatin1StringView(name), QStringList());
  }

  m_window = std::make_unique<QQuickWindow>();
  m_engine = std::make_unique<QQmlEngine>();
  m_engine->rootContext()->setContextProperty(QStringLiteral("nandController"),
                                              &m_nand);
  m_engine->rootContext()->setContextProperty(
      QStringLiteral("nandBuilderController"), &m_builderStub);
}

void NandQmlTests::init() {
  m_nand.clear();
  g_warnings.clear();
}

void NandQmlTests::cleanup() {
  QCoreApplication::processEvents();
  QVERIFY2(g_warnings.isEmpty(),
           qPrintable(g_warnings.join(QStringLiteral("\n"))));
}

void NandQmlTests::cleanupTestCase() {
  m_engine.reset();
  m_window.reset();
  qInstallMessageHandler(g_previousHandler);
}

std::unique_ptr<QObject> NandQmlTests::create(const QString &relativePath) {
  QQmlComponent component(m_engine.get(), qmlFile(relativePath));
  if (component.isError()) {
    g_warnings.append(component.errorString());
    return nullptr;
  }
  std::unique_ptr<QObject> object(component.createWithInitialProperties(
      {{QStringLiteral("parent"),
        QVariant::fromValue(m_window->contentItem())}}));
  if (!object) {
    g_warnings.append(component.errorString());
  }
  return object;
}

void NandQmlTests::metadataFollowsController() {
  QQmlComponent component(
      m_engine.get(),
      qmlFile(QStringLiteral("components/nand/NandMetadata.qml")));
  std::unique_ptr<QObject> metadata(component.create());
  QVERIFY2(metadata, qPrintable(component.errorString()));

  QSet<QString> exposed;
  const QMetaObject *meta = metadata->metaObject();
  for (int i = meta->propertyOffset(); i < meta->propertyCount(); ++i) {
    exposed.insert(QString::fromLatin1(meta->property(i).name()));
  }
  QSet<QString> expected{QStringLiteral("metadata"),
                         QStringLiteral("components")};
  for (const auto &field : kMetadataFields) {
    expected.insert(QLatin1StringView(field.property));
  }
  QCOMPARE(exposed, expected);

  const auto verifyFields = [&](const NandInfoSnapshot &snapshot) {
    for (const auto &field : kMetadataFields) {
      const QVariant value = metadata->property(field.property);
      QVERIFY2(value.typeId() == QMetaType::QString, field.property);
      QCOMPARE(value.toString(), snapshot.*field.field);
    }
    QCOMPARE(toVariantList(metadata->property("components")),
             snapshot.components);
  };

  verifyFields({});

  const NandInfoSnapshot snapshot = makeDistinctSnapshot();
  m_nand.applySnapshot(snapshot, true);
  verifyFields(snapshot);

  m_nand.clear();
  verifyFields({});
}

void NandQmlTests::componentCardsFollowController() {
  m_window->resize(800, 600);
  QQmlComponent component(
      m_engine.get(),
      qmlFile(QStringLiteral("components/nand/Components.qml")));
  std::unique_ptr<QObject> object(component.createWithInitialProperties(
      {{QStringLiteral("parent"), QVariant::fromValue(m_window->contentItem())},
       {QStringLiteral("width"), 800},
       {QStringLiteral("height"), 600}}));
  auto *page = qobject_cast<QQuickItem *>(object.get());
  QVERIFY2(page, qPrintable(component.errorString()));

  // Empty versionStr makes the cards fall back to NandMetadata values.
  NandInfoSnapshot snapshot = makeDistinctSnapshot();
  snapshot.components = {
      QVariantMap{{QStringLiteral("cardType"), QStringLiteral("smc")},
                  {QStringLiteral("versionStr"), QString()}},
      QVariantMap{{QStringLiteral("cardType"), QStringLiteral("keyvault")},
                  {QStringLiteral("versionStr"), QString()}},
  };
  m_nand.applySnapshot(snapshot, true);
  QCoreApplication::processEvents();

  const QHash<QString, QString> cards = cardSummaries(page);
  QCOMPARE(cards.size(), 2);
  QCOMPARE(cards.value(QStringLiteral("SMC Firmware")),
           QStringLiteral("Version: ") + snapshot.smcVersion);
  QCOMPARE(cards.value(QStringLiteral("Keyvault")),
           QStringLiteral("Serial: ") + snapshot.serialNumber);

  m_nand.clear();
  QCoreApplication::processEvents();
  QVERIFY(cardSummaries(page).isEmpty());
}

void NandQmlTests::viewsBindWithoutWarnings_data() {
  QTest::addColumn<QString>("path");
  for (const char *path : {
           "components/nand/Overview.qml",
           "components/nand/Components.qml",
           "components/nand/Keyvault.qml",
           "components/nand/parts/Bootloaders.qml",
           "components/nand/parts/Kernel.qml",
           "components/nand/parts/Smc.qml",
           "components/nand/parts/Updates.qml",
           "components/NandBuilderDonor.qml",
           "pages/Home.qml",
           "pages/NandInfo.qml",
       }) {
    QTest::newRow(path) << QString::fromLatin1(path);
  }
}

void NandQmlTests::viewsBindWithoutWarnings() {
  QFETCH(QString, path);

  std::unique_ptr<QObject> view = create(path);
  QVERIFY(view);
  QCoreApplication::processEvents();

  m_nand.applySnapshot(makeDistinctSnapshot(), true);
  QCoreApplication::processEvents();

  m_nand.clear();
  QCoreApplication::processEvents();
}

QTEST_MAIN(NandQmlTests)
#include "NandQmlTests.moc"
