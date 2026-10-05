#include "backend/adapters/NandProMaxAdapter.hpp"
#include <nandpromax.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <sstream>
#include <vector>

namespace gxapi::backend {

namespace {

// libnandpromax drives one USB/serial device at a time.
std::mutex g_nandProMaxMutex;

using EmitFn = std::function<void(uint64_t done, uint64_t total, float pct,
                                  const std::string &msg)>;

struct ExecutionContext {
  EmitFn emit;
  std::string collectedLogs;
  // Last known state, so log lines don't reset the progress bar to 0% and
  // progress updates keep showing the latest status message.
  uint64_t lastDone{0};
  uint64_t lastTotal{0};
  float lastPct{0.0f};
  std::string lastMsg;
};

void onLog(const char *msg, void *userData) {
  if (!msg || !userData)
    return;
  auto *ctx = static_cast<ExecutionContext *>(userData);
  std::string str(msg);
  if (!str.empty() && str.back() == '\n')
    str.pop_back();
  ctx->collectedLogs += str + "\n";
  ctx->lastMsg = str;
  if (ctx->emit) {
    try {
      ctx->emit(ctx->lastDone, ctx->lastTotal, ctx->lastPct, str);
    } catch (...) { // must not unwind into Rust
    }
  }
}

void onUpdate(uint64_t done, uint64_t total, void *userData) {
  if (!userData)
    return;
  auto *ctx = static_cast<ExecutionContext *>(userData);
  ctx->lastDone = done;
  ctx->lastTotal = total;
  ctx->lastPct = total > 0 ? static_cast<float>(done) /
                                 static_cast<float>(total) * 100.0f
                           : 0.0f;
  if (ctx->emit) {
    try {
      ctx->emit(done, total, ctx->lastPct, ctx->lastMsg);
    } catch (...) {
    }
  }
}

ProgressC makeProgress(ExecutionContext &ctx) {
  return ProgressC{.log_fn = onLog, .update_fn = onUpdate, .user_data = &ctx};
}

ExecutionContext flashContext(const FlashProgressCallback &cb) {
  ExecutionContext ctx;
  if (cb) {
    ctx.emit = [cb](uint64_t done, uint64_t total, float pct,
                    const std::string &msg) {
      cb(FlashProgressInfo{.bytesDone = done,
                           .totalBytes = total,
                           .percentage = pct,
                           .statusMessage = msg});
    };
  }
  return ctx;
}

ExecutionContext jtagContext(const JtagProgressCallback &cb) {
  ExecutionContext ctx;
  if (cb) {
    ctx.emit = [cb](uint64_t done, uint64_t total, float pct,
                    const std::string &msg) {
      cb(JtagProgressInfo{.bytesDone = done,
                          .totalBytes = total,
                          .percentage = pct,
                          .statusMessage = msg});
    };
  }
  return ctx;
}

NandProDeviceC toDevice(FlashHardwareType hw) {
  switch (hw) {
  case FlashHardwareType::PicoFlasher:
    return NANDPRO_DEV_PICOFLASHER;
  case FlashHardwareType::NandX:
  case FlashHardwareType::Lpc:
    return NANDPRO_DEV_LPC;
  case FlashHardwareType::DemoN:
    return NANDPRO_DEV_DEMON;
  default:
    return NANDPRO_DEV_AUTO;
  }
}

NandProMediaC toMedia(FlashMediaType media) {
  switch (media) {
  case FlashMediaType::Spi:
    return NANDPRO_MEDIA_SPI;
  case FlashMediaType::Emmc:
    return NANDPRO_MEDIA_EMMC;
  default:
    return NANDPRO_MEDIA_AUTO;
  }
}

const char *serialOrNull(const FlashDeviceConfig &config) {
  return config.serialNumber.empty() ? nullptr : config.serialNumber.c_str();
}

const char *addrOrNull(const FlashDeviceConfig &config) {
  return config.ipAddress.empty() ? nullptr : config.ipAddress.c_str();
}

// The library maps every failure to -1/-2 and only reports detail through the
// log callback, so the best available reason is a log line.
std::string formatErrorMessage(const std::string &action, int rc,
                               const ExecutionContext &ctx) {
  std::vector<std::string> lines;
  {
    std::istringstream iss(ctx.collectedLogs);
    std::string line;
    while (std::getline(iss, line))
      lines.push_back(line);
  }

  std::string specificErrorLine;
  for (const auto &line : lines) {
    std::string lower = line;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (lower.find("error") != std::string::npos ||
        lower.find("fail") != std::string::npos ||
        lower.find("denied") != std::string::npos ||
        lower.find("not found") != std::string::npos ||
        lower.find("cannot") != std::string::npos ||
        lower.find("panic") != std::string::npos) {
      specificErrorLine = line;
      break;
    }
  }

  std::string errReason;
  if (!specificErrorLine.empty()) {
    errReason = specificErrorLine;
  } else if (rc == -1) {
    errReason = action + " failed: Invalid arguments or parameters";
  } else if (rc == -2) {
    errReason = action + " failed (hardware execution error: flasher device "
                         "not connected or access denied)";
  } else {
    errReason = action + " failed with code " + std::to_string(rc);
  }

  // Progress chatter can run to hundreds of lines; keep the tail only.
  constexpr size_t kMaxDiagLines = 8;
  if (!lines.empty()) {
    std::string diag;
    const size_t first =
        lines.size() > kMaxDiagLines ? lines.size() - kMaxDiagLines : 0;
    for (size_t i = first; i < lines.size(); ++i)
      diag += lines[i] + "\n";
    if (diag != specificErrorLine + "\n")
      errReason += "\nDiagnostic log:\n" + diag;
  }
  return errReason;
}

// Value following `key` in the collected log, parsed as decimal or 0x-hex.
bool findLogNumber(const std::string &logs, const std::string &key,
                   uint64_t &out) {
  const auto pos = logs.find(key);
  if (pos == std::string::npos)
    return false;
  const char *start = logs.c_str() + pos + key.size();
  char *end = nullptr;
  const unsigned long long v = std::strtoull(start, &end, 0);
  if (end == start)
    return false;
  out = v;
  return true;
}

std::string extension(const std::filesystem::path &path) {
  std::string ext = path.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return ext;
}

} // namespace

std::expected<FlashInfo, std::string>
NandProMaxAdapter::getFlashInfo(const FlashDeviceConfig &config) {
  std::lock_guard<std::mutex> lock(g_nandProMaxMutex);

  // nandpromax_cmd_info treats DEV_AUTO as PicoFlasher only, so with no
  // explicit hardware try each backend in turn.
  std::vector<NandProDeviceC> candidates;
  if (const auto dev = toDevice(config.hardware); dev != NANDPRO_DEV_AUTO)
    candidates = {dev};
  else
    candidates = {NANDPRO_DEV_PICOFLASHER, NANDPRO_DEV_LPC, NANDPRO_DEV_DEMON};

  std::string firstError;
  for (const auto dev : candidates) {
    ExecutionContext ctx;
    ProgressC progress = makeProgress(ctx);
    const int rc = nandpromax_cmd_info(dev, serialOrNull(config),
                                       addrOrNull(config), config.timeoutMs,
                                       &progress);
    if (rc != 0) {
      if (firstError.empty())
        firstError = formatErrorMessage("getFlashInfo", rc, ctx);
      continue;
    }

    FlashInfo info;
    info.flashType = "Xbox 360 SFC NAND";
    switch (dev) {
    case NANDPRO_DEV_LPC:
      info.hardwareName = "LPC / XFlash (NandProMax)";
      break;
    case NANDPRO_DEV_DEMON:
      info.hardwareName = "DemoN (NandProMax)";
      break;
    default:
      info.hardwareName = "PicoFlasher (NandProMax)";
      break;
    }

    uint64_t value = 0;
    if (findLogNumber(ctx.collectedLogs, "Flash Config: ", value))
      info.configWord = static_cast<uint32_t>(value);
    if (findLogNumber(ctx.collectedLogs, "File Size: ", value))
      info.totalBytes = value;
    return info;
  }

  return std::unexpected(firstError);
}

// startBlock / blockCount are interpreted by the library per device:
// 0x210-byte pages for PicoFlasher, flash blocks for LPC and DemoN. A
// blockCount of 0 reads to the end of the flash.
std::expected<void, std::string>
NandProMaxAdapter::readNand(const std::filesystem::path &outputPath,
                            uint32_t startBlock, uint32_t blockCount,
                            const FlashDeviceConfig &config,
                            FlashProgressCallback progressCb) {
  std::lock_guard<std::mutex> lock(g_nandProMaxMutex);

  ExecutionContext ctx = flashContext(progressCb);
  ProgressC progress = makeProgress(ctx);

  const bool countHasVal = (blockCount > 0);
  const int rc = nandpromax_cmd_read_nand(
      outputPath.string().c_str(), toDevice(config.hardware),
      toMedia(config.media), startBlock, blockCount, countHasVal,
      serialOrNull(config), addrOrNull(config), config.timeoutMs, &progress);

  if (rc != 0) {
    return std::unexpected(formatErrorMessage("readNand", rc, ctx));
  }
  return {};
}

std::expected<void, std::string>
NandProMaxAdapter::writeNand(const std::filesystem::path &inputPath,
                             uint32_t startBlock, bool eraseFirst,
                             bool verifyAfter, const FlashDeviceConfig &config,
                             FlashProgressCallback progressCb) {
  if (!std::filesystem::exists(inputPath)) {
    return std::unexpected("Input file does not exist: " + inputPath.string());
  }

  std::lock_guard<std::mutex> lock(g_nandProMaxMutex);

  ExecutionContext ctx = flashContext(progressCb);
  ProgressC progress = makeProgress(ctx);

  // libnandpromax ignores the erase and verify flags (cmd_write_nand takes
  // them as unused parameters): the device erases as part of programming and
  // no readback comparison is performed. Say so rather than imply a verify.
  if (verifyAfter && progressCb) {
    progressCb(FlashProgressInfo{
        .statusMessage =
            "Note: NandProMax does not verify after write; skipping verify"});
  }

  const int rc = nandpromax_cmd_write_nand(
      inputPath.string().c_str(), toDevice(config.hardware),
      toMedia(config.media), startBlock, 0, false, eraseFirst, verifyAfter,
      serialOrNull(config), addrOrNull(config), config.timeoutMs, &progress);

  if (rc != 0) {
    return std::unexpected(formatErrorMessage("writeNand", rc, ctx));
  }
  return {};
}

std::expected<void, std::string>
NandProMaxAdapter::eraseNand(uint32_t startBlock, uint32_t blockCount,
                             const FlashDeviceConfig &config) {
  (void)startBlock;
  (void)blockCount;
  (void)config;
  // The library has no erase command. The previous implementation "wrote"
  // /dev/null with erase=true, which the library treats as an empty write: it
  // returned success without touching the flash (and fails outright on
  // Windows, which has no /dev/null).
  return std::unexpected(
      "Erase is not supported by the NandProMax backend; writing an image "
      "erases the blocks it programs");
}

// Only reports whether an LPC/XFlash programmer is present: the library does
// not expose the JTAG chain IDCODEs, so the returned list is empty.
std::expected<std::vector<uint32_t>, std::string>
NandProMaxAdapter::scanChain(const JtagDeviceConfig &config) {
  (void)config;
  std::lock_guard<std::mutex> lock(g_nandProMaxMutex);

  ExecutionContext ctx;
  ProgressC progress = makeProgress(ctx);

  const int rc = nandpromax_cmd_xsvf_detect(NANDPRO_DEV_AUTO, &progress);
  if (rc != 0) {
    return std::unexpected(formatErrorMessage("scanChain", rc, ctx));
  }
  return std::vector<uint32_t>{};
}

std::expected<void, std::string>
NandProMaxAdapter::flashCpld(const std::filesystem::path &bitstreamPath,
                             const JtagDeviceConfig &config,
                             JtagProgressCallback progressCb) {
  (void)config;
  if (!std::filesystem::exists(bitstreamPath)) {
    return std::unexpected("Bitstream file does not exist: " +
                           bitstreamPath.string());
  }
  // The library streams the file's bytes to the device as XSVF without
  // parsing it, so a text SVF file would be programmed as garbage.
  if (extension(bitstreamPath) == ".svf") {
    return std::unexpected(
        "NandProMax can only program .xsvf files; convert the .svf first");
  }

  std::lock_guard<std::mutex> lock(g_nandProMaxMutex);

  ExecutionContext ctx = jtagContext(progressCb);
  ProgressC progress = makeProgress(ctx);

  const int rc = nandpromax_cmd_xsvf_write(bitstreamPath.string().c_str(),
                                           NANDPRO_DEV_AUTO, &progress);
  if (rc != 0) {
    return std::unexpected(formatErrorMessage("flashCpld", rc, ctx));
  }
  return {};
}

} // namespace gxapi::backend
