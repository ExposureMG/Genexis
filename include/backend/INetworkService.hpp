#pragma once

#include "IFlashService.hpp"
#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace gxapi::backend {

struct DiscoveredConsoleInfo {
  std::string ipAddress;
  uint16_t port{0};
  std::string consoleType;
};

class INetworkService {
public:
  virtual ~INetworkService() = default;

  [[nodiscard]] virtual std::string serviceName() const = 0;

  virtual std::vector<DiscoveredConsoleInfo> discoverConsoles() = 0;
  virtual bool connect(const std::string &ipAddress, uint16_t port = 730) = 0;
  virtual void disconnect() = 0;
  [[nodiscard]] virtual bool isConnected() const = 0;
  [[nodiscard]] virtual std::string connectedIp() const = 0;

  virtual std::expected<void, std::string>
  readNand(const std::filesystem::path &outputPath, size_t dumpSize = 0,
           FlashProgressCallback progressCb = nullptr) = 0;

  virtual std::expected<void, std::string>
  writeNand(const std::filesystem::path &inputPath,
            FlashProgressCallback progressCb = nullptr) = 0;

  virtual std::expected<void, std::string> rebootConsole() = 0;
  virtual std::expected<void, std::string> shutdownConsole() = 0;
};

} 
