#include "backend/adapters/GxBuild3Adapter.hpp"
#include "../src/pages/NandBuildConfigMapper.hpp"

#include "BuildRunner.hpp"
#include "cli/BuildInputResolver.hpp"
#include "nand/FlashDriver.hpp"
#include "nand/bootloaders/2bl.hpp"
#include "nand/bootloaders/3bl.hpp"
#include "nand/bootloaders/4bl.hpp"
#include "nand/bootloaders/5bl.hpp"
#include "nand/bootloaders/6bl.hpp"
#include "nand/bootloaders/7bl.hpp"
#include "nand/objects/Keyvault.hpp"

#include <QtTest/QTest>
#include <QFileInfo>
#include <QTemporaryDir>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace {

using Bytes = std::vector<uint8_t>;
using gxapi::backend::GxBuild3Adapter;
using gxapi::backend::NandBuildConfig;
using gxbuild3::NAND::Keyvault;

constexpr std::array<uint8_t, 16> kCpuKey{
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0xe5, 0x8d,
};
constexpr std::string_view kCpuKeyHex = "FFFFFFFFFFFF1F0000000000006CE58D";
constexpr std::string_view kVersion = "17559";

bool writeBytes(const std::filesystem::path &path,
                std::span<const uint8_t> bytes) {
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output) {
    return false;
  }
  output.write(reinterpret_cast<const char *>(bytes.data()),
               static_cast<std::streamsize>(bytes.size()));
  return output.good();
}

bool writeText(const std::filesystem::path &path, std::string_view text) {
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output) {
    return false;
  }
  output << text;
  return output.good();
}

InputBootloaders validBootloaders() {
  BootloaderCb cb{};
  cb.header.header.magic = NANDBootloaderMagic::CB;
  cb.header.header.version = 1;
  cb.data.resize(0x380, 0);
  cb.header.header.size =
      static_cast<uint32_t>(sizeof(generic_header) + cb.data.size());
  cb.decrypted = true;

  BootloaderSc sc{};
  sc.header.header.magic = NANDBootloaderMagic::SC;
  sc.header.header.version = 2;
  sc.header.header.size = static_cast<uint32_t>(sizeof(sc_header) + 0x20);
  sc.data.assign(0x20, 0x53);
  sc.decrypted = true;
  auto cbForScKey = cb;
  cbForScKey.encrypt(key_1bl);
  if (!cbForScKey.derived_key) {
    return {};
  }
  sc.encrypt(cbForScKey.derived_key->data());

  BootloaderCd cd{};
  cd.header.header.magic = NANDBootloaderMagic::CD;
  cd.header.header.version = 3;
  cd.header.header.size = static_cast<uint32_t>(sizeof(cd_header) + 0x20);
  cd.header.ce_hash[0] = 1;
  cd.data.resize(0x20, 0x42);
  cd.decrypted = true;

  BootloaderCe ce{};
  ce.header.header.magic = NANDBootloaderMagic::CE;
  ce.header.header.version = 4;
  ce.header.header.size = static_cast<uint32_t>(sizeof(ce_header) + 0x20);
  ce.data.assign(0x20, 0x45);
  ce.decrypted = true;

  const auto makeCf = [](uint16_t version, uint8_t marker) {
    BootloaderCf cf{};
    cf.header.header.magic = NANDBootloaderMagic::CF;
    cf.header.header.version = version;
    cf.header.header.size = static_cast<uint32_t>(sizeof(cf_header) + 0x200);
    std::fill(std::begin(cf.header.cg_key), std::end(cf.header.cg_key), marker);
    cf.data.assign(0x200, 0);
    cf.data[2] = marker;
    cf.decrypted = true;
    return cf;
  };
  const auto makeCg = [](uint16_t version, uint8_t marker) {
    BootloaderCg cg{};
    cg.header.header.magic = NANDBootloaderMagic::CG;
    cg.header.header.version = version;
    cg.header.header.size = static_cast<uint32_t>(sizeof(cg_header) + 0x40);
    cg.header.source_size = 0x1000;
    cg.data.assign(0x40, marker);
    cg.decrypted = true;
    return cg;
  };

  auto cf0 = makeCf(5, 0x50);
  auto cg0 = makeCg(6, 0x60);
  cg0.encrypt(cf0.header.cg_key);
  auto cf1 = makeCf(7, 0x70);
  auto cg1 = makeCg(8, 0x80);
  cg1.encrypt(cf1.header.cg_key);

  InputBootloaders bootloaders{};
  bootloaders.cb_or_a = cb.serialize();
  bootloaders.sc = sc.serialize();
  bootloaders.cd = cd.serialize();
  bootloaders.ce = ce.serialize();
  bootloaders.cf0 = cf0.serialize();
  bootloaders.cg0 = cg0.serialize();
  bootloaders.cf1 = cf1.serialize();
  bootloaders.cg1 = cg1.serialize();
  return bootloaders;
}

