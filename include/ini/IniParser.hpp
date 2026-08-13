#pragma once

#include <array>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Ini {

enum class ParseError {
  FileNotFound,
  ReadError,
  SectionNotFound,
  MalformedEntry,
  WriteError,
};

constexpr std::string_view ParseErrorString(ParseError e) {
  switch (e) {
  case ParseError::FileNotFound:
    return "File not found";
  case ParseError::ReadError:
    return "Failed to read file";
  case ParseError::SectionNotFound:
    return "Section not found";
  case ParseError::MalformedEntry:
    return "Malformed entry";
  case ParseError::WriteError:
    return "Failed to write file";
  }
  return "Unknown";
}

struct Entry {
  std::string key;
  std::string value;
  std::string hash; // may be empty
  uint8_t chain{0};
};

using Section = std::vector<Entry>;

struct Document {
  std::unordered_map<std::string, Section> sections;

  [[nodiscard]] const Section *get(std::string_view name) const;

  [[nodiscard]] std::expected<const Section *, ParseError>
  require(std::string_view name) const;

  void set(std::string_view section_name, std::string_view key,
           std::string_view value, std::string_view hash = "");

  void add_entry(std::string_view section_name, Entry entry);
};

[[nodiscard]] std::expected<Document, ParseError>
Parse(std::string_view content);
[[nodiscard]] std::expected<Document, ParseError>
ParseFile(const std::filesystem::path &path);

[[nodiscard]] std::string Serialize(const Document &doc,
                                    bool xebuild_format = false);
[[nodiscard]] std::string Create(const Document &doc,
                                 bool xebuild_format = false);
[[nodiscard]] std::expected<void, ParseError>
SaveFile(const Document &doc, const std::filesystem::path &path,
         bool xebuild_format = false);

[[nodiscard]] Entry MakeEntry(std::string_view key, std::string_view value,
                              std::string_view hash = "");

[[nodiscard]] std::string_view Trim(std::string_view sv);

} // namespace Ini
