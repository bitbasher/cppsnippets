#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>

#include "JsonWriter/JsonWriter.hpp"

class JsonWriterQtTest : public QObject {
  Q_OBJECT

private slots:
  void writesValidObject() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/test-object.json";
    
    QJsonObject obj;
    obj["name"] = "test";
    obj["value"] = 42;
    obj["active"] = true;
    
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), obj, err);
    QVERIFY(ok);
    QVERIFY(!err.hasError());
    
    // Verify file exists and is readable
    QVERIFY(QFileInfo::exists(path));
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray content = file.readAll();
    file.close();
    
    // Verify it's valid JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(content, &parseError);
    QVERIFY(parseError.error == QJsonParseError::NoError);
    QVERIFY(doc.isObject());
    
    QJsonObject readObj = doc.object();
    QCOMPARE(readObj["name"].toString(), QString("test"));
    QCOMPARE(readObj["value"].toInt(), 42);
    QCOMPARE(readObj["active"].toBool(), true);
  }

  void writesValidArray() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/test-array.json";
    
    QJsonArray arr;
    arr.append(1);
    arr.append(2);
    arr.append(3);
    
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeArray(path.toStdString(), arr, err);
    QVERIFY(ok);
    QVERIFY(!err.hasError());
    
    // Verify file exists
    QVERIFY(QFileInfo::exists(path));
    
    // Read and verify
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();
    
    QVERIFY(parseError.error == QJsonParseError::NoError);
    QVERIFY(doc.isArray());
    QJsonArray readArr = doc.array();
    QCOMPARE(readArr.size(), 3);
    QCOMPARE(readArr.at(0).toInt(), 1);
    QCOMPARE(readArr.at(1).toInt(), 2);
    QCOMPARE(readArr.at(2).toInt(), 3);
  }

  void handlesCompactFormat() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/compact.json";
    
    QJsonObject obj;
    obj["a"] = 1;
    obj["b"] = 2;
    
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), obj, err, JsonWriter::Compact);
    QVERIFY(ok);
    
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray content = file.readAll();
    file.close();
    
    // Compact format should not contain newlines (except possibly trailing)
    QString contentStr = QString::fromUtf8(content).trimmed();
    QVERIFY(!contentStr.contains('\n'));
  }

  void handlesIndentedFormat() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/indented.json";
    
    QJsonObject obj;
    obj["a"] = 1;
    obj["b"] = 2;
    
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), obj, err, JsonWriter::Indented);
    QVERIFY(ok);
    
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QByteArray content = file.readAll();
    file.close();
    
    // Indented format should contain newlines
    QString contentStr = QString::fromUtf8(content);
    QVERIFY(contentStr.contains('\n'));
  }

  void reportsWriteErrors() {
    // Try to write to an invalid path (root directory is typically not writable)
#ifdef Q_OS_WIN
    QString invalidPath = "Z:/nonexistent/path/file.json";
#else
    QString invalidPath = "/root/nonexistent/path/file.json";
#endif
    
    QJsonObject obj;
    obj["test"] = true;
    
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(invalidPath.toStdString(), obj, err);
    QVERIFY(!ok);
    QVERIFY(err.hasError());
    QVERIFY(!err.message.empty());
    QVERIFY(!err.formatError().empty());
  }

  void overwritesExistingFile() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/overwrite.json";
    
    // Write first version
    QJsonObject obj1;
    obj1["version"] = 1;
    
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), obj1, err);
    QVERIFY(ok);
    
    // Write second version (should overwrite)
    QJsonObject obj2;
    obj2["version"] = 2;
    
    ok = JsonWriter::writeObject(path.toStdString(), obj2, err);
    QVERIFY(ok);
    
    // Read and verify it's the second version
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    QVERIFY(doc.isObject());
    QCOMPARE(doc.object()["version"].toInt(), 2);
  }
};

QTEST_MAIN(JsonWriterQtTest)
#include "jsonwriter_qttest.moc"
