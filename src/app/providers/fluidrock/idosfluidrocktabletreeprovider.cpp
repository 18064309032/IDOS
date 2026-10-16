#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QObject>
#include <QVariant>

#include "data/fluidrock/idosfluidrocktable.h"
#include "idosdataobject.h"
#include "tree/idostreeitem.h"

#include "idosfluidrocktabletreeprovider.h"

IDOSFluidRockTableTreeProvider::IDOSFluidRockTableTreeProvider(IDOSDataObject* data)
    :
    QObject(nullptr)
    , IDOSTreeItemProvider(data)
{
}

QAction* IDOSFluidRockTableTreeProvider::addAction(QMenu* menu, const QString& text)
{
    QAction* action = menu->addAction(text);
    IDOSTreeItem* treeItem = item();
    action->setProperty("dataId", treeItem != nullptr ? treeItem->provider()->dataId() : QString());
    return action;
}

QMenu* IDOSFluidRockTableTreeProvider::createContextMenu()
{
    IDOSTreeItem* treeItem = item();
    if (treeItem == nullptr || !treeItem->provider()->caseId().isEmpty())
    {
        return nullptr;
    }
    QMenu* menu = new QMenu();
    if (treeItem->provider()->key() == QStringLiteral("data.fluids") ||
        treeItem->provider()->key() == QStringLiteral("data.rockPhysics"))
    {
        const bool isFluidCategory = treeItem->provider()->key() == QStringLiteral("data.fluids");
        const QString actionText = isFluidCategory
            ? tr("Import Fluid Tables...") : tr("Import Rock Physics Functions...");
        QAction* import = addAction(menu, actionText);
        import->setObjectName(isFluidCategory
                                  ? QStringLiteral("importFluidTablesAction")
                                  : QStringLiteral("importRockPhysicsTablesAction"));
        import->setProperty("category", isFluidCategory
                                             ? QStringLiteral("fluid")
                                             : QStringLiteral("rockPhysics"));
        import->setProperty("operation", QStringLiteral("node.importFluidRockTables"));
        if (isFluidCategory)
        {
            QAction* catalog = addAction(menu, tr("Import Fluid Catalog..."));
            catalog->setObjectName(QStringLiteral("importFluidCatalogAction"));
            catalog->setProperty("providerId", QStringLiteral("idos.fracture.materialCatalog.tsv"));
            catalog->setProperty("dataTypeId", QStringLiteral("idos.fracture.fluidCatalog"));
            catalog->setProperty("fileFilter", tr("Fluid Catalog (*.txt *.TXT);;All Files (*)"));
            catalog->setProperty("operation", QStringLiteral("node.importDataset"));
        }
        return menu;
    }
    IDOSFluidRockTable* table = qobject_cast<IDOSFluidRockTable*>(
        dataSource());
    if (table != nullptr)
    {
        QAction* add = addAction(menu, tr("Add to Simulation Case..."));
        add->setObjectName(QStringLiteral("addFluidRockTableToCaseAction"));
        add->setProperty("operation", QStringLiteral("node.addFluidRockTableToCase"));
    }
    return menu;
}

void IDOSFluidRockTableTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    IDOSFluidRockTable* table = qobject_cast<IDOSFluidRockTable*>(dataSource());
    if (item == nullptr || table == nullptr)
    {
        return;
    }
    item->provider()->setName(table->name());
    item->provider()->setIcon(QIcon(table->category() == IDOSFluidRockTable::Category::Fluid
        ? QStringLiteral(":/images/gui-fluids.svg")
        : QStringLiteral(":/images/gui-rock-physics.svg")));
}
