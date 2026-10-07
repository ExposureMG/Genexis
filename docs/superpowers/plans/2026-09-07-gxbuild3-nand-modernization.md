# gxbuild3 NAND Modernization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make Genexis’s NAND inspection and NAND-building flows thin, tested Qt frontends over gxbuild3’s current public APIs.

**Architecture:** Add a pure C++ mapper from gxbuild3 `AllNandInfo` to the stable QML presentation fields. Replace manual construction of gxbuild3 `Input` with a translation from `NandBuildConfig` to `cli::BuildArgs`, a temporary override root, and `BuildInputResolver::Resolve`.

**Tech Stack:** C++23, Qt 6 Core/Test, CMake/CTest, gxbuild3 public API (`Library.hpp`, `cli/BuildInputResolver.hpp`).

**Spec:** `docs/superpowers/specs/2026-09-07-gxbuild3-nand-modernization-design.md`

## Global Constraints

- Keep existing QML `nandController` properties and signals stable.
- Outside the mapper/adapter, Genexis must not include gxbuild3 low-level NAND headers or construct `Input`.
- Inspect with `GxBuild::ExtractSomeInfo` before a CPU key and `GxBuild::ExtractAllInfo` after one.
- Build with `BuildInputResolver` then `GxBuild::RunBuild`.
- Linux and MSVC must compile the same gxbuild3 library source set.
- Preserve the user-owned `C:\\temp\\ida_mcp*` files and existing gxbuild3 submodule changes.
- Return an explicit unsupported-operation error for XeLL-only UI requests until gxbuild3 publishes an equivalent public resolver API.

---

## File Structure

| File | Responsibility |
| --- | --- |
| `include/pages/NandInfoMapper.hpp` | Defines `NandInfoSnapshot` and public/decrypted mapping functions. |
| `src/pages/NandInfoMapper.cpp` | Formats gxbuild3 NAND facts for stable QML properties and component-card roles. |
| `src/pages/Nand.cpp` | File I/O, thread handoff, QML state, and snapshot application; no direct NAND parsing. |
| `include/backend/adapters/GxBuild3Adapter.hpp` | Testable data-root constructor and request-resolution interface. |
| `src/backend/adapters/GxBuild3Adapter.cpp` | `BuildArgs` translation, override staging, resolution, execution, cleanup, and output writing. |
| `tests/NandInfoMapperTests.cpp` | Pure mapper tests. |
| `tests/GxBuild3AdapterTests.cpp` | Resolver and output integration tests against real gxbuild3 fixture data. |
| `cmake/GenexisGxBuild3MSVC.cmake` | gxbuild3 upstream source parity on Windows. |
| `CMakeLists.txt` | Mapper source and CTest target registration. |

### Task 1: Add a testable NAND-info mapper

**Files:**
- Create: `include/pages/NandInfoMapper.hpp`
- Create: `src/pages/NandInfoMapper.cpp`
- Create: `tests/NandInfoMapperTests.cpp`
- Modify: `CMakeLists.txt:120-240`

**Interfaces:**
- Produces `NandInfoSnapshot MapPublicNandInfo(const AllNandInfo&)`.
- Produces `NandInfoSnapshot MapDecryptedNandInfo(const AllNandInfo&, NandInfoSnapshot)`.
- `NandInfoSnapshot` has every current `Nand` presentation value and `QVariantList components`.

- [ ] **Step 1: Write the failing mapper tests**

