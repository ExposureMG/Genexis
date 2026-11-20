#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace genexis::adapters::gxbuild3 {

/// Initializes gxbuild3's spdlog logger idempotently.
void initLogging();

/// Shuts down gxbuild3's spdlog logger if initialized.
void shutdownLogging();

} // namespace genexis::adapters::gxbuild3

namespace genexis::adapters::gxb3_ini {

/// Represents a single parsed entry inside an INI section.
struct IniEntry {
    std::string key;
    std::string value;
    std::string hash;
    uint8_t chain{0};
};

using IniSection = std::vector<IniEntry>;

/// Holds parsed INI sections indexed by lowercased section name.
struct IniDocument {
    std::unordered_map<std::string, IniSection> sections;

    /// Returns the section matching \p name (case-insensitive), or nullptr if absent.
    [[nodiscard]] const IniSection *get(std::string_view name) const;
};

/// Parses an INI file at \p path using gxbuild3's parser.
[[nodiscard]] std::expected<IniDocument, std::string> parseFile(const std::filesystem::path &path);

/// Parses raw INI text using gxbuild3's parser.
[[nodiscard]] std::expected<IniDocument, std::string> parseText(std::string_view content);

} // namespace genexis::adapters::gxb3_ini
