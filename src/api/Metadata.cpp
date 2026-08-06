#include "api/Metadata.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace gxapi::Metabuild {

namespace {

static std::string ToLower(std::string_view sv) {
  std::string result(sv);
  std::transform(result.begin(), result.end(), result.begin(), ::tolower);
  return result;
}

} // namespace

// ---------------------------------------------------------------------------
// String Conversion Functions
// ---------------------------------------------------------------------------

std::string ImageTypeToString(ImageType type) {
  switch (type) {
  case ImageType::Retail:
    return "Retail";
  case ImageType::Dev:
    return "Dev";
  case ImageType::Glitch:
    return "Glitch";
  case ImageType::Jtag:
    return "Jtag";
  case ImageType::RGLoader:
    return "RGLoader";
  case ImageType::XDKBuild:
    return "XDKBuild";
  default:
    return "Unknown";
  }
}

ImageType StringToImageType(std::string_view sv) {
  std::string s = ToLower(sv);
  if (s == "retail")
    return ImageType::Retail;
  if (s == "dev" || s == "devkit")
    return ImageType::Dev;
  if (s == "glitch" || s == "rgh")
    return ImageType::Glitch;
  if (s == "jtag" || s == "rjtag")
    return ImageType::Jtag;
  if (s == "rgloader")
    return ImageType::RGLoader;
  if (s == "xdkbuild")
    return ImageType::XDKBuild;
  return ImageType::Unknown;
}

std::string ConsoleTargetToString(ConsoleTarget target) {
  switch (target) {
  case ConsoleTarget::Xenon:
    return "Xenon";
  case ConsoleTarget::Zephyr:
    return "Zephyr";
  case ConsoleTarget::Falcon:
    return "Falcon";
  case ConsoleTarget::Jasper:
    return "Jasper";
  case ConsoleTarget::Trinity:
    return "Trinity";
  case ConsoleTarget::Corona:
    return "Corona";
  case ConsoleTarget::Winchester:
    return "Winchester";
  default:
    return "Unknown";
  }
}

ConsoleTarget StringToConsoleTarget(std::string_view sv) {
  std::string s = ToLower(sv);
  if (s == "xenon")
    return ConsoleTarget::Xenon;
  if (s == "zephyr")
    return ConsoleTarget::Zephyr;
  if (s == "falcon")
    return ConsoleTarget::Falcon;
  if (s == "jasper")
    return ConsoleTarget::Jasper;
  if (s == "trinity")
    return ConsoleTarget::Trinity;
  if (s == "corona")
    return ConsoleTarget::Corona;
  if (s == "winchester")
    return ConsoleTarget::Winchester;
  return ConsoleTarget::Unknown;
}

std::string BootloaderTypeToString(BootloaderType type) {
  switch (type) {
  case BootloaderType::BL1:
    return "1BL";
  case BootloaderType::CB:
    return "CB";
  case BootloaderType::CB_A:
    return "CB_A";
  case BootloaderType::CB_B:
    return "CB_B";
  case BootloaderType::CD:
    return "CD";
  case BootloaderType::CE:
    return "CE";
  case BootloaderType::CF:
    return "CF";
  case BootloaderType::CG:
    return "CG";
  default:
    return "Unknown";
  }
}

BootloaderType StringToBootloaderType(std::string_view sv) {
  std::string s = ToLower(sv);
  if (s == "1bl" || s == "bl1")
    return BootloaderType::BL1;
  if (s == "cb")
    return BootloaderType::CB;
  if (s == "cb_a" || s == "cba")
    return BootloaderType::CB_A;
  if (s == "cb_b" || s == "cbb")
    return BootloaderType::CB_B;
  if (s == "cd")
    return BootloaderType::CD;
  if (s == "ce")
    return BootloaderType::CE;
  if (s == "cf")
    return BootloaderType::CF;
  if (s == "cg")
    return BootloaderType::CG;
  return BootloaderType::Unknown;
}

std::string PatchTypeToString(PatchType type) {
  switch (type) {
  case PatchType::Binary:
    return "binary";
  case PatchType::PpcAsm:
    return "ppc_asm";
  case PatchType::Json:
    return "json";
  case PatchType::PatchSet:
    return "patchset";
  default:
    return "unknown";
  }
}

PatchType StringToPatchType(std::string_view sv) {
  std::string s = ToLower(sv);
  if (s == "binary")
    return PatchType::Binary;
  if (s == "ppc_asm" || s == "asm")
    return PatchType::PpcAsm;
  if (s == "json")
    return PatchType::Json;
  if (s == "patchset")
    return PatchType::PatchSet;
  return PatchType::Unknown;
}

std::string PatchTargetTypeToString(PatchTargetType target) {
  switch (target) {
  case PatchTargetType::FullImage:
    return "full_image";
  case PatchTargetType::Addon:
    return "addon";
  case PatchTargetType::Bootloader:
    return "bootloader";
  default:
    return "unknown";
  }
}

