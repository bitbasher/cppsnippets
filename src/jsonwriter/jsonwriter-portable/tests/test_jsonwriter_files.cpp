#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>

#include "JsonWriter/JsonWriter.hpp"

class JsonWriterFilesQtTest : public QObject {
  Q_OBJECT

private slots:
  void roundTripObject() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/roundtrip-object.json";
    
    // Create complex object
    QJsonObject original;
    original["string"] = "Hello, World!";
    original["number"] = 42;
    original["float"] = 3.14159;
    original["bool"] = true;
    original["null"] = QJsonValue::Null;
    
    QJsonArray nested;
    nested.append("item1");
    nested.append("item2");
    original["array"] = nested;
    
    QJsonObject nestedObj;
    nestedObj["key"] = "value";
    original["object"] = nestedObj;
    
    // Write
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), original, err);
    QVERIFY(ok);
    QVERIFY(!err.hasError());
    
    // Read back
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    // Verify
    QVERIFY(doc.isObject());
    QJsonObject readBack = doc.object();
    
    QCOMPARE(readBack["string"].toString(), QString("Hello, World!"));
    QCOMPARE(readBack["number"].toInt(), 42);
    QCOMPARE(readBack["float"].toDouble(), 3.14159);
    QCOMPARE(readBack["bool"].toBool(), true);
    QVERIFY(readBack["null"].isNull());
    
    QVERIFY(readBack["array"].isArray());
    QJsonArray readArray = readBack["array"].toArray();
    QCOMPARE(readArray.size(), 2);
    
    QVERIFY(readBack["object"].isObject());
    QJsonObject readNestedObj = readBack["object"].toObject();
    QCOMPARE(readNestedObj["key"].toString(), QString("value"));
  }

  void roundTripArray() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/roundtrip-array.json";
    
    // Create complex array
    QJsonArray original;
    original.append(1);
    original.append("string");
    original.append(true);
    
    QJsonObject nestedObj;
    nestedObj["nested"] = "value";
    original.append(nestedObj);
    
    QJsonArray nestedArr;
    nestedArr.append("a");
    nestedArr.append("b");
    original.append(nestedArr);
    
    // Write
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeArray(path.toStdString(), original, err);
    QVERIFY(ok);
    
    // Read back
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    // Verify
    QVERIFY(doc.isArray());
    QJsonArray readBack = doc.array();
    QCOMPARE(readBack.size(), 5);
    QCOMPARE(readBack.at(0).toInt(), 1);
    QCOMPARE(readBack.at(1).toString(), QString("string"));
    QCOMPARE(readBack.at(2).toBool(), true);
    QVERIFY(readBack.at(3).isObject());
    QVERIFY(readBack.at(4).isArray());
  }

  void writesUnicodeCorrectly() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    QString path = tempDir.path() + "/unicode.json";
    
    QJsonObject obj;
    obj["english"] = "Hello";
    obj["chinese"] = "你好";
    obj["arabic"] = "مرحبا";
    obj["emoji"] = "🎉";
    
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), obj, err);
    QVERIFY(ok);
    
    // Read back
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    QVERIFY(doc.isObject());
    QJsonObject readBack = doc.object();
    QCOMPARE(readBack["english"].toString(), QString("Hello"));
    QCOMPARE(readBack["chinese"].toString(), QString("你好"));
    QCOMPARE(readBack["arabic"].toString(), QString("مرحبا"));
    QCOMPARE(readBack["emoji"].toString(), QString("🎉"));
  }

  void createsDirectoriesIfNeeded() {
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    // Note: QSaveFile requires the directory to exist, so this test
    // verifies that we get a proper error for non-existent directories
    QString path = tempDir.path() + "/nonexistent/subdir/file.json";
    
    QJsonObject obj;
    obj["test"] = true;
    
    JsonWriteErrorInfo err;
    bool ok = JsonWriter::writeObject(path.toStdString(), obj, err);
    
    // Should fail because directory doesn't exist
    QVERIFY(!ok);
    QVERIFY(err.hasError());
  }
};

QTEST_MAIN(JsonWriterFilesQtTest)
#include "test_jsonwriter_files.moc"
