#include "adapters/Gxbuild3.hpp"

#include "ini/IniParser.hpp"
#include "utils/Log.hpp"

#include <algorithm>
#include <cctype>
#include <mutex>

namespace genexis::adapters::gxbuild3 {

namespace {
std::mutex g_logMutex;
bool g_loggingInitialized = false;
} // namespace

void initLogging() {
    std::scoped_lock lock(g_logMutex);
    if (g_loggingInitialized) {
        return;
    }
    ::gxbuild3::Log::Init();
    g_loggingInitialized = true;
}

void shutdownLogging() {
    std::scoped_lock lock(g_logMutex);
    if (!g_loggingInitialized) {
        return;
    }
    ::gxbuild3::Log::Shutdown();
    g_loggingInitialized = false;
}

} // namespace genexis::adapters::gxbuild3

namespace genexis::adapters::gxb3_ini {

namespace {

std::string toLower(std::string_view sv) {
    std::string out(sv);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

IniDocument convertDocument(const ::gxbuild3::ini::Document &src) {
    IniDocument doc;
    doc.sections.reserve(src.sections.size());

    for (const auto &[sectionName, srcEntries] : src.sections) {
        IniSection entries;
        entries.reserve(srcEntries.size());
        for (const auto &entry : srcEntries) {
            entries.push_back(IniEntry{
                .key = entry.key,
                .value = entry.value,
                .hash = entry.hash,
                .chain = entry.chain,
            });
        }
        doc.sections.emplace(sectionName, std::move(entries));
    }

    return doc;
}

} // namespace

const IniSection *IniDocument::get(std::string_view name) const {
    const auto it = sections.find(toLower(name));
    return it != sections.end() ? &it->second : nullptr;
}

std::expected<IniDocument, std::string> parseFile(const std::filesystem::path &path) {
    auto res = ::gxbuild3::ini::parse_file(path);
    if (!res) {
        return std::unexpected(res.error().describe());
    }
    return convertDocument(*res);
}

std::expected<IniDocument, std::string> parseText(std::string_view content) {
    auto res = ::gxbuild3::ini::parse(content);
    if (!res) {
        return std::unexpected(res.error().describe());
    }
    return convertDocument(*res);
}

} // namespace genexis::adapters::gxb3_ini