PatchTargetType StringToPatchTargetType(std::string_view sv) {
  std::string s = ToLower(sv);
  if (s == "full_image" || s == "image")
    return PatchTargetType::FullImage;
  if (s == "addon")
    return PatchTargetType::Addon;
  if (s == "bootloader" || s == "bl")
    return PatchTargetType::Bootloader;
  return PatchTargetType::Unknown;
}

std::string FlashHardwareTypeToString(FlashHardwareType flash) {
  switch (flash) {
  case FlashHardwareType::OSB:
    return "OSB";
  case FlashHardwareType::SB:
    return "SB";
  case FlashHardwareType::BB:
    return "BB";
  case FlashHardwareType::eMMC:
    return "eMMC";
  default:
    return "Unknown";
  }
}

FlashHardwareType StringToFlashHardwareType(std::string_view sv) {
  std::string s = ToLower(sv);
  if (s == "osb")
    return FlashHardwareType::OSB;
  if (s == "sb")
    return FlashHardwareType::SB;
  if (s == "bb" || s == "bigblock")
    return FlashHardwareType::BB;
  if (s == "emmc" || s == "mmc")
    return FlashHardwareType::eMMC;
  return FlashHardwareType::Unknown;
}

// ---------------------------------------------------------------------------
// JSON Serialization
// ---------------------------------------------------------------------------

nlohmann::json serializeBuildConfig(const BuildConfig& config) {
  nlohmann::json j;

  // 1. meta
  j["meta"] = {
      {"name", config.meta.name},
      {"image_type", ImageTypeToString(config.meta.imageType)},
      {"console_target", ConsoleTargetToString(config.meta.consoleTarget)},
      {"build_type", config.meta.buildType},
      {"version", config.meta.version},
      {"flags", config.meta.flags}};

  // 2. smc
  j["smc"] = {
      {"firmware", {{"type", config.smc.firmwareType}, {"version", config.smc.firmwareVersion}}},
      {"config", {{"type", config.smc.configType}, {"version", config.smc.configVersion}}}};

  // 3. bootloaders
  nlohmann::json blArray = nlohmann::json::array();
  for (const auto& bl : config.bootloaders) {
    blArray.push_back({
        {"id", bl.id},
        {"type", BootloaderTypeToString(bl.type)},
        {"version", bl.version}});
  }
  j["bootloaders"] = blArray;

  // 4. image_structure
  j["image_structure"] = config.imageStructure;

  // 5. patches
  nlohmann::json patchArray = nlohmann::json::array();
  for (const auto& p : config.patches) {
    nlohmann::json pj = {
        {"name", p.name},
        {"type", PatchTypeToString(p.type)},
        {"target", PatchTargetTypeToString(p.target)},
        {"target_version", p.targetVersion}};
    if (p.targetBootloaderId) {
      pj["target_bootloader_id"] = *p.targetBootloaderId;
    }
    patchArray.push_back(pj);
  }
  j["patches"] = patchArray;

  // 6. payloads
  nlohmann::json payloadObj;
  payloadObj["xell"] = {
      {"type", config.payloads.xell.type},
      {"version", config.payloads.xell.version}};

  if (config.payloads.rebooter) {
    payloadObj["rebooter"] = {
        {"type", config.payloads.rebooter->type},
        {"version", config.payloads.rebooter->version}};
  }
  if (config.payloads.payload) {
    payloadObj["payload"] = {
        {"type", config.payloads.payload->type},
        {"version", config.payloads.payload->version}};
  }
  if (config.payloads.fuses) {
    payloadObj["fuses"] = {
        {"type", config.payloads.fuses->type},
        {"version", config.payloads.fuses->version}};
  }
  j["payloads"] = payloadObj;

  // 7. flashfs
  j["flashfs"] = config.flashfsFiles;

  return j;
}

// ---------------------------------------------------------------------------
// JSON Parsing
// ---------------------------------------------------------------------------

