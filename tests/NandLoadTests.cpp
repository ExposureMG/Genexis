#include "Async.hpp"
#include "pages/Nand.hpp"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QTest>

namespace {

QString writeFile(const QTemporaryDir &dir, const QString &name) {
  const QString path = dir.filePath(name);
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    return {};
  }
  file.write(QByteArray(0x4200, '\xAB'));
  return path;
}

// Lets every queued worker finish and deliver its completion.
void drainLoads() {
  gxapi::asyncPool().waitForDone();
  QCoreApplication::processEvents();
  QCoreApplication::processEvents();
}

} // namespace

class NandLoadTests final : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void laterOpenWinsOverEarlierOpen();
  void clearDropsInFlightLoad();
};

void NandLoadTests::laterOpenWinsOverEarlierOpen() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString first = writeFile(dir, QStringLiteral("first.bin"));
  const QString second = writeFile(dir, QStringLiteral("second.bin"));
  QVERIFY(!first.isEmpty() && !second.isEmpty());

  Nand nand;
  nand.openFile(first);
  nand.openFile(second);
  QVERIFY(nand.isLoading());

  drainLoads();

  QCOMPARE(nand.loadedFilePath(), second);
  QVERIFY(!nand.isLoading());
}

void NandLoadTests::clearDropsInFlightLoad() {
  QTemporaryDir dir;
  QVERIFY(dir.isValid());
  const QString path = writeFile(dir, QStringLiteral("nand.bin"));
  QVERIFY(!path.isEmpty());

  Nand nand;
  nand.openFile(path);
  nand.clear();
  QVERIFY(!nand.isLoading());

  QSignalSpy loading(&nand, &Nand::loadingChanged);
  QSignalSpy pathChanged(&nand, &Nand::loadedFilePathChanged);
  drainLoads();

  QVERIFY(nand.loadedFilePath().isEmpty());
  QVERIFY(!nand.isLoading());
  QCOMPARE(loading.count(), 0);
  QCOMPARE(pathChanged.count(), 0);
}

QTEST_GUILESS_MAIN(NandLoadTests)
#include "NandLoadTests.moc"
