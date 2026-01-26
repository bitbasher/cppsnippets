#pragma once

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// Portable JsonWriter library for writing JSON documents to files.
// Companion to JsonReader with consistent error reporting.

struct JsonWriteErrorInfo {
  std::string message;
  std::string filename;

  [[nodiscard]] bool hasError() const { return !message.empty(); }

  void clear() {
    message.clear();
    filename.clear();
  }

  [[nodiscard]] std::string formatError() const;
};

class JsonWriter {
public:
  enum FormatStyle {
    Compact,    // Single line, no whitespace
    Indented    // Pretty-printed with indentation
  };

  static bool writeFile(const fs::path& path, const QJsonDocument& doc, JsonWriteErrorInfo& error, FormatStyle style = Indented);
  static bool writeFile(const std::string& path, const QJsonDocument& doc, JsonWriteErrorInfo& error, FormatStyle style = Indented);

  static bool writeObject(const fs::path& path, const QJsonObject& obj, JsonWriteErrorInfo& error, FormatStyle style = Indented);
  static bool writeObject(const std::string& path, const QJsonObject& obj, JsonWriteErrorInfo& error, FormatStyle style = Indented);

  static bool writeArray(const fs::path& path, const QJsonArray& arr, JsonWriteErrorInfo& error, FormatStyle style = Indented);
  static bool writeArray(const std::string& path, const QJsonArray& arr, JsonWriteErrorInfo& error, FormatStyle style = Indented);
};
