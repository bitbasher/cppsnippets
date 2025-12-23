#include "JsonWriter/JsonWriter.h"

#include <QFile>
#include <QSaveFile>

std::string JsonWriteErrorInfo::formatError() const
{
  if (message.empty()) return "";

  std::string result = filename;
  if (!result.empty()) result += ": ";

  result += message;
  return result;
}

bool JsonWriter::writeFile(const fs::path& path, const QJsonDocument& doc, JsonWriteErrorInfo& error, FormatStyle style)
{
  return writeFile(path.generic_string(), doc, error, style);
}

bool JsonWriter::writeFile(const std::string& path, const QJsonDocument& doc, JsonWriteErrorInfo& error, FormatStyle style)
{
  error.clear();
  error.filename = path;

  // Use QSaveFile for atomic write (creates temp file, then renames on commit)
  QSaveFile file(QString::fromStdString(path));
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    error.message = "Cannot open file for writing: " + file.errorString().toStdString();
    return false;
  }

  QByteArray jsonData = (style == Indented) 
    ? doc.toJson(QJsonDocument::Indented) 
    : doc.toJson(QJsonDocument::Compact);

  qint64 written = file.write(jsonData);
  if (written != jsonData.size()) {
    error.message = "Failed to write complete data: " + file.errorString().toStdString();
    file.cancelWriting();
    return false;
  }

  if (!file.commit()) {
    error.message = "Failed to commit file: " + file.errorString().toStdString();
    return false;
  }

  return true;
}

bool JsonWriter::writeObject(const fs::path& path, const QJsonObject& obj, JsonWriteErrorInfo& error, FormatStyle style)
{
  return writeObject(path.generic_string(), obj, error, style);
}

bool JsonWriter::writeObject(const std::string& path, const QJsonObject& obj, JsonWriteErrorInfo& error, FormatStyle style)
{
  QJsonDocument doc(obj);
  return writeFile(path, doc, error, style);
}

bool JsonWriter::writeArray(const fs::path& path, const QJsonArray& arr, JsonWriteErrorInfo& error, FormatStyle style)
{
  return writeArray(path.generic_string(), arr, error, style);
}

bool JsonWriter::writeArray(const std::string& path, const QJsonArray& arr, JsonWriteErrorInfo& error, FormatStyle style)
{
  QJsonDocument doc(arr);
  return writeFile(path, doc, error, style);
}