std::optional<Bytes> donorNand(const InputBootloaders &bootloaders) {
  Input input{};
  input.image_type = ImageType::SmallBlock;
  input.metadata.cpu_key.assign(kCpuKey.begin(), kCpuKey.end());
  input.metadata.smc = Bytes(0x300, 0x61);
  (*input.metadata.smc)[0x100] = 0x10;
  input.metadata.keyvault = keyvault_decrypt(
      kCpuKey, keyvault_encrypt(kCpuKey, Bytes(Keyvault::kSize, 0x72)));
  input.bootloaders = bootloaders;
  const auto built = RunBuild(input);
  if (!built) {
    return std::nullopt;
  }
  return *built;
}

struct FixturePaths {
  std::filesystem::path donor;
  std::filesystem::path keyvault;
  std::filesystem::path smc;
  std::filesystem::path addon;
};

std::optional<FixturePaths> createFixture(const std::filesystem::path &root) {
  const auto version = root / kVersion;
  const auto data = root / "data";
  const auto common = root / "common";
  const auto versionBin = version / "bin";
  const auto external = root / "external";
  std::error_code error;
  std::filesystem::create_directories(data, error);
  std::filesystem::create_directories(common, error);
  std::filesystem::create_directories(versionBin, error);
  std::filesystem::create_directories(external, error);
  if (error) {
    return std::nullopt;
  }

  const auto bootloaders = validBootloaders();
  const auto donor = donorNand(bootloaders);
  if (!donor) {
    return std::nullopt;
  }

  constexpr std::string_view bootloaderEntries =
      "cb_1.bin\nsc.bin\ncd.bin\nce.bin\ncf_1.bin\ncg_1.bin\n"
      "cf_2.bin\ncg_2.bin\n";
  std::string ini;
  for (const std::string_view section : {
           "falconbl", "xenonbl", "xenonbl_1928", "zephyrbl",
           "zephyrbl_4572", "coronabl", "coronabl_WB", "mysterybl_1"}) {
    ini += "[" + std::string(section) + "]\n";
    ini += bootloaderEntries;
  }
  const Bytes kv = keyvault_encrypt(kCpuKey, Bytes(Keyvault::kSize, 0x35));
  Bytes smc(0x300, 0x44);
  smc[0x100] = 0x10;
  const Bytes automaticPatch{0x50, 0x41, 0x54, 0x43, 0x48};
  const Bytes addon{0x41, 0x44, 0x44, 0x4f, 0x4e};

  const FixturePaths paths{.donor = root / "nanddump.bin",
                           .keyvault = external / "kv-selected.bin",
                           .smc = external / "smc-selected.bin",
                           .addon = external / "addon.bin"};
  if (!writeBytes(paths.donor, *donor) ||
      !writeBytes(data / "cb_1.bin", bootloaders.cb_or_a) ||
      !writeBytes(data / "sc.bin", *bootloaders.sc) ||
      !writeBytes(data / "cd.bin", bootloaders.cd) ||
      !writeBytes(data / "ce.bin", *bootloaders.ce) ||
      !writeBytes(data / "cf_1.bin", *bootloaders.cf0) ||
      !writeBytes(data / "cg_1.bin", *bootloaders.cg0) ||
      !writeBytes(data / "cf_2.bin", *bootloaders.cf1) ||
      !writeBytes(data / "cg_2.bin", *bootloaders.cg1) ||
      !writeText(version / "_retail.ini", ini) ||
      !writeText(version / "_glitch2.ini", ini) ||
      !writeBytes(versionBin / "patches_g2falcon.bin", automaticPatch) ||
      !writeBytes(paths.keyvault, kv) || !writeBytes(paths.smc, smc) ||
      !writeBytes(paths.addon, addon)) {
    return std::nullopt;
  }
  return paths;
}

} // namespace