std::expected<BuildConfig, std::string> parseBuildConfigJson(const nlohmann::json& j) {
  if (!j.is_object()) {
    return std::unexpected("Root JSON element must be an object.");
  }

  BuildConfig config;

  // 1. meta
  if (!j.contains("meta") || !j["meta"].is_object()) {
    return std::unexpected("Missing or invalid 'meta' section.");
  }
  const auto& metaJ = j["meta"];
  if (metaJ.contains("name") && metaJ["name"].is_string()) {
    config.meta.name = metaJ["name"].get<std::string>();
  }
  if (metaJ.contains("image_type") && metaJ["image_type"].is_string()) {
    config.meta.imageType = StringToImageType(metaJ["image_type"].get<std::string>());
  }
  if (metaJ.contains("console_target") && metaJ["console_target"].is_string()) {
    config.meta.consoleTarget = StringToConsoleTarget(metaJ["console_target"].get<std::string>());
  }
  if (metaJ.contains("build_type") && metaJ["build_type"].is_string()) {
    config.meta.buildType = metaJ["build_type"].get<std::string>();
  }
  if (metaJ.contains("version") && metaJ["version"].is_number_integer()) {
    config.meta.version = metaJ["version"].get<uint32_t>();
  }
  if (metaJ.contains("flags") && metaJ["flags"].is_array()) {
    for (const auto& item : metaJ["flags"]) {
      if (item.is_string()) {
        config.meta.flags.push_back(item.get<std::string>());
      }
    }
  }

  // 2. smc
  if (j.contains("smc") && j["smc"].is_object()) {
    const auto& smcJ = j["smc"];
    if (smcJ.contains("firmware") && smcJ["firmware"].is_object()) {
      if (smcJ["firmware"].contains("type") && smcJ["firmware"]["type"].is_string()) {
        config.smc.firmwareType = smcJ["firmware"]["type"].get<std::string>();
      }
      if (smcJ["firmware"].contains("version") && smcJ["firmware"]["version"].is_string()) {
        config.smc.firmwareVersion = smcJ["firmware"]["version"].get<std::string>();
      }
    }
    if (smcJ.contains("config") && smcJ["config"].is_object()) {
      if (smcJ["config"].contains("type") && smcJ["config"]["type"].is_string()) {
        config.smc.configType = smcJ["config"]["type"].get<std::string>();
      }
      if (smcJ["config"].contains("version") && smcJ["config"]["version"].is_string()) {
        config.smc.configVersion = smcJ["config"]["version"].get<std::string>();
      }
    }
  }

  // 3. bootloaders
  if (j.contains("bootloaders") && j["bootloaders"].is_array()) {
    for (const auto& blJ : j["bootloaders"]) {
      if (!blJ.is_object()) continue;
      BootloaderDescriptor bl;
      if (blJ.contains("id") && blJ["id"].is_string()) {
        bl.id = blJ["id"].get<std::string>();
      }
      if (blJ.contains("type") && blJ["type"].is_string()) {
        bl.type = StringToBootloaderType(blJ["type"].get<std::string>());
      }
      if (blJ.contains("version")) {
        if (blJ["version"].is_string()) {
          bl.version = blJ["version"].get<std::string>();
        } else if (blJ["version"].is_number_integer()) {
          bl.version = std::to_string(blJ["version"].get<uint32_t>());
        }
      }
      config.bootloaders.push_back(bl);
    }
  }

  // 4. image_structure
  if (j.contains("image_structure") && j["image_structure"].is_array()) {
    for (const auto& item : j["image_structure"]) {
      if (item.is_string()) {
        config.imageStructure.push_back(item.get<std::string>());
      }
    }
  }

  // 5. patches
  if (j.contains("patches") && j["patches"].is_array()) {
    for (const auto& pJ : j["patches"]) {
      if (!pJ.is_object()) continue;
      PatchDescriptor patch;
      if (pJ.contains("name") && pJ["name"].is_string()) {
        patch.name = pJ["name"].get<std::string>();
      }
      if (pJ.contains("type") && pJ["type"].is_string()) {
        patch.type = StringToPatchType(pJ["type"].get<std::string>());
      }
      if (pJ.contains("target") && pJ["target"].is_string()) {
        patch.target = StringToPatchTargetType(pJ["target"].get<std::string>());
      }
      if (pJ.contains("target_bootloader_id") && pJ["target_bootloader_id"].is_string()) {
        patch.targetBootloaderId = pJ["target_bootloader_id"].get<std::string>();
      }
      if (pJ.contains("target_version") && pJ["target_version"].is_number_integer()) {
        patch.targetVersion = pJ["target_version"].get<uint32_t>();
      }
      config.patches.push_back(patch);
    }
  }

  // 6. payloads
  if (j.contains("payloads") && j["payloads"].is_object()) {
    const auto& pyJ = j["payloads"];
    if (pyJ.contains("xell") && pyJ["xell"].is_object()) {
      if (pyJ["xell"].contains("type") && pyJ["xell"]["type"].is_string()) {
        config.payloads.xell.type = pyJ["xell"]["type"].get<std::string>();
      }
      if (pyJ["xell"].contains("version") && pyJ["xell"]["version"].is_string()) {
        config.payloads.xell.version = pyJ["xell"]["version"].get<std::string>();
      }
    }
    if (pyJ.contains("rebooter") && pyJ["rebooter"].is_object()) {
      PayloadsSection::PayloadEntry entry;
      if (pyJ["rebooter"].contains("type") && pyJ["rebooter"]["type"].is_string()) {
        entry.type = pyJ["rebooter"]["type"].get<std::string>();
      }
      if (pyJ["rebooter"].contains("version") && pyJ["rebooter"]["version"].is_string()) {
        entry.version = pyJ["rebooter"]["version"].get<std::string>();
      }
      config.payloads.rebooter = entry;
    }
    if (pyJ.contains("payload") && pyJ["payload"].is_object()) {
      PayloadsSection::PayloadEntry entry;
      if (pyJ["payload"].contains("type") && pyJ["payload"]["type"].is_string()) {
        entry.type = pyJ["payload"]["type"].get<std::string>();
      }
      if (pyJ["payload"].contains("version") && pyJ["payload"]["version"].is_string()) {
        entry.version = pyJ["payload"]["version"].get<std::string>();
      }
      config.payloads.payload = entry;
    }
    if (pyJ.contains("fuses") && pyJ["fuses"].is_object()) {
      PayloadsSection::PayloadEntry entry;
      if (pyJ["fuses"].contains("type") && pyJ["fuses"]["type"].is_string()) {
        entry.type = pyJ["fuses"]["type"].get<std::string>();
      }
      if (pyJ["fuses"].contains("version") && pyJ["fuses"]["version"].is_string()) {
        entry.version = pyJ["fuses"]["version"].get<std::string>();
      }
      config.payloads.fuses = entry;
    }
  }

  // 7. flashfs
  if (j.contains("flashfs") && j["flashfs"].is_array()) {
    for (const auto& fItem : j["flashfs"]) {
      if (fItem.is_string()) {
        config.flashfsFiles.push_back(fItem.get<std::string>());
      }
    }
  }

  return config;
}

