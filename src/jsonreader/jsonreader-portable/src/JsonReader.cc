#include "JsonReader/JsonReader.hpp"

#include <QFile>

std::string JsonErrorInfo::formatError() const
{
  if (message.empty()) return "";

  std::string result = filename;
  if (!result.empty()) result += ":";

  if (line > 0) {
    result += std::to_string(line);
    if (column > 0) result += ":" + std::to_string(column);
    result += ": ";
  } else if (!filename.empty()) {
    result += " ";
  }

  result += message;
  return result;
}

void JsonReader::offsetToLineColumn(const QByteArray& content, int offset, int& line, int& column)
{
  line = 1;
  column = 1;
  for (int i = 0; i < offset && i < content.size(); ++i) {
    if (content[i] == '\n') {
      ++line;
      column = 1;
    } else {
      ++column;
    }
  }
}

bool JsonReader::readFile(const fs::path& path, QJsonDocument& doc, JsonErrorInfo& error)
{
  return readFile(path.generic_string(), doc, error);
}

bool JsonReader::readFile(const std::string& path, QJsonDocument& doc, JsonErrorInfo& error)
{
  error.clear();
  error.filename = path;

  QFile file(QString::fromStdString(path));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    error.message = "Cannot open file for reading";
    return false;
  }

  QByteArray content = file.readAll();
  file.close();

  QJsonParseError parseError;
  doc = QJsonDocument::fromJson(content, &parseError);

  if (parseError.error != QJsonParseError::NoError) {
    error.message = parseError.errorString().toStdString();
    error.offset = parseError.offset;
    offsetToLineColumn(content, parseError.offset, error.line, error.column);
    return false;
  }

  return true;
}

bool JsonReader::readObject(const fs::path& path, QJsonObject& obj, JsonErrorInfo& error)
{
  return readObject(path.generic_string(), obj, error);
}

bool JsonReader::readObject(const std::string& path, QJsonObject& obj, JsonErrorInfo& error)
{
  QJsonDocument doc;
  if (!readFile(path, doc, error)) return false;
  if (!doc.isObject()) { error.message = "JSON root must be an object"; return false; }
  obj = doc.object();
  return true;
}

bool JsonReader::readArray(const fs::path& path, QJsonArray& arr, JsonErrorInfo& error)
{
  return readArray(path.generic_string(), arr, error);
}

bool JsonReader::readArray(const std::string& path, QJsonArray& arr, JsonErrorInfo& error)
{
  QJsonDocument doc;
  if (!readFile(path, doc, error)) return false;
  if (!doc.isArray()) { error.message = "JSON root must be an array"; return false; }
  arr = doc.array();
  return true;
}