```cpp
#include "pages/NandInfoMapper.hpp"
#include <QtTest/QTest>

class NandInfoMapperTests final : public QObject {
    Q_OBJECT
private slots:
    void publicInfoMapsGeometryAndBootloaders();
    void decryptedInfoEnrichesKeyvaultAndLockdownValues();
};

void NandInfoMapperTests::publicInfoMapsGeometryAndBootloaders() {
    AllNandInfo info{};
    info.block_type = ImageType::NewSmallBlock;
    info.smc = {.version = "1.02", .motherboard_name = "Trinity", .type_name = "Retail",
                .size = 12288, .present = true};
    info.bootloaders.cb_a = BootloaderEntryInfo{.name = "CB_A", .version = 13121,
                                                .size = 0x4000, .present = true};
    info.bootloaders.cd = BootloaderEntryInfo{.name = "CD", .version = 17559,
                                              .size = 0x8000, .present = true};

    const auto snapshot = MapPublicNandInfo(info);

    QCOMPARE(snapshot.imageSize, QStringLiteral("16MB"));
    QCOMPARE(snapshot.blockType, QStringLiteral("New Small Block"));
    QCOMPARE(snapshot.consoleTarget, QStringLiteral("Trinity"));
    QCOMPARE(snapshot.cbAVersion, QStringLiteral("13121"));
    QCOMPARE(snapshot.cdVersion, QStringLiteral("17559"));
    QCOMPARE(snapshot.components.size(), 3);
    QVERIFY(snapshot.serialNumber.isEmpty());
}
```

Create `decryptedInfoEnrichesKeyvaultAndLockdownValues` with serial number, DVD key, region, CB LDV `5`, and pairing bytes `12 34 56`; assert the stable presentation strings are populated.

- [ ] **Step 2: Register and run the test to verify it fails**

Create `genexis_nand_info_mapper_tests`, linked with `Qt6::Test`, `Qt6::Core`, and `gxbuild3_lib`, and register it as `nand_info_mapper`.

Run: `cmake -S . -B build -DBUILD_TESTING=ON && cmake --build build --target genexis_nand_info_mapper_tests -j2 && ctest --test-dir build -R nand_info_mapper --output-on-failure`

Expected: compile failure because `NandInfoMapper.hpp` and the mapping functions do not exist.

- [ ] **Step 3: Implement the smallest mapper that makes the tests pass**

```cpp
struct NandInfoSnapshot {
    QString imageSize, blockType, consoleTarget;
    QString smcVersion, smcType, smcSize, smcConfigOffset;
    QString cbVersion, cbAVersion, cbBVersion, cbXVersion, cbSize, cbMagic;
    QString scVersion, ccVersion, cdVersion, ceVersion;
    QString cf0Version, cg0Version, cf1Version, cg1Version;
    QString cbLdv, cbPairing, cbALdv, cbAPairing, cf0Ldv, cf0Pairing, cf1Ldv, cf1Pairing;
    QString serialNumber, consoleId, dvdKey, gameRegion, consoleType, kvVersion, ldvCount;
    QVariantList components;
};

NandInfoSnapshot MapPublicNandInfo(const AllNandInfo& info);
NandInfoSnapshot MapDecryptedNandInfo(const AllNandInfo& info, NandInfoSnapshot current);
```

Map `SmallBlock`/ `NewSmallBlock` to `16MB`, `BigBlock` to `64MB`, and `Emmc` to `48MB`. Build cards with existing `cardType`, `title`, `versionStr`, and `sizeStr` roles. Public mapping must not expose Keyvault contents; decrypted mapping sets Keyvault and per-box LDV/pairing values and replaces the existing encrypted Keyvault card version.

- [ ] **Step 4: Run the focused tests and existing compiler-policy suite**

Run: `cmake --build build --target genexis_nand_info_mapper_tests -j2 && ctest --test-dir build -R nand_info_mapper --output-on-failure && python -m unittest tests/test_compiler_policy.py -v`

Expected: mapper and policy tests pass.

- [ ] **Step 5: Commit the mapper**

```bash
git add CMakeLists.txt include/pages/NandInfoMapper.hpp src/pages/NandInfoMapper.cpp tests/NandInfoMapperTests.cpp
git commit -m "feat: add gxbuild3 NAND info mapper"
```

### Task 2: Make NAND Info consume gxbuild3 inspection APIs

**Files:**
- Modify: `src/pages/Nand.cpp:1-610`
- Modify: `include/pages/Nand.hpp:1-220`
- Modify: `tests/NandInfoMapperTests.cpp`

**Interfaces:**
- Consumes Task 1’s mapper.
- Calls `GxBuild::ExtractSomeInfo(const std::vector<uint8_t>&)` on NAND load.
- Calls `GxBuild::ExtractAllInfo(const std::vector<uint8_t>&, const std::vector<uint8_t>&)` after CPU-key validation.
- Retains all current `Nand` properties and signals.