std::expected<BuildConfig, std::string> loadBuildConfigFromFile(const std::filesystem::path& jsonPath) {
  if (!std::filesystem::exists(jsonPath)) {
    return std::unexpected("BuildConfig JSON file does not exist: " + jsonPath.string());
  }

  std::ifstream f(jsonPath);
  if (!f.is_open()) {
    return std::unexpected("Failed to open file: " + jsonPath.string());
  }

  nlohmann::json j;
  try {
    f >> j;
  } catch (const std::exception& e) {
    return std::unexpected("JSON parse error in " + jsonPath.string() + ": " + e.what());
  }

  return parseBuildConfigJson(j);
}

// ---------------------------------------------------------------------------
// xeBuild INI Converter Implementation
// ---------------------------------------------------------------------------

namespace {

static bool SectionMatchesConsole(std::string_view sec, ConsoleTarget target) {
  std::string s = ToLower(sec);
  switch (target) {
  case ConsoleTarget::Xenon:
    return s == "xenonbl" || s == "xenonbl_elpis";
  case ConsoleTarget::Zephyr:
    return s == "zephyrbl";
  case ConsoleTarget::Falcon:
    return s == "falconbl";
  case ConsoleTarget::Jasper:
    return s == "jasperbl";
  case ConsoleTarget::Trinity:
    return s == "trinitybl";
  case ConsoleTarget::Corona:
    return s == "coronabl" || s == "coronabl_wb" || s == "coronabl_wb4g";
  case ConsoleTarget::Winchester:
    return s == "winchesterbl" || s == "coronabl";
  default:
    return false;
  }
}

static BootloaderDescriptor ParseXeBuildFileName(std::string_view rawName) {
  BootloaderDescriptor desc;
  std::string fname(rawName);

  // Strip extension if present (.bin)
  size_t dotPos = fname.rfind('.');
  std::string base = (dotPos != std::string::npos) ? fname.substr(0, dotPos) : fname;
  std::string baseLower = ToLower(base);

  if (baseLower.rfind("cba_", 0) == 0) {
    desc.type = BootloaderType::CB_A;
    desc.version = base.substr(4);
    desc.id = "bl_cba";
  } else if (baseLower.rfind("cbb_", 0) == 0) {
    desc.type = BootloaderType::CB_B;
    desc.version = base.substr(4);
    desc.id = "bl_cbb";
  } else if (baseLower.rfind("cb_", 0) == 0) {
    desc.type = BootloaderType::CB;
    desc.version = base.substr(3);
    desc.id = "bl_cb";
  } else if (baseLower.rfind("cd_", 0) == 0) {
    desc.type = BootloaderType::CD;
    desc.version = base.substr(3);
    desc.id = "bl_cd";
  } else if (baseLower.rfind("ce_", 0) == 0) {
    desc.type = BootloaderType::CE;
    desc.version = base.substr(3);
    desc.id = "bl_ce";
  } else if (baseLower.rfind("cf_", 0) == 0) {
    desc.type = BootloaderType::CF;
    desc.version = base.substr(3);
    desc.id = "bl_cf";
  } else if (baseLower.rfind("cg_", 0) == 0) {
    desc.type = BootloaderType::CG;
    desc.version = base.substr(3);
    desc.id = "bl_cg";
  } else {
    // Unlisted / Generic file fallback (e.g. smc.bin)
    desc.type = BootloaderType::Unknown;
    desc.version = "generic";
    desc.id = "bl_" + baseLower;
  }

  return desc;
}

} // namespace