class GxBuild3AdapterTests final : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void resolvesDonorRetailBuildWithExplicitCpuKey();
  void resolvesAllConsolesDiscoveredFromVariantSections();
  void resolvesGlitchBuildAndSelectedAddOn();
  void rejectsXellOnlyRequest();
  void buildImageWritesResolvedOutput();
  void normalizesControllerBuildRequests();

private:
  std::unique_ptr<QTemporaryDir> m_fixtureDirectory;
  FixturePaths m_fixture;
};

void GxBuild3AdapterTests::initTestCase() {
  m_fixtureDirectory = std::make_unique<QTemporaryDir>();
  QVERIFY(m_fixtureDirectory->isValid());
  const auto fixture = createFixture(m_fixtureDirectory->path().toStdString());
  QVERIFY(fixture.has_value());
  m_fixture = *fixture;
}

void GxBuild3AdapterTests::resolvesDonorRetailBuildWithExplicitCpuKey() {
  QTemporaryDir staging;
  QVERIFY(staging.isValid());
  const auto stagingRoot =
      std::filesystem::path(staging.path().toStdString()) / "retail-staging";
  const auto output = stagingRoot / "updflash.bin";
  GxBuild3Adapter adapter{m_fixtureDirectory->path().toStdString()};
  NandBuildConfig config{};
  config.version = "Latest";
  config.imageType = "retail";
  config.consoleModel = "falcon";
  config.cpuKeyHex = std::string(kCpuKeyHex);
  config.outputPath = output.string();
  config.sourceNandPath = m_fixture.donor;
  config.customKvPath = m_fixture.keyvault;
  config.customSmcPath = m_fixture.smc;
  config.rawOptions = {
      {"nofcrt", "false"}, {"cleanSmc", ""}, {"nofcrt", ""}};

  const auto request = adapter.resolveBuildRequest(config, stagingRoot);

  QVERIFY2(request.has_value(),
           request ? "" : request.error().c_str());
  QCOMPARE(request->input.build_type, BuildType::Retail);
  QCOMPARE(request->input.image_type, ImageType::SmallBlock);
  QCOMPARE(request->input.metadata.cpu_key,
           Bytes(kCpuKey.begin(), kCpuKey.end()));
  QVERIFY(request->input.metadata.smc.has_value());
  Bytes selectedSmc(0x300, 0x44);
  selectedSmc[0x100] = 0x10;
  QCOMPARE(*request->input.metadata.smc, selectedSmc);
  QVERIFY(request->input.options.nofcrt.value_or(false));
  QCOMPARE(request->output_path, output);
  QVERIFY(std::filesystem::is_regular_file(stagingRoot / "kv.bin"));
  QVERIFY(std::filesystem::is_regular_file(stagingRoot / "smc.bin"));
}

void GxBuild3AdapterTests::resolvesAllConsolesDiscoveredFromVariantSections() {
  QTemporaryDir staging;
  QVERIFY(staging.isValid());
  const auto stagingRoot =
      std::filesystem::path(staging.path().toStdString()) / "discovered";
  GxBuild3Adapter adapter{m_fixtureDirectory->path().toStdString()};
  const auto consoles =
      adapter.getAvailableConsoles(std::string(kVersion), "retail");
  QVERIFY(!consoles.empty());
  QCOMPARE(consoles.size(), size_t{4});

  for (const auto &console : consoles) {
    QVERIFY2(kImageTypeMap.contains(console), console.c_str());
    NandBuildConfig config{};
    config.version = std::string(kVersion);
    config.imageType = "retail";
    config.consoleModel = console;
    config.cpuKeyHex = std::string(kCpuKeyHex);
    config.outputPath = (stagingRoot / console / "updflash.bin").string();
    config.sourceNandPath = m_fixture.donor;

    const auto request =
        adapter.resolveBuildRequest(config, stagingRoot / console);

    QVERIFY2(request.has_value(), request ? "" : request.error().c_str());
  }
}

