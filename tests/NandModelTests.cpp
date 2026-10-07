#include "pages/Nand.hpp"

#include "NandSnapshotFixture.hpp"

#include <QSignalSpy>
#include <QtTest/QTest>

#include <memory>
#include <vector>

namespace {

class TestNand final : public Nand {
public:
  using Nand::applySnapshot;
};

const QString kCpuKey = QStringLiteral("0123456789ABCDEF0123456789ABCDEF");

} // namespace

class NandModelTests final : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void startsEmpty();
  void applySnapshotExposesMetadataAndState();
  void publicSnapshotLeavesCpuKeyUnloaded();
  void clearResetsEverything();
};

void NandModelTests::startsEmpty() {
  const Nand nand;

  QVERIFY(!nand.isNandLoaded());
  QVERIFY(!nand.isCpuKeyLoaded());
  QVERIFY(!nand.isSmcDecrypted());
  QCOMPARE(nand.metadata(), NandInfoToVariantMap({}));
}

void NandModelTests::applySnapshotExposesMetadataAndState() {
  TestNand nand;
  QSignalSpy metadataSpy(&nand, &Nand::metadataChanged);
  QSignalSpy smcSpy(&nand, &Nand::smcStateChanged);
  const NandInfoSnapshot snapshot = makeDistinctSnapshot();

  nand.applySnapshot(snapshot, true);

  QCOMPARE(nand.metadata(), NandInfoToVariantMap(snapshot));
  QVERIFY(nand.isNandLoaded());
  QVERIFY(nand.isCpuKeyLoaded());
  QVERIFY(nand.isSmcDecrypted());
  QCOMPARE(metadataSpy.count(), 1);
  QCOMPARE(smcSpy.count(), 1);
}

void NandModelTests::publicSnapshotLeavesCpuKeyUnloaded() {
  TestNand nand;
  NandInfoSnapshot snapshot = makeDistinctSnapshot();
  snapshot.smcDecrypted = false;

  nand.applySnapshot(snapshot, false);

  QVERIFY(nand.isNandLoaded());
  QVERIFY(!nand.isCpuKeyLoaded());
  QVERIFY(!nand.isSmcDecrypted());
}

void NandModelTests::clearResetsEverything() {
  TestNand nand;
  // A timing file sets the loaded path without starting a load.
  nand.openFile(QStringLiteral("/nonexistent/timing.svf"));
  nand.setCpuKey(kCpuKey);
  nand.applySnapshot(makeDistinctSnapshot(), true);
  QVERIFY(!nand.loadedFilePath().isEmpty());
  QCOMPARE(nand.cpuKey(), kCpuKey);
  QVERIFY(nand.metadata() != NandInfoToVariantMap({}));

  const std::vector<void (Nand::*)()> signalsToReset = {
      &Nand::nandStateChanged, &Nand::cpuKeyStateChanged,
      &Nand::smcStateChanged,  &Nand::loadedFilePathChanged,
      &Nand::cpuKeyChanged,    &Nand::metadataChanged,
  };
  std::vector<std::unique_ptr<QSignalSpy>> spies;
  for (const auto signal : signalsToReset) {
    spies.push_back(std::make_unique<QSignalSpy>(&nand, signal));
  }
  QSignalSpy loadingSpy(&nand, &Nand::loadingChanged);

  nand.clear();

  QVERIFY(!nand.isLoading());
  QVERIFY(!nand.isNandLoaded());
  QVERIFY(!nand.isCpuKeyLoaded());
  QVERIFY(!nand.isSmcDecrypted());
  QVERIFY(nand.loadedFilePath().isEmpty());
  QVERIFY(nand.cpuKey().isEmpty());
  QCOMPARE(nand.metadata(), NandInfoToVariantMap({}));
  for (const auto &spy : spies) {
    QCOMPARE(spy->count(), 1);
  }
  QCOMPARE(loadingSpy.count(), 0);
}

QTEST_GUILESS_MAIN(NandModelTests)
#include "NandModelTests.moc"