std::expected<BuildConfig, std::string> parseXeBuildIniString(
    std::string_view iniContent,
    ConsoleTarget consoleTarget,
    std::string_view buildType) {

  BuildConfig config;
  config.meta.name = std::string(ConsoleTargetToString(consoleTarget)) + " " + std::string(buildType);
  config.meta.consoleTarget = consoleTarget;
  config.meta.buildType = std::string(buildType);
  config.meta.imageType = ImageType::Glitch;

  std::istringstream stream((std::string(iniContent)));
  std::string line;
  std::string currentSection;
  bool foundConsoleSection = false;

  while (std::getline(stream, line)) {
    // Trim carriage return and whitespace
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    size_t first = line.find_first_not_of(" \t");
    if (first == std::string::npos) continue;
    line = line.substr(first);
    size_t last = line.find_last_not_of(" \t");
    if (last != std::string::npos) line = line.substr(0, last + 1);

    if (line.empty() || line[0] == ';' || line[0] == '#') continue;

    // Check for section header [section]
    if (line.front() == '[' && line.back() == ']') {
      currentSection = ToLower(line.substr(1, line.length() - 2));
      continue;
    }

    if (currentSection == "version") {
      try {
        config.meta.version = static_cast<uint32_t>(std::stoul(line));
      } catch (...) {}
    } else if (SectionMatchesConsole(currentSection, consoleTarget)) {
      foundConsoleSection = true;

      // Extract section flags (e.g. elpis, wb2k, wb4g)
      if (currentSection.find("elpis") != std::string::npos) {
        if (std::find(config.meta.flags.begin(), config.meta.flags.end(), "elpis") == config.meta.flags.end()) {
          config.meta.flags.push_back("elpis");
        }
      }
      if (currentSection.find("_wb4g") != std::string::npos || currentSection.find("wb4g") != std::string::npos) {
        if (std::find(config.meta.flags.begin(), config.meta.flags.end(), "wb4g") == config.meta.flags.end()) {
          config.meta.flags.push_back("wb4g");
        }
      } else if (currentSection.find("_wb") != std::string::npos || currentSection.find("wb2k") != std::string::npos) {
        if (std::find(config.meta.flags.begin(), config.meta.flags.end(), "wb2k") == config.meta.flags.end()) {
          config.meta.flags.push_back("wb2k");
        }
      }

      // Line format: filename,hash or filename
      size_t commaPos = line.find_first_of(",;");
      std::string fileName = (commaPos != std::string::npos) ? line.substr(0, commaPos) : line;

      // Trim filename
      size_t fLast = fileName.find_last_not_of(" \t");
      if (fLast != std::string::npos) fileName = fileName.substr(0, fLast + 1);

      if (!fileName.empty()) {
        BootloaderDescriptor desc = ParseXeBuildFileName(fileName);
        config.bootloaders.push_back(desc);
        config.imageStructure.push_back(desc.id);
      }
    } else if (currentSection == "flashfs") {
      size_t commaPos = line.find_first_of(",;");
      std::string fileName = (commaPos != std::string::npos) ? line.substr(0, commaPos) : line;

      size_t fLast = fileName.find_last_not_of(" \t");
      if (fLast != std::string::npos) fileName = fileName.substr(0, fLast + 1);

      if (!fileName.empty()) {
        config.flashfsFiles.push_back(fileName);
      }
    }
  }

  // Ensure generic smc.bin fallback is present in bootloaders if not parsed
  bool hasSmc = std::any_of(config.bootloaders.begin(), config.bootloaders.end(), [](const BootloaderDescriptor& b) {
    return b.id == "bl_smc" || b.id == "smc";
  });

  if (!hasSmc) {
    config.smc.firmwareType = "clean";
    config.smc.firmwareVersion = "1.0";
    config.smc.configType = "clean";
    config.smc.configVersion = "1.0";
  }

  return config;
}

std::expected<BuildConfig, std::string> parseXeBuildIni(
    const std::filesystem::path& iniPath,
    ConsoleTarget consoleTarget,
    std::string_view buildType) {

  if (!std::filesystem::exists(iniPath)) {
    return std::unexpected("xeBuild INI file does not exist: " + iniPath.string());
  }

  std::ifstream f(iniPath, std::ios::binary);
  if (!f.is_open()) {
    return std::unexpected("Failed to open file: " + iniPath.string());
  }

  std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  return parseXeBuildIniString(content, consoleTarget, buildType);
}

