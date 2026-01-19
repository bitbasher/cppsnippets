// Simple smoke test for TemplatesInventory as QAbstractItemModel
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include "../src/platformInfo/ResourceLocation.hpp"
#include "../src/resourceInventory/TemplatesInventory.hpp"

using namespace resourceInventory;
using namespace platformInfo;
using namespace resourceMetadata;

static bool writeTemplateFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open" << path;
        return false;
    }
    QTextStream out(&f);
    out << "{\n"
        << "  \"foo\": {\n"
        << "    \"prefix\": \"foo\",\n"
        << "    \"body\": [\"cube(10);\"],\n"
        << "    \"description\": \"sample\"\n"
        << "  }\n"
        << "}\n";
    return true;
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QTemporaryDir tmp;
    if (!tmp.isValid()) {
        qCritical() << "Failed to create temp dir";
        return 1;
    }

    // Create templates folder and file
    QString templatesDir = tmp.path() + "/templates";
    QDir().mkpath(templatesDir);
    QString filePath = templatesDir + "/foo.json";
    if (!writeTemplateFile(filePath)) {
        return 1;
    }

    // Build ResourceLocation and inventory
    ResourceLocation loc(tmp.path(), ResourceTier::User);
    TemplatesInventory model;
    int added = model.scanLocation(loc);
    if (added != 1) {
        qCritical() << "Expected 1 template, got" << added;
        return 1;
    }

    if (model.rowCount() != 1 || model.columnCount() != 2) {
        qCritical() << "Unexpected model shape" << model.rowCount() << model.columnCount();
        return 1;
    }

    QModelIndex nameIdx = model.index(0, 0);
    QModelIndex idIdx = model.index(0, 1);
    if (!nameIdx.isValid() || !idIdx.isValid()) {
        qCritical() << "Invalid indexes";
        return 1;
    }

    QVariant tmplVar = model.data(nameIdx, Qt::UserRole);
    ResourceTemplate tmpl = tmplVar.value<ResourceTemplate>();
    if (tmpl.path().isEmpty()) {
        qCritical() << "Template payload missing";
        return 1;
    }

    if (model.data(nameIdx).toString() != QStringLiteral("foo")) {
        qCritical() << "Name column mismatch" << model.data(nameIdx).toString();
        return 1;
    }

    qInfo() << "TemplatesInventory model smoke test passed";
    return 0;
}
