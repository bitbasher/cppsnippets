/*
 * @file template_tree_demo.cpp
 * @brief Minimal GUI to display TemplateTreeModel in a QTreeView
 */

#include <QApplication>
#include <QTreeView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QDir>

#include "resourceInventory/resourceScannerDirListing.h"
#include "resourceInventory/resourceStore.h"
#include "resourceInventory/templateTreeModel.h"

using namespace resourceInventory;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("TemplateTreeDemo");

    // Scan installation templates from test structure if available; otherwise no-op
    QString installPath;
    {
        QDir dir(QCoreApplication::applicationDirPath());
        QStringList searchPaths = {
            dir.absoluteFilePath("../../testFileStructure/inst/OpenSCAD"),
            dir.absoluteFilePath("../../../testFileStructure/inst/OpenSCAD"),
            QDir::currentPath() + "/testFileStructure/inst/OpenSCAD"
        };
        for (const QString& p : searchPaths) {
            if (QDir(p).exists()) { installPath = p; break; }
        }
    }

    ResourceStore store;
    ResourceScannerDirListing scanner;
    if (!installPath.isEmpty()) {
        store.scanTypeAndStore(scanner, installPath, ResourceType::Templates,
                               ResourceTier::Installation, installPath);
    }

    TemplateTreeModel* model = new TemplateTreeModel;
    model->setResourceStore(&store);

    auto* view = new QTreeView;
    view->setModel(model);
    view->setAlternatingRowColors(true);
    view->setUniformRowHeights(true);
    view->setAnimated(true);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->header()->setStretchLastSection(false);
    view->header()->setSectionResizeMode(0, QHeaderView::Stretch); // Name
    view->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents); // Category
    view->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Path

    view->expandAll();

    auto* lbl = new QLabel("Hover a row to see full path in tooltip.\n"
                           "Name shows the tier/location/library or template file name.");

    auto* root = new QWidget;
    auto* layout = new QVBoxLayout(root);
    layout->addWidget(lbl);
    layout->addWidget(view);

    root->resize(800, 500);
    root->setWindowTitle("Template Tree Demo");
    root->show();

    return app.exec();
}