// ---------------------------------------------------------------------------
// Genexis BuildConfig -> xeBuild INI Writer Implementation
// ---------------------------------------------------------------------------

std::string generateXeBuildIniString(const BuildConfig& config) {
  std::ostringstream ss;

  // 1. [version]
  ss << "[version]\n" << config.meta.version << "\n\n";

  // 2. Determine section name from console_target + flags
  std::string baseConsole = ToLower(ConsoleTargetToString(config.meta.consoleTarget));
  std::string sectionName = baseConsole + "bl";

  bool hasElpis = std::find(config.meta.flags.begin(), config.meta.flags.end(), "elpis") != config.meta.flags.end();
  bool hasWb4g = std::find(config.meta.flags.begin(), config.meta.flags.end(), "wb4g") != config.meta.flags.end();
  bool hasWb2k = std::find(config.meta.flags.begin(), config.meta.flags.end(), "wb2k") != config.meta.flags.end();

  if (baseConsole == "xenon" && hasElpis) {
    sectionName = "xenonbl_ELPIS";
  } else if (baseConsole == "corona") {
    if (hasWb4g) {
      sectionName = "coronabl_WB4G";
    } else if (hasWb2k) {
      sectionName = "coronabl_WB";
    }
  }

  ss << "[" << sectionName << "]\n";

  for (const auto& bl : config.bootloaders) {
    std::string fileName;
    switch (bl.type) {
    case BootloaderType::CB_A:
      fileName = "cba_" + bl.version + ".bin";
      break;
    case BootloaderType::CB_B:
      fileName = "cbb_" + bl.version + ".bin";
      break;
    case BootloaderType::CB:
      fileName = "cb_" + bl.version + ".bin";
      break;
    case BootloaderType::CD:
      fileName = "cd_" + bl.version + ".bin";
      break;
    case BootloaderType::CE:
      fileName = "ce_" + bl.version + ".bin";
      break;
    case BootloaderType::CF:
      fileName = "cf_" + bl.version + ".bin";
      break;
    case BootloaderType::CG:
      fileName = "cg_" + bl.version + ".bin";
      break;
    default:
      fileName = bl.id + ".bin";
      break;
    }
    ss << fileName << ",\n";
  }
  ss << "\n";

  // 3. [security] block
  ss << "[security]\ncrl.bin,\ndae.bin,\nextended.bin,\nfcrt.bin,\nsecdata.bin,\n\n";

  // 4. [flashfs] block
  if (!config.flashfsFiles.empty()) {
    ss << "[flashfs]\n";
    for (const auto& ffile : config.flashfsFiles) {
      ss << ffile << ",\n";
    }
    ss << "\n";
  }

  return ss.str();
}

std::expected<void, std::string> writeXeBuildIniFile(
    const BuildConfig& config,
    const std::filesystem::path& outputPath) {

  std::ofstream f(outputPath, std::ios::binary | std::ios::trunc);
  if (!f.is_open()) {
    return std::unexpected("Failed to open output file for writing: " + outputPath.string());
  }

  std::string content = generateXeBuildIniString(config);
  f.write(content.data(), static_cast<std::streamsize>(content.size()));
  if (!f.good()) {
    return std::unexpected("Error writing content to xeBuild INI file: " + outputPath.string());
  }

  return {};
}

// ---------------------------------------------------------------------------
// RGBuild INI Converter Implementation
// ---------------------------------------------------------------------------

namespace {

static std::string GetRGBuildSectionName(ConsoleTarget target, bool isRgh2 = true) {
  std::string consoleStr = ConsoleTargetToString(target);
  if (consoleStr == "Unknown") consoleStr = "Trinity";
  return consoleStr + (isRgh2 ? "RGH2" : "RGH");
}

} // namespace

