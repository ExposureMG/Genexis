#include "utils/files/XebuildIni.hpp"

#include <algorithm>
#include <array>

namespace genexis::utils::files::xebuildini {

namespace {

constexpr std::array<std::string_view, 4> kMetadataSections = {
    "version",
    "security",
    "flashfs",
    "rawpatch",
};

bool isMetadataSection(std::string_view name) {
    return std::find(kMetadataSections.begin(), kMetadataSections.end(), name) !=
           kMetadataSections.end();
}

} // namespace

XebuildIni::XebuildIni(genexis::adapters::gxb3_ini::IniDocument doc) : m_doc(std::move(doc)) {}

std::expected<XebuildIni, std::string> XebuildIni::load(const std::filesystem::path &iniPath) {
    auto doc = genexis::adapters::gxb3_ini::parseFile(iniPath);
    if (!doc) {
        return std::unexpected(std::move(doc.error()));
    }
    return XebuildIni(std::move(*doc));
}

std::expected<XebuildIni, std::string> XebuildIni::parseText(std::string_view content) {
    auto doc = genexis::adapters::gxb3_ini::parseText(content);
    if (!doc) {
        return std::unexpected(std::move(doc.error()));
    }
    return XebuildIni(std::move(*doc));
}

std::string XebuildIni::version() const {
    const auto *sec = m_doc.get("version");
    if (!sec || sec->empty()) {
        return {};
    }
    return sec->front().key;
}

bool XebuildIni::hasSection(std::string_view name) const {
    return m_doc.get(name) != nullptr;
}

std::vector<IniEntry> XebuildIni::entries(std::string_view sectionName) const {
    const auto *sec = m_doc.get(sectionName);
    if (!sec) {
        return {};
    }
    return *sec;
}

std::vector<std::string> XebuildIni::sectionNames() const {
    std::vector<std::string> names;
    names.reserve(m_doc.sections.size());
    for (const auto &[name, _] : m_doc.sections) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::vector<ConsoleSection> XebuildIni::consoleTargets() const {
    std::vector<ConsoleSection> targets;
    for (const auto &[sectionName, _] : m_doc.sections) {
        if (isMetadataSection(sectionName)) {
            continue;
        }

        const auto usPos = sectionName.find('_');
        const std::string base =
            (usPos != std::string::npos) ? sectionName.substr(0, usPos) : sectionName;
        if (!base.ends_with("bl") || base.size() <= 2) {
            continue;
        }

        const std::string console = base.substr(0, base.size() - 2);
        const std::string variant =
            (usPos != std::string::npos) ? sectionName.substr(usPos + 1) : std::string{};
        const std::string stem = variant.empty() ? console : (console + "_" + variant);

        targets.push_back(ConsoleSection{
            .section = sectionName,
            .console = console,
            .variant = variant,
            .stem = stem,
        });
    }

    std::sort(targets.begin(), targets.end(),
              [](const ConsoleSection &a, const ConsoleSection &b) {
                  return a.section < b.section;
              });
    return targets;
}

} // namespace genexis::utils::files::xebuildini
