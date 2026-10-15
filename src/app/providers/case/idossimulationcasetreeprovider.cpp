#include <QObject>
#include <QVariant>

#include "data/case/idossimulationcaseobject.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "providers/idostreemetadata.h"
#include "providers/idostreeregistry.h"
#include "tree/idostreeitem.h"

#include "idossimulationcasetreeprovider.h"

IDOSSimulationCaseTreeProvider::IDOSSimulationCaseTreeProvider(IDOSDataObject* data, IDOSTreeRegistry* registry)
    :
    IDOSTreeProvider(data)
    , m_registry(registry)
{
}

IDOSTreeItem* IDOSSimulationCaseTreeProvider::addItem(IDOSTreeItem* parent, const QString& key, const QString& title)
{
    IDOSTreeItem* item = new IDOSTreeItem();
    item->setKey(key);
    item->setName(title);
    item->setDataId(parent->dataId());
    item->setCaseId(parent->caseId());
    item->setPartKey(key);
    parent->appendChild(item);
    return item;
}

bool IDOSSimulationCaseTreeProvider::dependsOn(const QStringList& dataIds) const
{
    Q_UNUSED(dataIds);
    // References and derived property lists can change when any project data changes.
    return true;
}

void IDOSSimulationCaseTreeProvider::populate(IDOSTreeItem* item)
{
    IDOSSimulationCaseObject* caseObject = qobject_cast<IDOSSimulationCaseObject*>(dataSource());
    if (caseObject == nullptr || item == nullptr)
    {
        return;
    }
    item->clearChildren();
    item->setName(caseObject->name());
    item->setCaseId(caseObject->objectId());
    item->setIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
    addItem(item, QStringLiteral("case.runSettings"), QObject::tr("Run Settings"))->setGroup(true);
    IDOSTreeItem* grids = addItem(item, QStringLiteral("case.grids"), QObject::tr("Grids"));
    grids->setGroup(true);
    IDOSTreeItem* fluidRock = addItem(item, QStringLiteral("case.fluidRock"),
                                      QObject::tr("Fluid and Rock Properties"));
    fluidRock->setGroup(true);
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
            IDOSTreeItem* table = addItem(
                fluidRock, QStringLiteral("case.curveTable:%1:%2").arg(keyword).arg(tableIndex),
                QObject::tr("%1 — Table %2").arg(keyword).arg(tableIndex));
            table->setItemKey(keyword);
            table->setPartKey(QString::number(tableIndex));
        }
    }
    addItem(item, QStringLiteral("case.initialState"), QObject::tr("Initial State"))->setGroup(true);
    IDOSTreeItem* wells = addItem(item, QStringLiteral("case.schedule"), QObject::tr("Wells and Development Plan"));
    wells->setGroup(true);
    addItem(item, QStringLiteral("case.outputSettings"), QObject::tr("Output Settings"))->setGroup(true);
    IDOSTreeItem* summaryResults = addItem(item, QStringLiteral("case.summaryResults"),
                                           QObject::tr("Summary Results"));
    summaryResults->setGroup(true);
    const IDOSProject* project = qobject_cast<const IDOSProject*>(caseObject->parent());
    for (const IDOSCaseItemRef& ref : refs)
    {
        if (ref.role() == QStringLiteral("case.gridProperty"))
        {
            continue;
        }
        IDOSDataObject* data = project != nullptr ? project->object(ref.objectId()) : nullptr;
        IDOSTreeItem* parent = ref.role() == QStringLiteral("case.grid") ? grids
                             : ref.role() == QStringLiteral("case.fluidRockTable") ? fluidRock
                             : ref.role() == QStringLiteral("case.summaryResults") ? summaryResults : wells;
        IDOSTreeItem* reference = addItem(parent, ref.role() + QLatin1Char(':') + ref.objectId(),
                                        data != nullptr ? data->name() : QObject::tr("Missing object: %1").arg(ref.objectId()));
        reference->setDataId(ref.objectId());
        reference->setReference(true);
        reference->setPartKey(ref.partKey());
        reference->setItemKey(ref.itemKey());
        reference->setCheckable(data != nullptr);
        reference->setChecked(data != nullptr && data->isVisible());
        if (data != nullptr && m_registry != nullptr)
        {
            bool cycle = false;
            for (const IDOSTreeItem* ancestor = reference->parent(); ancestor != nullptr; ancestor = ancestor->parent())
            {
                if (!ancestor->isGroup() && ancestor->dataId() == data->objectId())
                {
                    cycle = true;
                    break;
                }
            }
            if (cycle)
            {
                continue;
            }
            const IDOSTreeMetadata* metadata = m_registry->metadata(data->typeId());
            IDOSTreeProvider* provider = metadata != nullptr ? metadata->create(data) : nullptr;
            if (provider != nullptr)
            {
                reference->setProvider(provider);
                provider->populate(reference);
            }
        }
    }
    for (const QString& name : caseObject->pendingWellNames())
    {
        IDOSTreeItem* unresolved = addItem(wells, QStringLiteral("unresolved:") + name, QObject::tr("%1 (unresolved)").arg(name));
        unresolved->setGroup(true);
    }
}