std::expected<BuildConfig, std::string> parseRGBuildIniString(
    std::string_view buildIniContent,
    std::string_view bootloadersIniContent,
    ConsoleTarget consoleTarget,
    std::string_view buildType) {

  BuildConfig config;
  config.meta.name = std::string(ConsoleTargetToString(consoleTarget)) + " " + std::string(buildType);
  config.meta.consoleTarget = consoleTarget;
  config.meta.buildType = std::string(buildType);
  config.meta.imageType = ImageType::RGLoader;

  // 1. Parse bootloaders.ini if content provided
  if (!bootloadersIniContent.empty()) {
    std::istringstream stream((std::string(bootloadersIniContent)));
    std::string line;
    std::string currentSection;

    std::string targetSectionRgh2 = ToLower(GetRGBuildSectionName(consoleTarget, true));
    std::string targetSectionRgh1 = ToLower(GetRGBuildSectionName(consoleTarget, false));

    while (std::getline(stream, line)) {
      line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
      size_t first = line.find_first_not_of(" \t");
      if (first == std::string::npos) continue;
      line = line.substr(first);
      if (line.empty() || line[0] == ';' || line[0] == '#') continue;

      if (line.front() == '[' && line.back() == ']') {
        currentSection = ToLower(line.substr(1, line.length() - 2));
        continue;
      }

      if (currentSection == targetSectionRgh2 || currentSection == targetSectionRgh1) {
        size_t eqPos = line.find('=');
        if (eqPos != std::string::npos) {
          std::string key = ToLower(line.substr(0, eqPos));
          std::string val = line.substr(eqPos + 1);
          // Trim val
          size_t valStart = val.find_first_not_of(" \t");
          if (valStart != std::string::npos) val = val.substr(valStart);

          if (key == "2bl") {
            // Split by ':' e.g. CB_A.9188.bin:CB_B.9188.bin
            std::istringstream blStream(val);
            std::string blItem;
            while (std::getline(blStream, blItem, ':')) {
              if (blItem.empty()) continue;
              BootloaderDescriptor desc;
              std::string itemLower = ToLower(blItem);
              if (itemLower.find("cb_a") != std::string::npos) {
                desc.type = BootloaderType::CB_A;
                desc.id = "bl_cba";
              } else if (itemLower.find("cb_b") != std::string::npos) {
                desc.type = BootloaderType::CB_B;
                desc.id = "bl_cbb";
              } else {
                desc.type = BootloaderType::CB;
                desc.id = "bl_cb";
              }
              // Extract version from CB_A.9188.bin
              size_t p1 = blItem.find('.');
              size_t p2 = blItem.rfind('.');
              if (p1 != std::string::npos && p1 != p2) {
                desc.version = blItem.substr(p1 + 1, p2 - p1 - 1);
              } else {
                desc.version = "9188";
              }
              config.bootloaders.push_back(desc);
              config.imageStructure.push_back(desc.id);
            }
          } else if (key == "4bl") {
            BootloaderDescriptor desc;
            desc.type = BootloaderType::CD;
            desc.id = "bl_cd";
            size_t p1 = val.find('.');
            size_t p2 = val.rfind('.');
            if (p1 != std::string::npos && p1 != p2) {
              desc.version = val.substr(p1 + 1, p2 - p1 - 1);
            } else {
              desc.version = "9452";
            }
            config.bootloaders.push_back(desc);
            config.imageStructure.push_back(desc.id);
          }
        }
      }
    }
  }

  // 2. Parse main build INI (e.g. slim.ini, fatRGH2.ini)
  std::istringstream stream((std::string(buildIniContent)));
  std::string line;
  std::string currentSection;

  while (std::getline(stream, line)) {
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    size_t first = line.find_first_not_of(" \t");
    if (first == std::string::npos) continue;
    line = line.substr(first);
    if (line.empty() || line[0] == ';' || line[0] == '#') continue;

    if (line.front() == '[' && line.back() == ']') {
      currentSection = ToLower(line.substr(1, line.length() - 2));
      continue;
    }

    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) continue;

    std::string key = ToLower(line.substr(0, eqPos));
    std::string val = line.substr(eqPos + 1);
    size_t vStart = val.find_first_not_of(" \t");
    if (vStart != std::string::npos) val = val.substr(vStart);
    size_t vEnd = val.find_last_not_of(" \t");
    if (vEnd != std::string::npos) val = val.substr(0, vEnd + 1);

    if (currentSection == "bootloaders") {
      if (key == "5bl") {
        // e.g. SE.17489.bin
        BootloaderDescriptor desc;
        desc.type = BootloaderType::CE;
        desc.id = "bl_se";
        size_t p1 = val.find('.');
        size_t p2 = val.rfind('.');
        if (p1 != std::string::npos && p1 != p2) {
          desc.version = val.substr(p1 + 1, p2 - p1 - 1);
          try {
            config.meta.version = static_cast<uint32_t>(std::stoul(desc.version));
          } catch (...) {}
        } else {
          desc.version = "17489";
        }
        config.bootloaders.push_back(desc);
        config.imageStructure.push_back(desc.id);
      }
    } else if (currentSection == "files") {
      std::string valLower = ToLower(val);

      if (valLower.find(".rglp") != std::string::npos) {
        PatchDescriptor p;
        p.name = val;
        p.type = PatchType::PatchSet;
        p.target = PatchTargetType::FullImage;
        p.targetVersion = config.meta.version;
        config.patches.push_back(p);
      } else if (valLower.find("xell") != std::string::npos) {
        config.payloads.xell.type = "xell_reloaded";
        config.payloads.xell.version = val;
      } else if (valLower != "crl.bin" && valLower != "dae.bin" &&
                 valLower != "secdata.bin" && valLower != "extended.bin" &&
                 valLower != "fcrt.bin" && valLower != "none") {
        config.flashfsFiles.push_back(val);
      }
    }
  }

  return config;
}

