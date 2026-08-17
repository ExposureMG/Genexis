#include "backend/adapters/NandProMaxAdapter.hpp"
#include <nandpromax.h>

#include <algorithm>
#include <sstream>

namespace gxapi::backend {

namespace {

struct ExecutionContext {
  FlashProgressCallback flashCb{nullptr};
  JtagProgressCallback jtagCb{nullptr};
  std::string collectedLogs;
};

static std::string formatErrorMessage(const std::string &action, int rc,
                                      const ExecutionContext &ctx) {
  std::string specificErrorLine;
  std::istringstream iss(ctx.collectedLogs);
  std::string line;
  while (std::getline(iss, line)) {
    std::string lower = line;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
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

  if (!ctx.collectedLogs.empty() &&
      ctx.collectedLogs != specificErrorLine + "\n") {
    errReason += "\nDiagnostic log:\n" + ctx.collectedLogs;
  }
  return errReason;
}

} 

std::expected<FlashInfo, std::string>
NandProMaxAdapter::getFlashInfo(const FlashDeviceConfig &config) {
  ExecutionContext ctx;
  ProgressC progress{.log_fn =
                         [](const char *msg, void *user_data) {
                           if (msg && user_data) {
                             auto *ctxPtr =
                                 static_cast<ExecutionContext *>(user_data);
                             std::string str(msg);
                             if (!str.empty() && str.back() == '\n')
                               str.pop_back();
                             ctxPtr->collectedLogs += str + "\n";
                           }
                         },
                     .update_fn = nullptr,
                     .user_data = &ctx};

  int rc = nandpromax_cmd_info(
      NANDPRO_DEV_AUTO,
      config.serialNumber.empty() ? nullptr : config.serialNumber.c_str(),
      config.ipAddress.empty() ? nullptr : config.ipAddress.c_str(),
      config.timeoutMs, &progress);

  if (rc != 0) {
    return std::unexpected(formatErrorMessage("getFlashInfo", rc, ctx));
  }

  FlashInfo info;
  info.hardwareName = "NandProMax Hardware (PicoFlasher / LPC / DemoN)";
  info.flashType = "Xbox 360 SFC NAND";
  return info;
}

std::expected<void, std::string>
NandProMaxAdapter::readNand(const std::filesystem::path &outputPath,
                            uint32_t startBlock, uint32_t blockCount,
                            const FlashDeviceConfig &config,
                            FlashProgressCallback progressCb) {
  ExecutionContext ctx{.flashCb = progressCb};
  ProgressC progress{
      .log_fn =
          [](const char *msg, void *user_data) {
            if (msg && user_data) {
              auto *ctxPtr = static_cast<ExecutionContext *>(user_data);
              std::string str(msg);
              if (!str.empty() && str.back() == '\n')
                str.pop_back();
              ctxPtr->collectedLogs += str + "\n";
              if (ctxPtr->flashCb) {
                FlashProgressInfo pInfo{.bytesDone = 0,
                                        .totalBytes = 0,
                                        .percentage = 0.0f,
                                        .statusMessage = str};
                ctxPtr->flashCb(pInfo);
              }
            }
          },
      .update_fn =
          [](uint64_t done, uint64_t total, void *user_data) {
            if (user_data) {
              auto *ctxPtr = static_cast<ExecutionContext *>(user_data);
              if (ctxPtr && ctxPtr->flashCb) {
                float pct = (total > 0)
                                ? (static_cast<float>(done) / total * 100.0f)
                                : 0.0f;
                FlashProgressInfo pInfo{.bytesDone = done,
                                        .totalBytes = total,
                                        .percentage = pct,
                                        .statusMessage = ""};
                ctxPtr->flashCb(pInfo);
              }
            }
          },
      .user_data = &ctx};

  bool countHasVal = (blockCount > 0);
  int rc = nandpromax_cmd_read_nand(
      outputPath.string().c_str(), NANDPRO_DEV_AUTO, NANDPRO_MEDIA_AUTO,
      startBlock, blockCount, countHasVal,
      config.serialNumber.empty() ? nullptr : config.serialNumber.c_str(),
      config.ipAddress.empty() ? nullptr : config.ipAddress.c_str(),
      config.timeoutMs, &progress);

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

  ExecutionContext ctx{.flashCb = progressCb};
  ProgressC progress{
      .log_fn =
          [](const char *msg, void *user_data) {
            if (msg && user_data) {
              auto *ctxPtr = static_cast<ExecutionContext *>(user_data);
              std::string str(msg);
              if (!str.empty() && str.back() == '\n')
                str.pop_back();
              ctxPtr->collectedLogs += str + "\n";
              if (ctxPtr->flashCb) {
                FlashProgressInfo pInfo{.bytesDone = 0,
                                        .totalBytes = 0,
                                        .percentage = 0.0f,
                                        .statusMessage = str};
                ctxPtr->flashCb(pInfo);
              }
            }
          },
      .update_fn =
          [](uint64_t done, uint64_t total, void *user_data) {
            if (user_data) {
              auto *ctxPtr = static_cast<ExecutionContext *>(user_data);
              if (ctxPtr && ctxPtr->flashCb) {
                float pct = (total > 0)
                                ? (static_cast<float>(done) / total * 100.0f)
                                : 0.0f;
                FlashProgressInfo pInfo{.bytesDone = done,
                                        .totalBytes = total,
                                        .percentage = pct,
                                        .statusMessage = ""};
                ctxPtr->flashCb(pInfo);
              }
            }
          },
      .user_data = &ctx};

  int rc = nandpromax_cmd_write_nand(
      inputPath.string().c_str(), NANDPRO_DEV_AUTO, NANDPRO_MEDIA_AUTO,
      startBlock, 0, false, eraseFirst, verifyAfter,
      config.serialNumber.empty() ? nullptr : config.serialNumber.c_str(),
      config.ipAddress.empty() ? nullptr : config.ipAddress.c_str(),
      config.timeoutMs, &progress);

  if (rc != 0) {
    return std::unexpected(formatErrorMessage("writeNand", rc, ctx));
  }
  return {};
}

std::expected<void, std::string>
NandProMaxAdapter::eraseNand(uint32_t startBlock, uint32_t blockCount,
                             const FlashDeviceConfig &config) {
  ExecutionContext ctx;
  ProgressC progress{.log_fn =
                         [](const char *msg, void *user_data) {
                           if (msg && user_data) {
                             auto *ctxPtr =
                                 static_cast<ExecutionContext *>(user_data);
                             std::string str(msg);
                             if (!str.empty() && str.back() == '\n')
                               str.pop_back();
                             ctxPtr->collectedLogs += str + "\n";
                           }
                         },
                     .update_fn = nullptr,
                     .user_data = &ctx};

  int rc = nandpromax_cmd_write_nand(
      "/dev/null", NANDPRO_DEV_AUTO, NANDPRO_MEDIA_AUTO, startBlock, blockCount,
      true,
      true,  
      false, 
      config.serialNumber.empty() ? nullptr : config.serialNumber.c_str(),
      config.ipAddress.empty() ? nullptr : config.ipAddress.c_str(),
      config.timeoutMs, &progress);

  if (rc != 0) {
    return std::unexpected(formatErrorMessage("eraseNand", rc, ctx));
  }
  return {};
}

std::expected<std::vector<uint32_t>, std::string>
NandProMaxAdapter::scanChain(const JtagDeviceConfig &config) {
  (void)config;
  ExecutionContext ctx;
  ProgressC progress{.log_fn =
                         [](const char *msg, void *user_data) {
                           if (msg && user_data) {
                             auto *ctxPtr =
                                 static_cast<ExecutionContext *>(user_data);
                             std::string str(msg);
                             if (!str.empty() && str.back() == '\n')
                               str.pop_back();
                             ctxPtr->collectedLogs += str + "\n";
                           }
                         },
                     .update_fn = nullptr,
                     .user_data = &ctx};

  int rc = nandpromax_cmd_xsvf_detect(NANDPRO_DEV_AUTO, &progress);
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

  ExecutionContext ctx{.jtagCb = progressCb};
  ProgressC progress{
      .log_fn =
          [](const char *msg, void *user_data) {
            if (msg && user_data) {
              auto *ctxPtr = static_cast<ExecutionContext *>(user_data);
              std::string str(msg);
              if (!str.empty() && str.back() == '\n')
                str.pop_back();
              ctxPtr->collectedLogs += str + "\n";
              if (ctxPtr->jtagCb) {
                JtagProgressInfo pInfo{.bytesDone = 0,
                                       .totalBytes = 0,
                                       .percentage = 0.0f,
                                       .statusMessage = str};
                ctxPtr->jtagCb(pInfo);
              }
            }
          },
      .update_fn =
          [](uint64_t done, uint64_t total, void *user_data) {
            if (user_data) {
              auto *ctxPtr = static_cast<ExecutionContext *>(user_data);
              if (ctxPtr && ctxPtr->jtagCb) {
                float pct = (total > 0)
                                ? (static_cast<float>(done) / total * 100.0f)
                                : 0.0f;
                JtagProgressInfo pInfo{.bytesDone = done,
                                       .totalBytes = total,
                                       .percentage = pct,
                                       .statusMessage = ""};
                ctxPtr->jtagCb(pInfo);
              }
            }
          },
      .user_data = &ctx};

  int rc = nandpromax_cmd_xsvf_write(bitstreamPath.string().c_str(),
                                     NANDPRO_DEV_AUTO, &progress);
  if (rc != 0) {
    return std::unexpected(formatErrorMessage("flashCpld", rc, ctx));
  }
  return {};
}

} 
