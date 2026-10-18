#include <QAction>
#include <QIcon>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QObject>
#include <QVariant>
#include <QVariantMap>

#include "data/fluidrock/idosfluidrocktableobject.h"
#include "data/well/idoswellobject.h"
#include "idosdataobject.h"
#include "tree/idoscasetreeitemmodel.h"
#include "tree/idosdatatreeitemmodel.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"

#include "idosfluidrocktabletreeprovider.h"

IDOSFluidRockTableTreeProvider::IDOSFluidRockTableTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeItemProvider(data)
{
}

QMenu* IDOSFluidRockTableTreeProvider::createContextMenu(const QList<IDOSTreeItem*>& selectedItems)
{
    if (item() == nullptr || item()->model() == nullptr)
    {
        return nullptr;
    }
    QMenu* menu = new QMenu();
    menu->setProperty("treeSession", item()->model()->sessionId());
    const QList<IDOSTreeItem*> targets = selectedItems.isEmpty() ? QList<IDOSTreeItem*>{item()} : selectedItems;
    if (isGroup())
    {
        if (selectedItems.size() <= 1 && caseId().isEmpty())
        {

            if (key() == QStringLiteral("data.fluids") || key() == QStringLiteral("data.rockPhysics"))
            {
                const bool isFluidCategory = key() == QStringLiteral("data.fluids");
                const QString actionText =
                    isFluidCategory ? tr("Import Fluid Tables...") : tr("Import Rock Physics Functions...");
                QAction* import = new QAction(actionText, menu);
                menu->addAction(import);
                import->setObjectName(isFluidCategory ? QStringLiteral("importFluidTablesAction")
                                                      :
                                                      QStringLiteral("importRockPhysicsTablesAction"));
                import->setProperty("category",
                                    isFluidCategory ? QStringLiteral("fluid") : QStringLiteral("rockPhysics"));
                connect(import, &QAction::triggered, this, &IDOSFluidRockTableTreeProvider::onImportFluidRockTables);
                if (isFluidCategory)
                {
                    QAction* catalog = new QAction(tr("Import Fluid Catalog..."), menu);
                    menu->addAction(catalog);
                    catalog->setObjectName(QStringLiteral("importFluidCatalogAction"));
                    catalog->setProperty("providerId", QStringLiteral("idos.fracture.materialCatalog.tsv"));
                    catalog->setProperty("dataTypeId", QStringLiteral("idos.fracture.fluidCatalog"));
                    catalog->setProperty("fileFilter", tr("Fluid Catalog (*.txt *.TXT);;All Files (*)"));
                    connect(catalog, &QAction::triggered, this, &IDOSFluidRockTableTreeProvider::onImportDataset);
                }
            }
        }
        return menu;
    }
    {
        const QStringList deletableTypes = {QStringLiteral("idos.microseismic.set"),
                                            QStringLiteral("idos.seismic.volume"),
                                            QStringLiteral("idos.fracture.schedule"),
                                            QStringLiteral("idos.naturalFracture.set"),
                                            QStringLiteral("idos.production.history"),
                                            QStringLiteral("idos.fracture.fluidCatalog"),
                                            QStringLiteral("idos.fracture.proppantCatalog"),
                                            QStringLiteral("idos.grid.surface"),
                                            QStringLiteral("idos.grid.boundary"),
                                            QStringLiteral("idos.fluid.table"),
                                            QStringLiteral("idos.rockPhysics.table")};
        QStringList ids;
        for (IDOSTreeItem* selected : targets)
        {
            IDOSDataObject* data = item()->model()->object(selected->provider()->dataId());
            if (data == nullptr || !selected->provider()->caseId().isEmpty() || selected->provider()->isGroup() ||
                !deletableTypes.contains(data->typeId()))
            {
                ids.clear();
                break;
            }
            ids.append(data->objectId());
        }
        if (!ids.isEmpty())
        {
            QAction* remove = new QAction(tr("Delete Data..."), menu);
            menu->addAction(remove);
            remove->setObjectName(QStringLiteral("deleteDataAction"));
            remove->setProperty("objectIds", ids);
            connect(remove, &QAction::triggered, this, &IDOSFluidRockTableTreeProvider::onDeleteData);
        }

        {
            if (caseId().isEmpty() && targets.size() >= 2)
            {
                QJsonArray records;
                for (IDOSTreeItem* selected : targets)
                {
                    IDOSTreeItemProvider* provider = selected->provider();
                    IDOSDataObject* object = provider->object();
                    const bool well = qobject_cast<IDOSWellObject*>(object) != nullptr;
                    const bool table = qobject_cast<IDOSFluidRockTableObject*>(object) != nullptr;
                    if ((!well && !table) || provider->isGroup() || !provider->caseId().isEmpty() ||
                        !provider->partKey().isEmpty() || !provider->itemKey().isEmpty())
                    {
                        records = QJsonArray();
                        break;
                    }
                    QJsonObject record;
                    record.insert(QStringLiteral("objectId"), object->objectId());
                    record.insert(QStringLiteral("role"),
                                  well ? QStringLiteral("case.well") : QStringLiteral("case.fluidRockTable"));
                    records.append(record);
                }
                if (records.size() == targets.size())
                {
                    QAction* attach = new QAction(tr("Add to Case..."), menu);
                    menu->addAction(attach);
                    attach->setObjectName(QStringLiteral("addToCaseAction"));
                    attach->setProperty("items", records);
                    connect(attach, &QAction::triggered, this, &IDOSFluidRockTableTreeProvider::onAddToCase);
                }
            }
        }
        IDOSDataObject* object = item()->model()->object(dataId());
        if (targets.size() == 1 && !isGroup() && partKey().isEmpty() && itemKey().isEmpty() && object != nullptr &&
            object->canClone())
        {
            QAction* rename = new QAction(tr("Rename..."), menu);
            menu->addAction(rename);
            rename->setObjectName(QStringLiteral("renameObjectAction"));
            connect(rename, &QAction::triggered, this, &IDOSFluidRockTableTreeProvider::onRenameObject);
        }
    }
    if (selectedItems.size() <= 1 && caseId().isEmpty())
    {
        QAction* add = new QAction(tr("Add to Simulation Case..."), menu);
        menu->addAction(add);
        add->setObjectName(QStringLiteral("addFluidRockTableToCaseAction"));
        connect(add, &QAction::triggered, this, &IDOSFluidRockTableTreeProvider::onAddFluidRockTableToCase);
    }
    return menu;
}

void IDOSFluidRockTableTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    IDOSFluidRockTableObject* table = qobject_cast<IDOSFluidRockTableObject*>(object());
    if (item == nullptr || table == nullptr)
    {
        return;
    }
    item->provider()->setName(table->name());
    item->provider()->setIcon(QIcon(table->category() == IDOSFluidRockTableObject::Category::Fluid
                                        ? QStringLiteral(":/images/gui-fluids.svg")
                                        :
                                        QStringLiteral(":/images/gui-rock-physics.svg")));
}

void IDOSFluidRockTableTreeProvider::onRenameObject()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->rename(id(), action->parent()->property("treeSession").toString(), dataId());
    }
    else if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->rename(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSFluidRockTableTreeProvider::onDeleteData()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->removeObjects(id(), action->parent()->property("treeSession").toString(),
                          action->property("objectIds").toStringList());
    }
}

void IDOSFluidRockTableTreeProvider::onImportFluidRockTables()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importFluidRockTables(id(), action->parent()->property("treeSession").toString(),
                                     action->property("category").toString());
    }
}

void IDOSFluidRockTableTreeProvider::onImportDataset()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->importDataset(id(), action->parent()->property("treeSession").toString(), action->text(),
                             action->property("fileFilter").toString(), action->property("providerId").toString(),
                             action->property("dataTypeId").toString());
    }
}

void IDOSFluidRockTableTreeProvider::onAddFluidRockTableToCase()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->addFluidRockTableToCase(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSFluidRockTableTreeProvider::onAddToCase()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSDataTreeItemModel* model = qobject_cast<IDOSDataTreeItemModel*>(item()->model()))
    {
        model->addToCase(id(), action->parent()->property("treeSession").toString(),
                         action->property("items").value<QJsonArray>());
    }
}