std::expected<BuildConfig, std::string> parseRGBuildIni(
    const std::filesystem::path& buildIniPath,
    const std::filesystem::path& bootloadersIniPath,
    ConsoleTarget consoleTarget,
    std::string_view buildType) {

  if (!std::filesystem::exists(buildIniPath)) {
    return std::unexpected("RGBuild build INI file does not exist: " + buildIniPath.string());
  }

  std::ifstream fBuild(buildIniPath, std::ios::binary);
  if (!fBuild.is_open()) {
    return std::unexpected("Failed to open build INI file: " + buildIniPath.string());
  }
  std::string buildContent((std::istreambuf_iterator<char>(fBuild)), std::istreambuf_iterator<char>());

  std::string bootloaderContent;
  if (std::filesystem::exists(bootloadersIniPath)) {
    std::ifstream fBL(bootloadersIniPath, std::ios::binary);
    if (fBL.is_open()) {
      bootloaderContent.assign((std::istreambuf_iterator<char>(fBL)), std::istreambuf_iterator<char>());
    }
  }

  return parseRGBuildIniString(buildContent, bootloaderContent, consoleTarget, buildType);
}

std::string generateRGBuildIniString(const BuildConfig& config) {
  std::ostringstream ss;

  ss << "[Image]\n"
     << "6BLAddress = 0xF0000\n"
     << "exploit = RGH2\n\n";

  ss << "[ConsoleSpecific]\n"
     << "ImageInfoIni = imageinfo.ini\n\n";

  ss << "[Bootloaders]\n";
  std::string seVer = std::to_string(config.meta.version);
  for (const auto& bl : config.bootloaders) {
    if (bl.type == BootloaderType::CE || bl.id == "bl_se") {
      seVer = bl.version;
      break;
    }
  }
  ss << "5BL = SE." << seVer << ".bin\n"
     << "6BL = None\n"
     << "7BL = None\n\n";

  ss << "[Files]\n";
  int fileIdx = 1;
  // Security defaults
  ss << fileIdx++ << " = crl.bin\n";
  ss << fileIdx++ << " = dae.bin\n";
  ss << fileIdx++ << " = secdata.bin\n";
  ss << fileIdx++ << " = extended.bin\n";
  ss << fileIdx++ << " = fcrt.bin\n";

  for (const auto& patch : config.patches) {
    if (patch.type == PatchType::PatchSet) {
      ss << fileIdx++ << " = " << patch.name << "\n";
    }
  }

  if (!config.payloads.xell.version.empty()) {
    ss << fileIdx++ << " = " << config.payloads.xell.version << "\n";
  }

  for (const auto& ffile : config.flashfsFiles) {
    ss << fileIdx++ << " = " << ffile << "\n";
  }

  return ss.str();
}

std::string generateRGBuildBootloadersIniString(const BuildConfig& config) {
  std::ostringstream ss;
  std::string sectionName = GetRGBuildSectionName(config.meta.consoleTarget, true);

  ss << "[" << sectionName << "]\n";

  std::string cbaVer = "9188";
  std::string cbbVer = "9188";
  std::string cdVer = "9452";

  for (const auto& bl : config.bootloaders) {
    if (bl.type == BootloaderType::CB_A) cbaVer = bl.version;
    if (bl.type == BootloaderType::CB_B) cbbVer = bl.version;
    if (bl.type == BootloaderType::CD) cdVer = bl.version;
  }

  ss << "2BL = CB_A." << cbaVer << ".bin:CB_B." << cbbVer << ".bin\n";
  ss << "4BL = CD." << cdVer << ".bin\n";

  return ss.str();
}

std::expected<void, std::string> writeRGBuildIniFiles(
    const BuildConfig& config,
    const std::filesystem::path& outputDir) {

  std::error_code ec;
  std::filesystem::create_directories(outputDir, ec);

  std::filesystem::path buildPath = outputDir / "rgloader.ini";
  std::ofstream fBuild(buildPath, std::ios::binary | std::ios::trunc);
  if (!fBuild.is_open()) {
    return std::unexpected("Failed to open output file: " + buildPath.string());
  }

  std::string buildContent = generateRGBuildIniString(config);
  fBuild.write(buildContent.data(), static_cast<std::streamsize>(buildContent.size()));

  std::filesystem::path blPath = outputDir / "bootloaders.ini";
  std::ofstream fBL(blPath, std::ios::binary | std::ios::trunc);
  if (!fBL.is_open()) {
    return std::unexpected("Failed to open output file: " + blPath.string());
  }

  std::string blContent = generateRGBuildBootloadersIniString(config);
  fBL.write(blContent.data(), static_cast<std::streamsize>(blContent.size()));

  return {};
}

} // namespace gxapi::Metabuild
