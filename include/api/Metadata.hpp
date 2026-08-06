#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

namespace gxapi::Metabuild {

// ---------------------------------------------------------------------------
// Enumerations
// ---------------------------------------------------------------------------

enum class ImageType {
  Retail,
  Dev,
  Glitch,
  Jtag,
  RGLoader,
  XDKBuild,
  Unknown
};

enum class ConsoleTarget {
  Xenon,
  Zephyr,
  Falcon,
  Jasper,
  Trinity,
  Corona,
  Winchester,
  Unknown
};

enum class BootloaderType {
  BL1,
  CB,
  CB_A,
  CB_B,
  CD,
  CE,
  CF,
  CG,
  Unknown
};

enum class PatchType {
  Binary,
  PpcAsm,
  Json,
  PatchSet,
  Unknown
};

enum class PatchTargetType {
  FullImage,
  Addon,
  Bootloader,
  Unknown
};

enum class FlashHardwareType {
  OSB,  // Old Small Block (Xenon 16MB)
  SB,   // Small Block (Zephyr / Falcon / Jasper / Trinity 16MB)
  BB,   // Big Block (Jasper / Kronos 256MB / 512MB)
  eMMC, // eMMC (Corona 4GB)
  Unknown
};

// ---------------------------------------------------------------------------
// Metabuild Sections & Structures
// ---------------------------------------------------------------------------

// 1. Meta Section
struct MetaSection {
  std::string name;
  ImageType imageType{ImageType::Glitch};
  ConsoleTarget consoleTarget{ConsoleTarget::Trinity};
  std::string buildType{"Glitch2"};
  uint32_t version{17559};
  std::vector<std::string> flags; // e.g. "wb2k", "wb4g", "elpis"
};

// 2. SMC Section
struct SmcSection {
  std::string firmwareType{"clean"};
  std::string firmwareVersion{"1.0"};
  std::string configType{"clean"};
  std::string configVersion{"1.0"};
};

// 3. Bootloader Descriptor (Identified by abstract ID, NO PATHS)
struct BootloaderDescriptor {
  std::string id;            // Abstract identifier e.g. "bl_cba"
  BootloaderType type{BootloaderType::CB_A};
  std::string version;       // Version e.g. "9188"
};

// 4. Patch Descriptor
struct PatchDescriptor {
  std::string name;
  PatchType type{PatchType::Binary};
  PatchTargetType target{PatchTargetType::FullImage};
  std::optional<std::string> targetBootloaderId; // Set if target == Bootloader
  uint32_t targetVersion{17559};
};

// 5. Payloads Section
struct PayloadsSection {
  struct XeLLPayload {
    std::string type{"xell_reloaded"};
    std::string version{"0.99"};
  } xell;

  struct PayloadEntry {
    std::string type;
    std::string version;
  };

  std::optional<PayloadEntry> rebooter;
  std::optional<PayloadEntry> payload;
  std::optional<PayloadEntry> fuses;
};

// 6. Complete Metabuild Profile Configuration
struct BuildConfig {
  MetaSection meta;
  SmcSection smc;
  std::vector<BootloaderDescriptor> bootloaders;
  std::vector<std::string> imageStructure; // Ordered list of bootloader IDs
  std::vector<PatchDescriptor> patches;
  PayloadsSection payloads;
  std::vector<std::string> flashfsFiles;
};

// ---------------------------------------------------------------------------
// Separate Runtime User Session (Personal Dumps, KV, Keys)
// ---------------------------------------------------------------------------

struct UserSession {
  std::optional<std::filesystem::path> sourceNandImage;
  std::optional<std::filesystem::path> keyvaultKv;
  std::optional<std::vector<uint8_t>> cpuKey;
  std::optional<std::vector<uint8_t>> oneBlKey;

  // Physical NAND controller architecture
  FlashHardwareType flashType{FlashHardwareType::SB};

  // Custom User Overrides
  std::optional<std::filesystem::path> smcOverridePath;
  std::optional<std::filesystem::path> customFlashfsDir;
};

// ---------------------------------------------------------------------------
// String Conversion Helpers
// ---------------------------------------------------------------------------

std::string ImageTypeToString(ImageType type);
ImageType StringToImageType(std::string_view sv);

std::string ConsoleTargetToString(ConsoleTarget target);
ConsoleTarget StringToConsoleTarget(std::string_view sv);

std::string BootloaderTypeToString(BootloaderType type);
BootloaderType StringToBootloaderType(std::string_view sv);

std::string PatchTypeToString(PatchType type);
PatchType StringToPatchType(std::string_view sv);

std::string PatchTargetTypeToString(PatchTargetType target);
PatchTargetType StringToPatchTargetType(std::string_view sv);

std::string FlashHardwareTypeToString(FlashHardwareType flash);
FlashHardwareType StringToFlashHardwareType(std::string_view sv);

// ---------------------------------------------------------------------------
// JSON Parser & Serializer Functions
// ---------------------------------------------------------------------------

nlohmann::json serializeBuildConfig(const BuildConfig& config);
std::expected<BuildConfig, std::string> parseBuildConfigJson(const nlohmann::json& j);
std::expected<BuildConfig, std::string> loadBuildConfigFromFile(const std::filesystem::path& jsonPath);

// ---------------------------------------------------------------------------
// xeBuild INI Converter
// ---------------------------------------------------------------------------

std::expected<BuildConfig, std::string> parseXeBuildIni(
    const std::filesystem::path& iniPath,
    ConsoleTarget consoleTarget,
    std::string_view buildType = "Glitch2");

std::expected<BuildConfig, std::string> parseXeBuildIniString(
    std::string_view iniContent,
    ConsoleTarget consoleTarget,
    std::string_view buildType = "Glitch2");

std::string generateXeBuildIniString(const BuildConfig& config);

std::expected<void, std::string> writeXeBuildIniFile(
    const BuildConfig& config,
    const std::filesystem::path& outputPath);

// ---------------------------------------------------------------------------
// RGBuild INI Converter
// ---------------------------------------------------------------------------

std::expected<BuildConfig, std::string> parseRGBuildIni(
    const std::filesystem::path& buildIniPath,
    const std::filesystem::path& bootloadersIniPath,
    ConsoleTarget consoleTarget,
    std::string_view buildType = "RGLoader");

std::expected<BuildConfig, std::string> parseRGBuildIniString(
    std::string_view buildIniContent,
    std::string_view bootloadersIniContent,
    ConsoleTarget consoleTarget,
    std::string_view buildType = "RGLoader");

std::string generateRGBuildIniString(const BuildConfig& config);
std::string generateRGBuildBootloadersIniString(const BuildConfig& config);

std::expected<void, std::string> writeRGBuildIniFiles(
    const BuildConfig& config,
    const std::filesystem::path& outputDir);

} // namespace gxapi::Metabuild
