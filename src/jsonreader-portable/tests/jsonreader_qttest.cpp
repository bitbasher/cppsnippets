#include <QtTest/QtTest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>

#include "JsonReader/JsonReader.h"

class JsonReaderQtTest : public QObject {
  Q_OBJECT

private:
  QString dataDir() const {
    // tests/data/json relative to this source file
    QFileInfo fi(__FILE__);
    QDir d = fi.absoluteDir();
    d.cd("data");
    d.cd("json");
    return d.absolutePath();
  }

private slots:
  void readsValidArray() {
    QString path = dataDir() + "/valid-array.json";
    QVERIFY(QFileInfo::exists(path));

    QJsonArray arr; JsonErrorInfo err;
    bool ok = JsonReader::readArray(path.toStdString(), arr, err);
    QVERIFY(ok);
    QVERIFY(!err.hasError());
    QCOMPARE(arr.size(), 3);
    QCOMPARE(arr.at(0).toInt(), 1);
    QCOMPARE(arr.at(1).toInt(), 2);
    QCOMPARE(arr.at(2).toInt(), 3);
  }

  void reportsUnterminatedObject() {
    QString path = dataDir() + "/error-unterminated-object.json";
    QVERIFY(QFileInfo::exists(path));

    QJsonDocument doc; JsonErrorInfo err;
    bool ok = JsonReader::readFile(path.toStdString(), doc, err);
    QVERIFY(!ok);
    QVERIFY(err.hasError());
    QVERIFY(err.line > 0);
    QVERIFY(err.column > 0);
    QVERIFY(QString::fromStdString(err.formatError()).contains(":"));
  }

  void reportsMissingComma() {
    QString path = dataDir() + "/error-missing-comma.json";
    QVERIFY(QFileInfo::exists(path));

    QJsonDocument doc; JsonErrorInfo err;
    bool ok = JsonReader::readFile(path.toStdString(), doc, err);
    QVERIFY(!ok);
    QVERIFY(err.hasError());
    QVERIFY(err.line >= 1);
  }

  void reportsUnterminatedString() {
    QString path = dataDir() + "/error-unterminated-string.json";
    QVERIFY(QFileInfo::exists(path));

    QJsonDocument doc; JsonErrorInfo err;
    bool ok = JsonReader::readFile(path.toStdString(), doc, err);
    QVERIFY(!ok);
    QVERIFY(err.hasError());
  }
};

QTEST_MAIN(JsonReaderQtTest)
#include "jsonreader_qttest.moc"
