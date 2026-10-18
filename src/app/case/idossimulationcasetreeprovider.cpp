#include <QAction>
#include <QMenu>
#include <QObject>
#include <QVariant>
#include <QVariantMap>

#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/grid/idosgridfilterobject.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "tree/idoscasetreeitemmodel.h"
#include "tree/idosdatatreeitemmodel.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeitemprovidermetadata.h"
#include "tree/idostreeitemproviderregistry.h"
#include "tree/idostreemodel.h"

#include "idossimulationcasetreeprovider.h"

IDOSSimulationCaseTreeProvider::IDOSSimulationCaseTreeProvider(IDOSDataObject* data,
                                                               IDOSTreeItemProviderRegistry* registry)
    :
    IDOSTreeItemProvider(data)
    , m_registry(registry)
{
}

IDOSTreeItem* IDOSSimulationCaseTreeProvider::addItem(IDOSTreeItem* parent, const QString& key, const QString& title)
{
    IDOSTreeItem* item = parent->findChild(key);
    if (item == nullptr)
    {
        item = new IDOSTreeItem(new IDOSSimulationCaseTreeProvider(object(), m_registry));
        parent->appendChild(item);
    }

    item->provider()->setKey(key);
    item->provider()->setName(title);
    item->provider()->setDataId(parent->provider()->dataId());
    item->provider()->setCaseId(parent->provider()->caseId());
    item->provider()->setPartKey(key);
    return item;
}

bool IDOSSimulationCaseTreeProvider::dependsOn(const QStringList& dataIds) const
{
    Q_UNUSED(dataIds);
    // References and derived property lists can change when any project data changes.
    return true;
}

void IDOSSimulationCaseTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    IDOSSimulationCaseObject* caseObject = qobject_cast<IDOSSimulationCaseObject*>(object());
    if (caseObject == nullptr || item == nullptr || !partKey().isEmpty() || isGroup())
    {
        return;
    }

    item->provider()->setName(caseObject->name());
    item->provider()->setCaseId(caseObject->objectId());
    item->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
    addItem(item, QStringLiteral("case.runSettings"), QObject::tr("Run Settings"))->provider()->setGroup(true);
    IDOSTreeItem* grids = addItem(item, QStringLiteral("case.grids"), QObject::tr("Grids"));
    grids->provider()->setGroup(true);
    IDOSTreeItem* fluidRock = addItem(item, QStringLiteral("case.fluidRock"), QObject::tr("Fluid and Rock Properties"));
    fluidRock->provider()->setGroup(true);
    const QList<IDOSCaseItemRef> refs = caseObject->itemRefs();
    bool hasFluidRockReferences = false;
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.fluidRockTable"))
        {
            hasFluidRockReferences = true;
            break;
        }
    }
    if (!hasFluidRockReferences)
    {
        const QVariantList curveTables = caseObject->curveTables();
        for (const QVariant& curveTableValue : curveTables)
        {
            const QVariantMap curveTable = curveTableValue.toMap();
            const QString keyword = curveTable.value(QStringLiteral("keyword")).toString();
            const int tableIndex = curveTable.value(QStringLiteral("tableIndex")).toInt();
            if (keyword.isEmpty() || tableIndex < 1)
            {
                continue;
            }
            IDOSTreeItem* table =
                addItem(fluidRock, QStringLiteral("case.curveTable:%1:%2").arg(keyword).arg(tableIndex),
                        QObject::tr("%1 — Table %2").arg(keyword).arg(tableIndex));
            table->provider()->setItemKey(keyword);
            table->provider()->setPartKey(QString::number(tableIndex));
        }
    }
    addItem(item, QStringLiteral("case.initialState"), QObject::tr("Initial State"))->provider()->setGroup(true);
    IDOSTreeItem* wells = addItem(item, QStringLiteral("case.schedule"), QObject::tr("Wells and Development Plan"));
    wells->provider()->setGroup(true);
    addItem(item, QStringLiteral("case.outputSettings"), QObject::tr("Output Settings"))->provider()->setGroup(true);
    IDOSTreeItem* summaryResults = addItem(item, QStringLiteral("case.summaryResults"), QObject::tr("Summary Results"));
    summaryResults->provider()->setGroup(true);
    const IDOSProject* project = qobject_cast<const IDOSProject*>(caseObject->parent());
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.gridProperty"))
        {
            continue;
        }
        IDOSDataObject* data = project != nullptr ? project->object(ref.objectId()) : nullptr;
        IDOSTreeItem* parent = ref.role() == QStringLiteral("case.grid")             ? grids
                               : ref.role() == QStringLiteral("case.fluidRockTable") ? fluidRock
                               : ref.role() == QStringLiteral("case.summaryResults") ? summaryResults
                                                                                     : wells;
        bool cycle = false;
        for (const IDOSTreeItem* ancestor = parent; ancestor != nullptr; ancestor = ancestor->parent())
        {
            if (!ancestor->provider()->isGroup() && ancestor->provider()->dataId() == ref.objectId())
            {
                cycle = true;
                break;
            }
        }
        if (cycle)
        {
            continue;
        }
        const QString referenceKey = ref.role() + QLatin1Char(':') + ref.objectId();
        IDOSTreeItem* reference = parent->findChild(referenceKey);
        if (reference != nullptr && reference->provider()->object() != data)
        {
            parent->removeChild(reference);
            reference = nullptr;
        }
        IDOSTreeItemProvider* provider = reference != nullptr                       ? reference->provider()
                                         : data != nullptr && m_registry != nullptr ? m_registry->create(data)
                                                                                    : nullptr;
        if (provider == nullptr)
        {
            provider = new IDOSSimulationCaseTreeProvider(data, m_registry);
        }
        if (reference == nullptr)
        {
            reference = new IDOSTreeItem(provider);
            parent->appendChild(reference);
        }
        provider->setKey(ref.role() + QLatin1Char(':') + ref.objectId());
        provider->setName(data != nullptr ? data->name() : QObject::tr("Missing object: %1").arg(ref.objectId()));
        provider->setDataId(ref.objectId());
        provider->setCaseId(parent->provider()->caseId());
        provider->setReference(true);
        provider->setPartKey(ref.partKey());
        provider->setItemKey(ref.itemKey());
        provider->setCheckable(data != nullptr);
        provider->refresh();
    }
    for (const QString& name : caseObject->pendingWellNames())
    {
        IDOSTreeItem* unresolved =
            addItem(wells, QStringLiteral("unresolved:") + name, QObject::tr("%1 (unresolved)").arg(name));
        unresolved->provider()->setGroup(true);
    }
}

