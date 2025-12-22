#pragma once

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// Portable version of JsonErrorInfo and JsonReader extracted from OpenSCAD.
// No OpenSCAD-specific dependencies.

struct JsonErrorInfo {
  std::string message;
  std::string filename;
  int line = 0;
  int column = 0;
  int offset = 0;

  [[nodiscard]] bool hasError() const { return !message.empty(); }

  void clear() {
    message.clear();
    filename.clear();
    line = 0;
    column = 0;
    offset = 0;
  }

  [[nodiscard]] std::string formatError() const;
};

class JsonReader {
public:
  static bool readFile(const fs::path& path, QJsonDocument& doc, JsonErrorInfo& error);
  static bool readFile(const std::string& path, QJsonDocument& doc, JsonErrorInfo& error);

  static bool readObject(const fs::path& path, QJsonObject& obj, JsonErrorInfo& error);
  static bool readObject(const std::string& path, QJsonObject& obj, JsonErrorInfo& error);

  static bool readArray(const fs::path& path, QJsonArray& arr, JsonErrorInfo& error);
  static bool readArray(const std::string& path, QJsonArray& arr, JsonErrorInfo& error);

private:
  static void offsetToLineColumn(const QByteArray& content, int offset, int& line, int& column);
};