void GxBuild3AdapterTests::resolvesGlitchBuildAndSelectedAddOn() {
  QTemporaryDir staging;
  QVERIFY(staging.isValid());
  const auto stagingRoot =
      std::filesystem::path(staging.path().toStdString()) / "glitch-staging";
  GxBuild3Adapter adapter{m_fixtureDirectory->path().toStdString()};
  NandBuildConfig config{};
  config.version = std::string(kVersion);
  config.imageType = "glitch2";
  config.consoleModel = "falcon";
  config.cpuKeyHex = std::string(kCpuKeyHex);
  config.outputPath = (stagingRoot / "updflash.bin").string();
  config.sourceNandPath = m_fixture.donor;
  config.patches = {m_fixture.addon.string()};

  const auto request = adapter.resolveBuildRequest(config, stagingRoot);

  QVERIFY2(request.has_value(),
           request ? "" : request.error().c_str());
  QCOMPARE(request->input.build_type, BuildType::Glitch2);
  QVERIFY(request->input.patches.has_value());
  QVERIFY(request->input.patches->automatic.has_value());
  QCOMPARE(request->input.patches->automatic->name,
           std::string("patches_g2falcon.bin"));
  QCOMPARE(request->input.patches->addons.size(), size_t{1});
  QCOMPARE(request->input.patches->addons.front().name,
           std::string("addon.bin"));
  QVERIFY(std::filesystem::is_regular_file(stagingRoot / "bin/addon.bin"));
}

void GxBuild3AdapterTests::rejectsXellOnlyRequest() {
  QTemporaryDir staging;
  QVERIFY(staging.isValid());
  GxBuild3Adapter adapter{m_fixtureDirectory->path().toStdString()};
  NandBuildConfig config{};
  config.xellOnly = true;

  const auto result = adapter.resolveBuildRequest(
      config, std::filesystem::path(staging.path().toStdString()) / "xell");

  QVERIFY(!result);
  QVERIFY(QString::fromStdString(result.error()).contains("XeLL"));
}

void GxBuild3AdapterTests::buildImageWritesResolvedOutput() {
  QTemporaryDir outputDirectory;
  QVERIFY(outputDirectory.isValid());
  const auto output =
      std::filesystem::path(outputDirectory.path().toStdString()) /
      "nested" / "updflash.bin";
  GxBuild3Adapter adapter{m_fixtureDirectory->path().toStdString()};
  NandBuildConfig config{};
  config.version = std::string(kVersion);
  config.imageType = "retail";
  config.consoleModel = "falcon";
  config.cpuKeyHex = std::string(kCpuKeyHex);
  config.outputPath = output.string();
  config.sourceNandPath = m_fixture.donor;

  const auto result = adapter.buildImage(config);

  QVERIFY2(result.has_value(), result ? "" : result.error().c_str());
  QVERIFY(result->success);
  QCOMPARE(result->outputPath, output.string());
  const QFileInfo outputInfo(QString::fromStdString(result->outputPath));
  QVERIFY(outputInfo.exists());
  QVERIFY(outputInfo.size() > 0);
}