- [ ] **Step 1: Add a failing encrypted-to-decrypted component test**

```cpp
void NandInfoMapperTests::decryptedInfoReplacesEncryptedKeyvaultCard() {
    AllNandInfo publicInfo{};
    publicInfo.keyvault = {.present = true, .decrypted = false};
    auto snapshot = MapPublicNandInfo(publicInfo);
    QCOMPARE(snapshot.components.last().toMap().value("versionStr").toString(),
             QStringLiteral("Encrypted"));

    AllNandInfo decryptedInfo{};
    decryptedInfo.keyvault = {.serial_number = "123456789012", .present = true,
                              .decrypted = true};
    snapshot = MapDecryptedNandInfo(decryptedInfo, std::move(snapshot));
    QCOMPARE(snapshot.components.last().toMap().value("versionStr").toString(),
             QStringLiteral("123456789012"));
}
```

- [ ] **Step 2: Run it to verify it fails for the expected missing behavior**

Run: `cmake --build build --target genexis_nand_info_mapper_tests -j2 && ctest --test-dir build -R nand_info_mapper --output-on-failure`

Expected: the new Keyvault-card assertion fails.

- [ ] **Step 3: Replace direct parser/decryption code**

Add private `applySnapshot(const NandInfoSnapshot&, bool decrypted)` and `currentSnapshot() const` methods. Implement the flow:

```cpp
void Nand::parseNandData(const std::vector<uint8_t>& bytes) {
    const auto info = GxBuild::ExtractSomeInfo(bytes);
    if (!info) return;
    applySnapshot(MapPublicNandInfo(*info), false);
}

void Nand::setCpuKey(const QString& cpuKey) {
    // Normalize exactly 32 hexadecimal characters and validate with gxbuild3.
    const auto info = GxBuild::ExtractAllInfo(m_rawNandData, keyBytes);
    if (!info) {
        m_isCpuKeyLoaded = false;
        Q_EMIT cpuKeyStateChanged();
        return;
    }
    applySnapshot(MapDecryptedNandInfo(*info, currentSnapshot()), true);
}
```

Delete `parseKeyvault`, `hexToBytes`, `bytesToHex`, direct `FlashImage`, SMC, Keyvault, and bootloader includes/usages, and the side effect that writes `nanddump.bin` into the xeBuild data directory. The builder will use its explicit donor path or an isolated staging root instead.

- [ ] **Step 4: Verify the migration**

Run: `cmake --build build --target genexis_nand_info_mapper_tests genexis -j2 && ctest --test-dir build -R nand_info_mapper --output-on-failure && rg -n 'FlashImage|BootloaderCb|BootloaderCf|keyvault_decrypt|smc_' src/pages/Nand.cpp`

Expected: test and application target pass; the final search has no matches.

- [ ] **Step 5: Commit the inspection migration**

```bash
git add include/pages/Nand.hpp src/pages/Nand.cpp src/pages/NandInfoMapper.cpp tests/NandInfoMapperTests.cpp
git commit -m "refactor: inspect NANDs through gxbuild3"
```

### Task 3: Translate builder selections into gxbuild3 resolver requests

**Files:**
- Modify: `include/backend/IBuilderService.hpp:10-35`
- Modify: `include/backend/adapters/GxBuild3Adapter.hpp:1-55`
- Modify: `src/backend/adapters/GxBuild3Adapter.cpp:1-610`
- Create: `tests/GxBuild3AdapterTests.cpp`
- Modify: `CMakeLists.txt:120-240`

**Interfaces:**
- Add `bool xellOnly{false};` to `NandBuildConfig`.
- Add `GxBuild3Adapter(std::filesystem::path xeBuildDataPath)` for tests.
- Produce `std::expected<gxbuild3::cli::BuildRequest, std::string> resolveBuildRequest(const NandBuildConfig&, const std::filesystem::path&) const`.
- `buildImage` uses only this request and `GxBuild::RunBuild`.

