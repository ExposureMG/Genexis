#pragma once

#include "adapters/Gxbuild3.hpp"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace genexis::utils::files::xebuildini {

using IniEntry = genexis::adapters::gxb3_ini::IniEntry;

/// Describes a console bootloader target section parsed from a xeBuild INI file.
struct ConsoleSection {
    std::string section; // e.g. "xenonbl", "coronabl_wb"
    std::string console; // e.g. "xenon", "corona"
    std::string variant; // e.g. "", "elpis", "wb", "wb4g"
    std::string stem;    // e.g. "xenon", "corona_wb"
};

/// High-level wrapper around xeBuild _*.ini files backed by gxb3_ini.
class XebuildIni {
public:
    /// Loads and parses a xeBuild INI file from \p iniPath.
    [[nodiscard]] static std::expected<XebuildIni, std::string>
    load(const std::filesystem::path &iniPath);

    /// Parses raw xeBuild INI text.
    [[nodiscard]] static std::expected<XebuildIni, std::string>
    parseText(std::string_view content);

    /// Returns the build version from the [version] section, or empty if missing.
    [[nodiscard]] std::string version() const;

    /// Returns true if \p name exists as a section (case-insensitive).
    [[nodiscard]] bool hasSection(std::string_view name) const;

    /// Returns all entries in \p sectionName (case-insensitive), or empty if missing.
    [[nodiscard]] std::vector<IniEntry> entries(std::string_view sectionName) const;

    /// Returns sorted section names present in the INI document.
    [[nodiscard]] std::vector<std::string> sectionNames() const;

    /// Discovers all console bootloader target sections, excluding metadata sections.
    [[nodiscard]] std::vector<ConsoleSection> consoleTargets() const;

private:
    explicit XebuildIni(genexis::adapters::gxb3_ini::IniDocument doc);

    genexis::adapters::gxb3_ini::IniDocument m_doc;
};

} // namespace genexis::utils::files::xebuildini