void GxBuild3AdapterTests::normalizesControllerBuildRequests() {
  using gxapi::pages::detail::NandBuildConfigDefaults;
  using gxapi::pages::detail::normalizeNandBuildConfig;

  const NandBuildConfigDefaults defaults{
      .cpuKeyHex = QString::fromLatin1(kCpuKeyHex),
      .version = QStringLiteral("17559"),
      .imageType = QStringLiteral("glitch2"),
      .consoleModel = QStringLiteral("falcon"),
      .outputPath = QStringLiteral("/tmp/genexis-test/updflash.bin")};

  const auto xell = normalizeNandBuildConfig(
      {{QStringLiteral("buildType"), QStringLiteral("XeLL Image")}},
      defaults);
  QVERIFY(xell.xellOnly);

  const auto localizedXell = normalizeNandBuildConfig(
      {{QStringLiteral("buildType"), QStringLiteral("Imagen XeLL")},
       {QStringLiteral("xellOnly"), true}},
      defaults);
  QVERIFY(localizedXell.xellOnly);
  const auto localizedNand = normalizeNandBuildConfig(
      {{QStringLiteral("buildType"), QStringLiteral("Imagen NAND")},
       {QStringLiteral("xellOnly"), false}},
      defaults);
  QVERIFY(!localizedNand.xellOnly);

  const auto simple = normalizeNandBuildConfig(
      {{QStringLiteral("buildType"), QStringLiteral("NAND Image")},
       {QStringLiteral("buildVersion"), QStringLiteral("Latest")},
       {QStringLiteral("imageType"), QStringLiteral("FreeBoot")},
       {QStringLiteral("hackVersion"), QStringLiteral("RGH 2")},
       {QStringLiteral("patches"),
        QVariantList{QStringLiteral("launch"), QStringLiteral("xam")}}},
      defaults);
  QVERIFY(!simple.xellOnly);
  QCOMPARE(simple.version, std::string("17559"));
  QCOMPARE(simple.imageType, std::string("glitch2"));
  QCOMPARE(simple.consoleModel, std::string("falcon"));
  QCOMPARE(simple.patches,
           std::vector<std::string>({"launch", "xam"}));

  const auto advanced = normalizeNandBuildConfig(
      {{QStringLiteral("buildType"), QStringLiteral("NAND Image")},
       {QStringLiteral("version"), QStringLiteral("17489")},
       {QStringLiteral("imageType"), QStringLiteral("retail")},
       {QStringLiteral("console"), QStringLiteral("zephyr")},
       {QStringLiteral("options"),
        QVariantMap{{QStringLiteral("nofcrt"), true},
                    {QStringLiteral("kvPath"),
                     QStringLiteral("/tmp/advanced-kv.bin")},
                    {QStringLiteral("smcPath"),
                     QStringLiteral("/tmp/advanced-smc.bin")}}}},
      defaults);
  QCOMPARE(advanced.version, std::string("17489"));
  QCOMPARE(advanced.imageType, std::string("retail"));
  QCOMPARE(advanced.consoleModel, std::string("zephyr"));
  QCOMPARE(advanced.customKvPath,
           std::optional<std::filesystem::path>{"/tmp/advanced-kv.bin"});
  QCOMPARE(advanced.customSmcPath,
           std::optional<std::filesystem::path>{"/tmp/advanced-smc.bin"});
  QVERIFY(std::ranges::contains(advanced.rawOptions,
                               std::pair<std::string, std::string>{"nofcrt", ""}));

  const auto donor = normalizeNandBuildConfig(
      {{QStringLiteral("mode"), QStringLiteral("donor")},
       {QStringLiteral("cpuKey"), QString::fromLatin1(kCpuKeyHex)},
       {QStringLiteral("keyvaultPath"),
        QStringLiteral("/tmp/donor-kv.bin")},
       {QStringLiteral("cfLdv"), 4},
       {QStringLiteral("version"), QStringLiteral("17559")},
       {QStringLiteral("imageType"), QStringLiteral("retail")},
       {QStringLiteral("consoleModel"), QStringLiteral("falcon")},
       {QStringLiteral("options"),
        QVariantMap{{QStringLiteral("nofcrt"), true}}}},
      defaults);
  QCOMPARE(donor.version, std::string("17559"));
  QCOMPARE(donor.imageType, std::string("retail"));
  QCOMPARE(donor.consoleModel, std::string("falcon"));
  QCOMPARE(donor.customKvPath,
           std::optional<std::filesystem::path>{"/tmp/donor-kv.bin"});
  QVERIFY(std::ranges::contains(
      donor.rawOptions,
      std::pair<std::string, std::string>{"cfldv", "4"}));
  QVERIFY(std::ranges::contains(
      donor.rawOptions,
      std::pair<std::string, std::string>{"nofcrt", ""}));
}

QTEST_MAIN(GxBuild3AdapterTests)
#include "GxBuild3AdapterTests.moc"
