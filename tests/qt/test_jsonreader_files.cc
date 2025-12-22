#include <QtTest/QtTest>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>

#include "JsonReader/JsonReader.h"
#include "scadtemplates/template_parser.h"

using namespace scadtemplates;

class TestJsonReaderFiles : public QObject {
    Q_OBJECT

private:
    QString m_testDataPath;
    QString m_installPath;
    QString m_userPath;

    QString findTestData() const {
        QDir dir(QCoreApplication::applicationDirPath());
        QStringList searchPaths = {
            dir.absoluteFilePath("../../testFileStructure"),
            dir.absoluteFilePath("../../../testFileStructure"),
            QDir::currentPath() + "/testFileStructure",
            QString(SRCDIR) + "/testFileStructure"
        };

        for (const QString& path : searchPaths) {
            QDir testDir(path);
            if (testDir.exists() && testDir.exists("inst/OpenSCAD")) {
                return testDir.absolutePath();
            }
        }
        return QString();
    }

private slots:
    void initTestCase() {
        m_testDataPath = findTestData();
        QVERIFY2(!m_testDataPath.isEmpty(), "Could not find testFileStructure");

        m_installPath = QDir(m_testDataPath).absoluteFilePath("inst/OpenSCAD");
        m_userPath = QDir(m_testDataPath).absoluteFilePath("pers/Jeff/Documents/OpenSCAD");

        QVERIFY(QDir(m_installPath).exists());
        QVERIFY(QDir(m_userPath).exists());
    }

    void readsEditorColorScheme() {
        QString path = QDir(m_installPath).absoluteFilePath("color-schemes/editor/high-contrast-dark.json");
        QFileInfo fi(path);
        QVERIFY2(fi.exists(), "Editor color scheme file missing");

        QJsonObject obj; JsonErrorInfo err;
        bool ok = JsonReader::readObject(path.toStdString(), obj, err);
        QVERIFY2(ok, QString::fromStdString(err.formatError()).toUtf8().constData());

        QCOMPARE(obj.value("name").toString(), QStringLiteral("High Contrast Dark"));
        QVERIFY(obj.value("colors").isObject());
        QJsonObject colors = obj.value("colors").toObject();
        QVERIFY(colors.contains("keyword1"));
        QVERIFY(colors.contains("selection-background"));
    }

    void readsRenderColorScheme() {
        QString path = QDir(m_installPath).absoluteFilePath("color-schemes/render/metallic.json");
        QFileInfo fi(path);
        QVERIFY2(fi.exists(), "Render color scheme file missing");

        QJsonObject obj; JsonErrorInfo err;
        bool ok = JsonReader::readObject(path.toStdString(), obj, err);
        QVERIFY2(ok, QString::fromStdString(err.formatError()).toUtf8().constData());

        QCOMPARE(obj.value("name").toString(), QStringLiteral("Metallic"));
        QVERIFY(obj.value("colors").isObject());
        QJsonObject colors = obj.value("colors").toObject();
        QVERIFY(colors.contains("background"));
        QVERIFY(colors.contains("cgal-edge-front"));
    }

    void parsesLegacyTemplate() {
        QString path = QDir(m_userPath).absoluteFilePath("templates/jeffdoc.json");
        QFileInfo fi(path);
        QVERIFY2(fi.exists(), "Legacy template file missing");

        TemplateParser parser;
        auto result = parser.parseFile(path.toStdString());
        QVERIFY2(result.success, QString::fromStdString(result.errorMessage).toUtf8().constData());
        QCOMPARE(result.templates.size(), static_cast<size_t>(1));

        const Template& tmpl = result.templates.front();
        QCOMPARE(QString::fromStdString(tmpl.getPrefix()), QStringLiteral("jeffdoc"));
        QVERIFY(!QString::fromStdString(tmpl.getBody()).isEmpty());
    }

    void parsesModernTemplate() {
        QString path = QDir(m_installPath).absoluteFilePath("templates/inst_template_sphere_param.json");
        QFileInfo fi(path);
        QVERIFY2(fi.exists(), "Modern template file missing");

        QJsonObject obj; JsonErrorInfo err;
        bool ok = JsonReader::readObject(path.toStdString(), obj, err);
        QVERIFY2(ok, QString::fromStdString(err.formatError()).toUtf8().constData());

        QCOMPARE(obj.value("prefix").toString(), QStringLiteral("sphere_param"));
        QVERIFY(obj.value("body").isArray());
        auto body = obj.value("body").toArray();
        QVERIFY(body.size() > 0);
    }
};

QTEST_MAIN(TestJsonReaderFiles)
#include "test_jsonreader_files.moc"