QMenu* IDOSSimulationCaseTreeProvider::createContextMenu(const QList<IDOSTreeItem*>& selectedItems)
{
    if (item() == nullptr || item()->model() == nullptr)
    {
        return nullptr;
    }
    QMenu* menu = new QMenu();
    menu->setProperty("treeSession", item()->model()->sessionId());
    const QList<IDOSTreeItem*> targets = selectedItems.isEmpty() ? QList<IDOSTreeItem*>{item()} : selectedItems;
    if (selectedItems.size() <= 1)
    {

        IDOSTreeModel* model = this->item() != nullptr ? this->item()->model() : nullptr;
        if (model == nullptr || model->sessionId().isEmpty())
        {
            return menu;
        }
        IDOSTreeItem* item = this->item();

        if (model->name() != QStringLiteral("case"))
        {
            return menu;
        }
        if (item != nullptr && item->provider()->key() == QStringLiteral("case.grids"))
        {
            QAction* import = new QAction(tr("Import Grid..."), menu);
            menu->addAction(import);
            import->setObjectName(QStringLiteral("importGridAction"));
            connect(import, &QAction::triggered, this, &IDOSSimulationCaseTreeProvider::onImportGrid);

            return menu;
        }
        if (item->provider()->isGroup())
        {
            if (item->provider()->key() == QStringLiteral("case.summaryResults"))
            {
                QAction* import = new QAction(tr("Import Summary Results..."), menu);
                menu->addAction(import);
                import->setObjectName(QStringLiteral("importSummaryResultsAction"));
                connect(import, &QAction::triggered, this, &IDOSSimulationCaseTreeProvider::onImportSummaryResults);
            }
            return menu;
        }
        if (item->provider()->key().startsWith(QStringLiteral("case.summaryResults:")))
        {
            QAction* show = new QAction(tr("View Summary Curves..."), menu);
            menu->addAction(show);
            show->setObjectName(QStringLiteral("viewSummaryCurvesAction"));
            connect(show, &QAction::triggered, this, &IDOSSimulationCaseTreeProvider::onViewSummaryResults);

            return menu;
        }
        if (item->provider()->key().startsWith(QStringLiteral("case.curveTable:")) ||
            item->provider()->key().startsWith(QStringLiteral("case.fluidRockTable:")))
        {
            QAction* viewCurves = new QAction(tr("View Property Curves..."), menu);
            menu->addAction(viewCurves);
            viewCurves->setObjectName(QStringLiteral("viewPropertyCurvesAction"));
            connect(viewCurves, &QAction::triggered, this, &IDOSSimulationCaseTreeProvider::onViewPropertyCurves);

            return menu;
        }
        IDOSCaseObject* caseObject = qobject_cast<IDOSCaseObject*>(model->object(item->provider()->dataId()));
        if (caseObject != nullptr && item->parent() == model->root())
        {
            QAction* generate = new QAction(tr("Generate Simulation Input..."), menu);
            menu->addAction(generate);
            generate->setObjectName(QStringLiteral("generateSimulationInputAction"));
            connect(generate, &QAction::triggered, this, &IDOSSimulationCaseTreeProvider::onGenerateInput);

            QAction* rename = new QAction(tr("Rename Case..."), menu);
            menu->addAction(rename);
            rename->setObjectName(QStringLiteral("renameCaseAction"));
            connect(rename, &QAction::triggered, this, &IDOSSimulationCaseTreeProvider::onRenameCase);

            QAction* remove = new QAction(tr("Delete Case..."), menu);
            menu->addAction(remove);
            remove->setObjectName(QStringLiteral("deleteCaseAction"));
            connect(remove, &QAction::triggered, this, &IDOSSimulationCaseTreeProvider::onDeleteCase);

            return menu;
        }
    }
    {
        IDOSDataObject* object = item()->model()->object(dataId());
        if (targets.size() == 1 && !isGroup() && partKey().isEmpty() && itemKey().isEmpty() && object != nullptr &&
            object->canClone() && qobject_cast<IDOSCaseObject*>(object) == nullptr &&
            qobject_cast<IDOSGridFilterObject*>(object) == nullptr)
        {
            QAction* rename = new QAction(tr("Rename..."), menu);
            menu->addAction(rename);
            rename->setObjectName(QStringLiteral("renameObjectAction"));
            connect(rename, &QAction::triggered, this, &IDOSSimulationCaseTreeProvider::onRenameObject);
        }
    }
    return menu;
}

void IDOSSimulationCaseTreeProvider::onRenameObject()
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

void IDOSSimulationCaseTreeProvider::onImportGrid()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importGrid(id(), action->parent()->property("treeSession").toString(), caseId());
    }
}

void IDOSSimulationCaseTreeProvider::onImportSummaryResults()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->importSummaryResults(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSSimulationCaseTreeProvider::onViewSummaryResults()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->viewSummaryResults(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSSimulationCaseTreeProvider::onViewPropertyCurves()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->viewPropertyCurves(id(), action->parent()->property("treeSession").toString(), caseId());
    }
}

void IDOSSimulationCaseTreeProvider::onGenerateInput()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->generateInput(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSSimulationCaseTreeProvider::onRenameCase()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->renameCase(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}

void IDOSSimulationCaseTreeProvider::onDeleteCase()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    if (action == nullptr || item() == nullptr)
    {
        return;
    }
    if (IDOSCaseTreeItemModel* model = qobject_cast<IDOSCaseTreeItemModel*>(item()->model()))
    {
        model->deleteCase(id(), action->parent()->property("treeSession").toString(), dataId());
    }
}
