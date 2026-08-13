#include "backend/adapters/UpdClientAdapter.hpp"

#include <fstream>
#include <updserver/updserver_client.hpp>
#include <updserver/updserver_discovery.hpp>

namespace gxapi::backend {

UpdClientAdapter::UpdClientAdapter()
    : m_client(std::make_unique<updclient::updserver::UpdServerClient>()) {}

UpdClientAdapter::~UpdClientAdapter() = default;

std::vector<DiscoveredConsoleInfo> UpdClientAdapter::discoverConsoles() {
  std::vector<DiscoveredConsoleInfo> results;
  auto discovered = updclient::updserver::UpdServerDiscovery::discoverAll();
  for (const auto &dev : discovered) {
    DiscoveredConsoleInfo info;
    info.ipAddress = dev.ipAddress;
    info.port = 730;
    info.consoleType = "Xbox 360 (UpdServer)";
    results.push_back(info);
  }
  return results;
}

bool UpdClientAdapter::connect(const std::string &ipAddress, uint16_t port) {
  if (!m_client) {
    m_client = std::make_unique<updclient::updserver::UpdServerClient>();
  }
  return m_client->connect(ipAddress, port);
}

void UpdClientAdapter::disconnect() {
  if (m_client) {
    m_client->disconnect();
  }
}

bool UpdClientAdapter::isConnected() const {
  return m_client && m_client->isConnected();
}

std::string UpdClientAdapter::connectedIp() const {
  return (m_client && m_client->isConnected()) ? m_client->targetIp() : "";
}

std::expected<void, std::string>
UpdClientAdapter::readNand(const std::filesystem::path &outputPath,
                           size_t dumpSize, FlashProgressCallback progressCb) {
  if (!m_client || !m_client->isConnected()) {
    return std::unexpected(
        "UpdClient is not connected to any UpdServer instance.");
  }

  size_t effectiveSize = (dumpSize > 0) ? dumpSize : (16 * 1024 * 1024);
  auto cb = [progressCb, effectiveSize](size_t bytesRead, size_t totalSize) {
    if (progressCb) {
      size_t total = totalSize > 0 ? totalSize : effectiveSize;
      float pct =
          total > 0 ? (static_cast<float>(bytesRead) / total * 100.0f) : 0.0f;
      FlashProgressInfo pInfo{.bytesDone = bytesRead,
                              .totalBytes = total,
                              .percentage = pct,
                              .statusMessage = ""};
      progressCb(pInfo);
    }
  };

  auto res = m_client->dumpFlash(outputPath.string(), effectiveSize, cb);
  if (!res.has_value()) {
    return std::unexpected("UpdServer dumpFlash failed: " + res.error());
  }
  return {};
}

std::expected<void, std::string>
UpdClientAdapter::writeNand(const std::filesystem::path &inputPath,
                            FlashProgressCallback progressCb) {
  if (!m_client || !m_client->isConnected()) {
    return std::unexpected(
        "UpdClient is not connected to any UpdServer instance.");
  }

  if (!std::filesystem::exists(inputPath)) {
    return std::unexpected("Input file does not exist: " + inputPath.string());
  }

  auto cb = [progressCb](size_t bytesSent) {
    if (progressCb) {
      FlashProgressInfo pInfo{.bytesDone = bytesSent,
                              .totalBytes = 0,
                              .percentage = 0.0f,
                              .statusMessage = ""};
      progressCb(pInfo);
    }
  };

  auto res = m_client->sendFile(inputPath.string(), "updflash.bin", cb);
  if (!res.has_value()) {
    return std::unexpected("UpdServer sendFile failed: " + res.error());
  }
  return {};
}

std::expected<void, std::string> UpdClientAdapter::rebootConsole() {
  if (!m_client || !m_client->isConnected()) {
    return std::unexpected("UpdClient is not connected.");
  }
  auto res = m_client->reboot();
  if (!res.has_value()) {
    return std::unexpected(res.error());
  }
  return {};
}

std::expected<void, std::string> UpdClientAdapter::shutdownConsole() {
  if (!m_client || !m_client->isConnected()) {
    return std::unexpected("UpdClient is not connected.");
  }
  auto res = m_client->shutdownConsole();
  if (!res.has_value()) {
    return std::unexpected(res.error());
  }
  return {};
}

// IFlashService implementation
std::expected<FlashInfo, std::string>
UpdClientAdapter::getFlashInfo(const FlashDeviceConfig &config) {
  if (!isConnected() && !config.ipAddress.empty()) {
    connect(config.ipAddress, 730);
  }

  if (!isConnected()) {
    return std::unexpected("UpdServer connection failed to target: " +
                           config.ipAddress);
  }

  auto infoRes = m_client->getInfo();
  if (!infoRes.has_value()) {
    return std::unexpected("Failed to get NAND info from UpdServer: " +
                           infoRes.error());
  }

  FlashInfo info;
  info.hardwareName = "Xbox 360 (UpdServer Remote Flasher)";
  info.flashType =
      (infoRes->dumpSize > 64 * 1024 * 1024) ? "eMMC 4GB" : "SPI NAND";
  info.configWord = infoRes->hwFlags;
  info.totalBytes = infoRes->dumpSize;
  return info;
}

std::expected<void, std::string>
UpdClientAdapter::readNand(const std::filesystem::path &outputPath,
                           uint32_t startBlock, uint32_t blockCount,
                           const FlashDeviceConfig &config,
                           FlashProgressCallback progressCb) {
  (void)startBlock;
  if (!isConnected() && !config.ipAddress.empty()) {
    connect(config.ipAddress, 730);
  }

  size_t dumpSize = (blockCount > 0) ? (blockCount * 16 * 1024) : 0;
  return readNand(outputPath, dumpSize, progressCb);
}

std::expected<void, std::string>
UpdClientAdapter::writeNand(const std::filesystem::path &inputPath,
                            uint32_t startBlock, bool eraseFirst,
                            bool verifyAfter, const FlashDeviceConfig &config,
                            FlashProgressCallback progressCb) {
  (void)startBlock;
  (void)eraseFirst;
  (void)verifyAfter;
  if (!isConnected() && !config.ipAddress.empty()) {
    connect(config.ipAddress, 730);
  }
  return writeNand(inputPath, progressCb);
}

std::expected<void, std::string>
UpdClientAdapter::eraseNand(uint32_t startBlock, uint32_t blockCount,
                            const FlashDeviceConfig &config) {
  if (!isConnected() && !config.ipAddress.empty()) {
    connect(config.ipAddress, 730);
  }

  if (!isConnected()) {
    return std::unexpected("UpdClient is not connected.");
  }

  auto res = m_client->eraseBlock(startBlock, blockCount);
  if (!res.has_value()) {
    return std::unexpected(res.error());
  }
  return {};
}

} // namespace gxapi::backend
