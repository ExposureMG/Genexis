#include <QtTest>

#include "adapters/Gxbuild3.hpp"
#include "pages/Nand.hpp"
#include "utils/Files.hpp"
#include "utils/Network.hpp"
#include "utils/Settings.hpp"
#include "utils/files/XebuildIni.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlError>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

namespace files = genexis::utils::files;
namespace xebuildini = genexis::utils::files::xebuildini;
namespace settings = genexis::utils::settings;

bool contains(const std::vector<std::string> &items, std::string_view value) {
    return std::find(items.begin(), items.end(), value) != items.end();
}

void touchFile(const std::filesystem::path &path, std::string_view content = {}) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
}

} // namespace

class Phase1Tests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() {
        genexis::adapters::gxbuild3::initLogging();
    }

    void cleanupTestCase() {
        genexis::adapters::gxbuild3::shutdownLogging();
    }

    void testSettingsRoundTrip() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const QString iniPath = tempDir.filePath(QStringLiteral("nested/genexis.ini"));

        {
            settings::Settings cfg(iniPath);
            QCOMPARE(cfg.buildBackend(), QStringLiteral("gxbuild3"));
            QCOMPARE(cfg.flashBackend(), QStringLiteral("NandProMax"));
            QCOMPARE(cfg.timingFlashBackend(), QStringLiteral("NandProMax"));
            QCOMPARE(cfg.wirelessBackend(), QStringLiteral("UpdClient"));
            QVERIFY(cfg.xebuildDataPath().isEmpty());
            QVERIFY(cfg.outputDirPath().isEmpty());

            cfg.setBuildBackend(QStringLiteral("gxbuild3"));
            cfg.setFlashBackend(QStringLiteral("FTDI2SPI"));
            cfg.setTimingFlashBackend(QStringLiteral("xsvftool"));
            cfg.setWirelessBackend(QStringLiteral("UpdClient"));
            cfg.setXebuildDataPath(QStringLiteral("/tmp/custom-xebuild"));
            cfg.setOutputDirPath(QStringLiteral("/tmp/custom-output"));
            cfg.save();
        }

        {
            settings::Settings reloaded(iniPath);
            QCOMPARE(reloaded.buildBackend(), QStringLiteral("gxbuild3"));
            QCOMPARE(reloaded.flashBackend(), QStringLiteral("FTDI2SPI"));
            QCOMPARE(reloaded.timingFlashBackend(), QStringLiteral("xsvftool"));
            QCOMPARE(reloaded.wirelessBackend(), QStringLiteral("UpdClient"));
            QCOMPARE(reloaded.xebuildDataPath(), QStringLiteral("/tmp/custom-xebuild"));
            QCOMPARE(reloaded.outputDirPath(), QStringLiteral("/tmp/custom-output"));
        }
    }

    void testXebuildIniParsing() {
        const std::filesystem::path supportDir(GENEXIS_TEST_SUPPORT_FILES_DIR);
        const std::filesystem::path glitch2Ini = supportDir / "17559" / "_glitch2.ini";

        const auto parsed = xebuildini::XebuildIni::load(glitch2Ini);
        QVERIFY2(parsed.has_value(), parsed ? "" : parsed.error().c_str());

        QCOMPARE(parsed->version(), std::string("17559"));
        QVERIFY(parsed->hasSection("coronabl"));
        QVERIFY(parsed->hasSection("coronabl_WB"));
        QVERIFY(!parsed->entries("coronabl").empty());

        const auto targets = parsed->consoleTargets();
        auto hasSection = [&](std::string_view sec) {
            return std::any_of(targets.begin(), targets.end(),
                               [&](const xebuildini::ConsoleSection &c) { return c.section == sec; });
        };
        auto hasConsole = [&](std::string_view console) {
            return std::any_of(targets.begin(), targets.end(),
                               [&](const xebuildini::ConsoleSection &c) { return c.console == console; });
        };

        QVERIFY(hasConsole("xenon"));
        QVERIFY(hasConsole("zephyr"));
        QVERIFY(hasConsole("falcon"));
        QVERIFY(hasConsole("jasper"));
        QVERIFY(hasConsole("trinity"));
        QVERIFY(hasConsole("corona"));

        QVERIFY(hasSection("xenonbl_elpis"));
        QVERIFY(hasSection("coronabl_wb"));
        QVERIFY(hasSection("coronabl_wb4g"));

        QVERIFY(!hasSection("version"));
        QVERIFY(!hasSection("security"));
        QVERIFY(!hasSection("flashfs"));
        QVERIFY(!hasSection("rawpatch"));

        const auto rawParsed = xebuildini::XebuildIni::parseText(
            "[version]\n17559\n[rawpatch]\npatch.bin,0\n[coronabl]\ncba.bin,1234\n");
        QVERIFY(rawParsed.has_value());
        QCOMPARE(rawParsed->consoleTargets().size(), size_t{1});
        QCOMPARE(rawParsed->consoleTargets().front().console, std::string("corona"));

        const auto missing = xebuildini::XebuildIni::load(supportDir / "17559" / "_does_not_exist.ini");
        QVERIFY(!missing.has_value());
        QVERIFY(!missing.error().empty());
    }

    void testFilesDiscoveryAndHelpers() {
        const std::filesystem::path supportDir(GENEXIS_TEST_SUPPORT_FILES_DIR);

        const auto versions = files::listVersions(supportDir);
        QVERIFY(contains(versions, "17559"));

        const auto imageTypes = files::listImageTypes(supportDir, "17559");
        QVERIFY(contains(imageTypes, "devgl"));
        QVERIFY(contains(imageTypes, "glitch"));
        QVERIFY(contains(imageTypes, "glitch2"));
        QVERIFY(contains(imageTypes, "glitch2m"));
        QVERIFY(contains(imageTypes, "glitch3"));
        QVERIFY(contains(imageTypes, "jtag"));
        QVERIFY(contains(imageTypes, "retail"));

        const auto consoles = files::listConsoles(supportDir, "17559", "glitch2");
        QVERIFY(contains(consoles, "xenon"));
        QVERIFY(contains(consoles, "zephyr"));
        QVERIFY(contains(consoles, "falcon"));
        QVERIFY(contains(consoles, "jasper"));
        QVERIFY(contains(consoles, "trinity"));
        QVERIFY(contains(consoles, "corona"));
        QVERIFY(contains(consoles, "corona_wb"));
        QVERIFY(contains(consoles, "corona_wb4g"));
        QVERIFY(contains(consoles, "xenon_elpis"));

        const auto addons = files::listAddonPatches(supportDir, "17559");
        QVERIFY(contains(addons, "nohdmiwait"));
        QVERIFY(contains(addons, "nofcrt"));
        QVERIFY(contains(addons, "usbdsec"));
        QVERIFY(!contains(addons, "patches_g2trinity"));
        QVERIFY(!contains(addons, "patches_fat"));

        const auto validEnv = files::validateBuildEnvironment(supportDir);
        QVERIFY(validEnv.valid);
        QVERIFY(validEnv.errors.empty());
        QVERIFY(contains(validEnv.versions, "17559"));

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        const std::filesystem::path tempRoot = files::qStringToPath(tempDir.path());

        const auto invalidEnv = files::validateBuildEnvironment(tempRoot / "empty_xebuild");
        QVERIFY(!invalidEnv.valid);
        QVERIFY(!invalidEnv.errors.empty());

        // Numeric descending version ordering
        const std::filesystem::path mockXebuild = tempRoot / "xebuild";
        touchFile(mockXebuild / "9199" / "_jtag.ini", "[version]\n9199\n");
        touchFile(mockXebuild / "17559" / "_glitch2.ini", "[version]\n17559\n");
        touchFile(mockXebuild / "17526" / "_glitch2.ini", "[version]\n17526\n");
        touchFile(mockXebuild / "beta" / "_retail.ini", "[version]\nbeta\n");
        const auto sortedVersions = files::listVersions(mockXebuild);
        QCOMPARE(sortedVersions, (std::vector<std::string>{"17559", "17526", "9199", "beta"}));

        // SMC and XeLL case-insensitive folder discovery
        const std::filesystem::path mockSmc = tempRoot / "smc";
        touchFile(mockSmc / "Corona" / "smc_clean.bin", "SMC");
        QCOMPARE(files::listSmcFiles(mockSmc, "corona"),
                 (std::vector<std::string>{"smc_clean.bin"}));

        const std::filesystem::path mockXell = tempRoot / "xell";
        touchFile(mockXell / "RGH3" / "corona.bin", "XELL");
        QCOMPARE(files::listXellHacks(mockXell), (std::vector<std::string>{"RGH3"}));
        QCOMPARE(files::listXellImages(mockXell, "rgh3"),
                 (std::vector<std::string>{"corona.bin"}));

        // urlToPath
        const QString nandFilePath = tempDir.filePath(QStringLiteral("nand/flashdmp.bin"));
        const QUrl nandUrl = QUrl::fromLocalFile(nandFilePath);
        QCOMPARE(files::urlToPath(nandUrl), files::qStringToPath(nandFilePath));

        // Binary write & read
        const std::vector<uint8_t> payload{0xDE, 0xAD, 0xBE, 0xEF};
        const auto binPath = files::qStringToPath(nandFilePath);
        const auto writeRes = files::writeBinaryFile(binPath, payload);
        QVERIFY(writeRes.has_value());
        const auto readRes = files::readBinaryFile(binPath);
        QVERIFY(readRes.has_value());
        QCOMPARE(*readRes, payload);

        // detectSiblingCpuKey (case-insensitive cpukey.txt / CPUKEY.TXT)
        touchFile(binPath.parent_path() / "CPUKEY.TXT",
                  "CPU Key: 0123456789abcdef0123456789ABCDEF\n");
        const auto detected = files::detectSiblingCpuKey(binPath);
        QVERIFY(detected.has_value());
        QCOMPARE(*detected, QStringLiteral("0123456789ABCDEF0123456789ABCDEF"));
        QCOMPARE(genexis::pages::nand::NandSession::instance().detectCpuKey(nandUrl),
                 QStringLiteral("0123456789ABCDEF0123456789ABCDEF"));
    }

    void testNetworkClientInvalidUrl() {
        genexis::utils::network::NetworkClient client;
        bool called = false;
        bool succeeded = true;
        client.getJson(QUrl(), QStringLiteral("/status"), QUrlQuery{},
                       [&](std::expected<QJsonDocument, QString> res) {
                           called = true;
                           succeeded = res.has_value();
                       });
        QVERIFY(called);
        QVERIFY(!succeeded);
    }

    void testQmlPagesLoadWithoutErrors() {
        if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
            QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
        }

        QQmlApplicationEngine engine;
        QStringList errors;
        QObject::connect(&engine, &QQmlApplicationEngine::warnings, this,
                         [&](const QList<QQmlError> &warnings) {
                             for (const auto &w : warnings) {
                                 const QString msg = w.toString();
                                 if (msg.contains(QStringLiteral(
                                         "Created graphical object was not placed in the graphics scene"))) {
                                     continue;
                                 }
                                 errors.push_back(msg);
                             }
                         });

        const QUrl mainUrl(QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/gui/Main.qml"));
        engine.load(mainUrl);
        QVERIFY2(!engine.rootObjects().isEmpty(), qPrintable(errors.join(QChar::fromLatin1('\n'))));
        QObject *rootObj = engine.rootObjects().first();

        const QStringList pageUrls = {
            QStringLiteral("pages/Home.qml"),
            QStringLiteral("pages/NandInfo.qml"),
            QStringLiteral("pages/NandBuilder.qml"),
            QStringLiteral("pages/BadUpdate.qml"),
            QStringLiteral("pages/Flasher.qml"),
            QStringLiteral("pages/Settings.qml"),
        };

        for (const auto &pageUrl : pageUrls) {
            QVERIFY(QMetaObject::invokeMethod(rootObj, "switchPage",
                                              Q_ARG(QVariant, QVariant(pageUrl))));
            QCoreApplication::processEvents();
        }

        const QStringList subComponents = {
            QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/gui/components/nand/Overview.qml"),
            QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/gui/components/nand/Components.qml"),
            QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/gui/components/nand/Keyvault.qml"),
            QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/gui/components/NandBuilderSimple.qml"),
            QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/gui/components/NandBuilderAdvanced.qml"),
            QStringLiteral("qrc:/qt/qml/org/gxoss/genexis/gui/components/NandBuilderDonor.qml"),
        };

        for (const auto &compUrl : subComponents) {
            QQmlComponent comp(&engine, QUrl(compUrl));
            QVERIFY2(!comp.isError(), qPrintable(comp.errorString()));
            std::unique_ptr<QObject> item(comp.create());
            QVERIFY2(item != nullptr, qPrintable(comp.errorString()));
        }

        QVERIFY2(errors.isEmpty(), qPrintable(errors.join(QChar::fromLatin1('\n'))));
    }
};

QTEST_MAIN(Phase1Tests)
#include "Phase1Tests.moc"