- [ ] **Step 1: Write failing resolver tests using real gxbuild3 data**

```cpp
class GxBuild3AdapterTests final : public QObject {
    Q_OBJECT
private slots:
    void resolvesDonorRetailBuildWithExplicitCpuKey();
    void resolvesGlitchBuildAndSelectedAddOn();
    void rejectsXellOnlyRequest();
};

void GxBuild3AdapterTests::rejectsXellOnlyRequest() {
    GxBuild3Adapter adapter{testDataRoot()};
    NandBuildConfig config{};
    config.xellOnly = true;

    const auto result = adapter.resolveBuildRequest(config, uniqueStagingRoot());

    QVERIFY(!result);
    QVERIFY(QString::fromStdString(result.error()).contains("XeLL"));
}
```

The fixture helper creates temporary `<version>/_retail.ini`, `<version>/_glitch2.ini`, `data`, `common`, and `bin/addon.bin` roots. Reuse gxbuild3’s valid bootloader/NAND construction pattern from `extern/gxbuild3/tests/CliFixtureGenerator.cpp`; use actual `nanddump.bin`, `kv.bin`, and `smc.bin` data, never mocks.

- [ ] **Step 2: Register and run the failing adapter test target**

Create `genexis_gxbuild3_adapter_tests`, linked with `Qt6::Test`, `Qt6::Core`, and `gxbuild3_lib`; register `gxbuild3_adapter`.

Run: `cmake --build build --target genexis_gxbuild3_adapter_tests -j2 && ctest --test-dir build -R gxbuild3_adapter --output-on-failure`

Expected: compilation fails because the constructor and `resolveBuildRequest` do not exist.

- [ ] **Step 3: Implement resolver-backed translation and staging**

Implement `resolveBuildRequest` as follows:

1. Reject `xellOnly`.
2. Resolve `Latest`, validate non-empty version/image type/console, and map image type to `BuildType` via gxbuild3’s public `kBuildTypeMap`.
3. Set `BuildArgs::build_ini` to `<version>/_<imageType>.ini`, `section` to the console stem, `build_type` to the mapped type, and `image_type` to the selected console’s public `kImageTypeMap` value.
4. Pass normalized CPU key as `BuildArgs::cpu_key`, donor path as `input_path`, target path as `output_path`, recognized UI options as ordered `name=value`/`name=true` config entries, and selected patches as extension-free `addons`.
5. Use ordered resolver roots `{stagingRoot, xeBuildDataPath/data, xeBuildDataPath/version, xeBuildDataPath/common}`.
6. Copy explicit KV/SMC files to `stagingRoot/kv.bin` and `stagingRoot/smc.bin`; copy an externally selected add-on only to `stagingRoot/bin/<name>.bin`. All copy and directory errors return contextual `std::unexpected` messages.
7. Instantiate `BuildInputResolver{xeBuildDataPath}`, call `Resolve(args)`, and append non-empty `ResolutionError::path` and `item` to the returned user-facing error.

Do not use `FileManager`, `OptionsManager`, `InputMetadata`, `Input`, or a process-wide current-directory change.

- [ ] **Step 4: Run the resolver tests and enforce the API boundary**

Run: `cmake --build build --target genexis_gxbuild3_adapter_tests -j2 && ctest --test-dir build -R gxbuild3_adapter --output-on-failure && rg -n 'FlashImage|FileManager|OptionsManager|InputMetadata|input\.overrides' src/backend/adapters/GxBuild3Adapter.cpp`

Expected: adapter tests pass and the final search produces no matches.

- [ ] **Step 5: Commit the resolver conversion**

```bash
git add CMakeLists.txt include/backend/IBuilderService.hpp include/backend/adapters/GxBuild3Adapter.hpp src/backend/adapters/GxBuild3Adapter.cpp tests/GxBuild3AdapterTests.cpp
git commit -m "refactor: resolve NAND builds through gxbuild3"
```

### Task 4: Wire execution, QML intent, and MSVC parity

**Files:**
- Modify: `src/backend/adapters/GxBuild3Adapter.cpp:buildImage`
- Modify: `src/pages/NandBuilderController.cpp:300-470`
- Modify: `cmake/GenexisGxBuild3MSVC.cmake:68-115`
- Modify: `tests/GxBuild3AdapterTests.cpp`

