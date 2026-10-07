#include "backend/FlasherDeviceRegistry.hpp"

#include <QtTest/QTest>

using gxapi::backend::findDeviceByName;
using gxapi::backend::findDeviceByVidPid;

class FlasherDeviceRegistryTests final : public QObject {
  Q_OBJECT

private Q_SLOTS:
  void emptyNameMatchesNothing();
  void placeholderSelectionsMatchNothing();
  void partialNamesMatchNothing();
  void exactNamesMatchIgnoringCase();
  void vidPidLookupFindsProfile();
  void flashBackendFollowsProfile();
  void jtagBackendFollowsProfile();
  void jtagPartNamesByIdcode();
};

void FlasherDeviceRegistryTests::emptyNameMatchesNothing() {
  QVERIFY(!findDeviceByName("").has_value());
}

void FlasherDeviceRegistryTests::placeholderSelectionsMatchNothing() {
  QVERIFY(!findDeviceByName("None").has_value());
  QVERIFY(!findDeviceByName("UpdClient").has_value());
  QVERIFY(!findDeviceByName("UpdClient (Network)").has_value());
}

void FlasherDeviceRegistryTests::partialNamesMatchNothing() {
  QVERIFY(!findDeviceByName("x").has_value());
  QVERIFY(!findDeviceByName("Flasher").has_value());
  QVERIFY(!findDeviceByName("Pico").has_value());
  QVERIFY(!findDeviceByName("xFlasher (FTDI2SPI)").has_value());
}

void FlasherDeviceRegistryTests::exactNamesMatchIgnoringCase() {
  for (const auto &dev : gxapi::backend::KnownFlasherDevices) {
    const auto found = findDeviceByName(dev.displayName);
    QVERIFY(found.has_value());
    QCOMPARE(found->displayName, dev.displayName);
  }

  const auto xflasher = findDeviceByName("XFLASHER");
  QVERIFY(xflasher.has_value());
  QCOMPARE(xflasher->displayName, std::string("xFlasher"));

  const auto dirtyJtag = findDeviceByName("pico-dirtyjtag");
  QVERIFY(dirtyJtag.has_value());
  QCOMPARE(dirtyJtag->displayName, std::string("Pico-DirtyJTAG"));
}

void FlasherDeviceRegistryTests::vidPidLookupFindsProfile() {
  const auto xflasher = findDeviceByVidPid(0x0403, 0x6010);
  QVERIFY(xflasher.has_value());
  QCOMPARE(xflasher->displayName, std::string("xFlasher"));
  QVERIFY(!findDeviceByVidPid(0x1234, 0x5678).has_value());
}

void FlasherDeviceRegistryTests::flashBackendFollowsProfile() {
  using gxapi::backend::flashBackendFor;
  QCOMPARE(flashBackendFor("UpdClient"), std::string("UpdClient"));
  QCOMPARE(flashBackendFor("xFlasher"), std::string("FTDI2SPI"));
  QCOMPARE(flashBackendFor("PicoFlasher"), std::string("NandProMax"));
  QCOMPARE(flashBackendFor("TX DemoN"), std::string("NandProMax"));
  // No flash backend in the profile, no profile at all, or a flasher-list
  // label that is not the backend name: all fall back to NandProMax.
  QCOMPARE(flashBackendFor("Pico-DirtyJTAG"), std::string("NandProMax"));
  QCOMPARE(flashBackendFor("None"), std::string("NandProMax"));
  QCOMPARE(flashBackendFor(""), std::string("NandProMax"));
  QCOMPARE(flashBackendFor("UpdClient (Network)"), std::string("NandProMax"));
}

void FlasherDeviceRegistryTests::jtagBackendFollowsProfile() {
  using gxapi::backend::jtagBackendFor;
  QCOMPARE(jtagBackendFor("xFlasher"), std::string("xsvftool"));
  QCOMPARE(jtagBackendFor("Pico-DirtyJTAG"), std::string("xsvftool"));
  QCOMPARE(jtagBackendFor("Nand-X / LPC"), std::string("NandProMax"));
  QCOMPARE(jtagBackendFor("PicoFlasher"), std::string("NandProMax"));
  QCOMPARE(jtagBackendFor("UpdClient"), std::string("NandProMax"));
  QCOMPARE(jtagBackendFor("None"), std::string("NandProMax"));
}

void FlasherDeviceRegistryTests::jtagPartNamesByIdcode() {
  using gxapi::backend::findJtagPartName;
  QCOMPARE(findJtagPartName(0x06e5e093),
           std::optional<std::string_view>("XC2C64A"));
  QCOMPARE(findJtagPartName(0x06e5c093),
           std::optional<std::string_view>("XC2C64A"));
  QVERIFY(!findJtagPartName(0x06e5d093).has_value());
  QVERIFY(!findJtagPartName(0).has_value());
}

QTEST_GUILESS_MAIN(FlasherDeviceRegistryTests)
#include "FlasherDeviceRegistryTests.moc"
