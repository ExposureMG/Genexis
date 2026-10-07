#include "FilePaths.hpp"
#include "pages/FlasherOperations.hpp"

#include <QtTest/QTest>

namespace backend = gxapi::backend;
namespace detail = gxapi::pages::detail;

class FlasherOperationsTests final : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void fileUrlsBecomeLocalPaths();
  void plainPathsAreKept();
  void timingFilesAreDetectedByExtension();
  void networkTargetUsesUpdClientAndIp();
  void usbTargetUsesSelectedProfile();
  void unsupportedByNamesProfilesLackingTheBackend();
  void jtagChainFormatting();
  void flashInfoFormatting();
  void toResultMapsExpected();
};

void FlasherOperationsTests::fileUrlsBecomeLocalPaths() {
  QCOMPARE(gxapi::toLocalPath(QStringLiteral("file:///tmp/nand dump.bin")),
           QStringLiteral("/tmp/nand dump.bin"));
}

void FlasherOperationsTests::plainPathsAreKept() {
  QCOMPARE(gxapi::toLocalPath(QStringLiteral("/tmp/nand.bin")),
           QStringLiteral("/tmp/nand.bin"));
  QCOMPARE(gxapi::toLocalPath(QStringLiteral(" /tmp/x.bin ")),
           QStringLiteral(" /tmp/x.bin "));
  QVERIFY(gxapi::toLocalPath(QString()).isEmpty());
}

void FlasherOperationsTests::timingFilesAreDetectedByExtension() {
  QVERIFY(gxapi::isJtagTimingFile(QStringLiteral("/t/timing.svf")));
  QVERIFY(gxapi::isJtagTimingFile(QStringLiteral("/t/TIMING.XSVF")));
  QVERIFY(gxapi::isJtagTimingFile(QStringLiteral("file:///t/timing.xsvf")));
  QVERIFY(!gxapi::isJtagTimingFile(QStringLiteral("/t/nand.bin")));
  QVERIFY(!gxapi::isJtagTimingFile(QStringLiteral("/t/svf")));
  QVERIFY(!gxapi::isJtagTimingFile(QString()));
}

void FlasherOperationsTests::networkTargetUsesUpdClientAndIp() {
  const auto target = detail::makeTarget(true, QStringLiteral("xFlasher"),
                                         QStringLiteral("10.0.0.5"));
  QCOMPARE(target.hardwareName, std::string(backend::kUpdClientBackend));
  QVERIFY(!target.profile.has_value());
  QCOMPARE(detail::flashConfig(target).ipAddress, std::string("10.0.0.5"));
  QVERIFY(detail::jtagConfig(target).backend.empty());
}

void FlasherOperationsTests::usbTargetUsesSelectedProfile() {
  const auto target = detail::makeTarget(false, QStringLiteral("xFlasher"),
                                         QStringLiteral("10.0.0.5"));
  QCOMPARE(target.hardwareName, std::string("xFlasher"));
  QVERIFY(target.profile.has_value());
  QCOMPARE(detail::jtagConfig(target).backend, std::string("FTDI"));
  QCOMPARE(detail::flashConfig(target).ipAddress,
           backend::FlashDeviceConfig{}.ipAddress);

  const auto none = detail::makeTarget(false, QStringLiteral("None"), {});
  QVERIFY(!none.profile.has_value());
}

void FlasherOperationsTests::unsupportedByNamesProfilesLackingTheBackend() {
  const auto dirtyJtag =
      detail::makeTarget(false, QStringLiteral("Pico-DirtyJTAG"), {});
  QCOMPARE(detail::unsupportedBy(dirtyJtag, false),
           std::optional<QString>(QStringLiteral("Pico-DirtyJTAG")));
  QVERIFY(!detail::unsupportedBy(dirtyJtag, true).has_value());

  const auto pico =
      detail::makeTarget(false, QStringLiteral("PicoFlasher"), {});
  QCOMPARE(detail::unsupportedBy(pico, true),
           std::optional<QString>(QStringLiteral("PicoFlasher")));
  QVERIFY(!detail::unsupportedBy(pico, false).has_value());

  const auto network = detail::makeTarget(true, {}, {});
  QVERIFY(!detail::unsupportedBy(network, true).has_value());
  QVERIFY(!detail::unsupportedBy(network, false).has_value());
}

void FlasherOperationsTests::jtagChainFormatting() {
  QCOMPARE(detail::formatJtagChain({}),
           QStringLiteral("JTAG Chain: 0 device(s)"));
  QCOMPARE(detail::formatJtagChain({0x06e5e093, 0x00001234}),
           QStringLiteral("JTAG Chain: 2 device(s)"
                          " | Device 0: XC2C64A (IDCODE: 0x06e5e093)"
                          " | Device 1: Unknown JTAG (IDCODE: 0x00001234)"));
}

void FlasherOperationsTests::flashInfoFormatting() {
  const backend::FlashInfo info{.hardwareName = "PicoFlasher",
                                .flashType = "eMMC",
                                .configWord = 0x00AA3020,
                                .totalBlocks = 1024,
                                .totalBytes = 16ull * 1024 * 1024 + 5};
  QCOMPARE(detail::formatFlashInfo(info),
           QStringLiteral("Config: 0x00aa3020 | Size: 16MB | eMMC"));
}

void FlasherOperationsTests::toResultMapsExpected() {
  const auto ok = detail::toResult(std::expected<void, std::string>{},
                                   QStringLiteral("ok"));
  QVERIFY(ok.success);
  QCOMPARE(ok.message, QStringLiteral("ok"));

  const auto failed = detail::toResult(std::unexpected(std::string("boom")),
                                       QStringLiteral("ok"));
  QVERIFY(!failed.success);
  QCOMPARE(failed.message, QStringLiteral("boom"));
}

QTEST_GUILESS_MAIN(FlasherOperationsTests)
#include "FlasherOperationsTests.moc"