**Interfaces:**
- Consumes `resolveBuildRequest` from Task 3.
- Produces existing `IBuilderService::buildImage` results and existing controller signals.
- `NandBuilderController` maps QML aliases (`keyvaultPath`, `console`, `buildVersion`) into `NandBuildConfig` and sets `xellOnly`.

- [ ] **Step 1: Write a failing build-output test**

```cpp
void GxBuild3AdapterTests::buildImageWritesResolvedOutput() {
    GxBuild3Adapter adapter{testDataRoot()};
    const auto output = uniqueStagingRoot() / "updflash.bin";
    const auto result = adapter.buildImage(donorRetailConfig(output));

    QVERIFY(result);
    QVERIFY(result->success);
    QVERIFY(QFileInfo::exists(QString::fromStdString(result->outputPath)));
    QVERIFY(QFileInfo(QString::fromStdString(result->outputPath)).size() > 0);
}
```

Also assert in the controller-focused test helper that `buildType == "XeLL Image"` sets `xellOnly`, while simple/advanced/donor NAND builds retain their version/image type/console/options.

- [ ] **Step 2: Run the targeted test to verify it fails**

Run: `cmake --build build --target genexis_gxbuild3_adapter_tests -j2 && ctest --test-dir build -R gxbuild3_adapter --output-on-failure`

Expected: the output test fails because `buildImage` still manually builds obsolete gxbuild3 inputs.

- [ ] **Step 3: Replace execution and synchronize the MSVC source list**

Implement:

```cpp
const auto staging = createUniqueStagingRoot();
const auto request = resolveBuildRequest(config, staging);
if (!request) return std::unexpected(request.error());
const auto image = GxBuild::RunBuild(request->input);
if (!image) return std::unexpected(image.error().message);
if (!writeOutput(request->output_path, *image))
    return std::unexpected("Could not write output NAND: " + request->output_path.string());
return BuildResult{.success = true,
                   .outputPath = request->output_path.string(),
                   .logOutput = "Built NAND image: " + request->output_path.string()};
```

Use scoped cleanup to remove only the explicit unique staging directory after success or failure. Emit progress for validation/resolution, assembly, and output writing. In the controller, normalize QML field aliases before creating `NandBuildConfig`; do not write output or mutate packaged data before resolution succeeds.

Add the exact missing upstream gxbuild3 source files to the MSVC list, preserving upstream order:

```cmake
${GXBUILD3_ROOT}/src/cli/BuildCommand.cpp
${GXBUILD3_ROOT}/src/cli/BuildInputResolver.cpp
${GXBUILD3_ROOT}/src/cli/CommandLine.cpp
${GXBUILD3_ROOT}/src/InputValidator.cpp
```

- [ ] **Step 4: Run all verification**

Run: `cmake --build build --target genexis_nand_info_mapper_tests genexis_gxbuild3_adapter_tests genexis -j2 && ctest --test-dir build --output-on-failure && python -m unittest tests/test_compiler_policy.py -v`

Expected: both new C++ suites, enabled gxbuild3 tests, and compiler-policy tests pass; `genexis` compiles.

On a Windows/Craft-capable checkout also run: `cmake -S . -B build-vs -G Ninja -DBUILD_TESTING=ON && cmake --build build-vs --target genexis_gxbuild3_adapter_tests genexis`.

- [ ] **Step 5: Commit and inspect the completed migration**

```bash
git add CMakeLists.txt cmake/GenexisGxBuild3MSVC.cmake include/backend/IBuilderService.hpp \
  include/backend/adapters/GxBuild3Adapter.hpp src/backend/adapters/GxBuild3Adapter.cpp \
  src/pages/NandBuilderController.cpp tests/GxBuild3AdapterTests.cpp
git commit -m "feat: modernize Genexis NAND management"
git diff HEAD~1 --check
git status --short
```

Expected: only intentional migration files are committed. The pre-existing temp files and gxbuild3 submodule state remain unstaged.
