#include <QObject>
#include <QVariant>

#include "data/case/idossimulationcaseobject.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "providers/idostreeitemprovidermetadata.h"
#include "providers/idostreeitemproviderregistry.h"
#include "tree/idostreeitem.h"
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
        item = new IDOSTreeItem(new IDOSTreeItemProvider());
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
    IDOSSimulationCaseObject* caseObject = qobject_cast<IDOSSimulationCaseObject*>(dataSource());
    if (caseObject == nullptr || item == nullptr)
    {
        return;
    }

    item->provider()->setName(caseObject->name());
    item->provider()->setCaseId(caseObject->objectId());
    item->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
    addItem(item, QStringLiteral("case.runSettings"), QObject::tr("Run Settings"))->provider()->setGroup(true);
    IDOSTreeItem* grids = addItem(item, QStringLiteral("case.grids"), QObject::tr("Grids"));
    grids->provider()->setGroup(true);
    IDOSTreeItem* fluidRock = addItem(item, QStringLiteral("case.fluidRock"),
                                      QObject::tr("Fluid and Rock Properties"));
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
            IDOSTreeItem* table = addItem(
                fluidRock, QStringLiteral("case.curveTable:%1:%2").arg(keyword).arg(tableIndex),
                QObject::tr("%1 — Table %2").arg(keyword).arg(tableIndex));
            table->provider()->setItemKey(keyword);
            table->provider()->setPartKey(QString::number(tableIndex));
        }
    }
    addItem(item, QStringLiteral("case.initialState"), QObject::tr("Initial State"))->provider()->setGroup(true);
    IDOSTreeItem* wells = addItem(item, QStringLiteral("case.schedule"), QObject::tr("Wells and Development Plan"));
    wells->provider()->setGroup(true);
    addItem(item, QStringLiteral("case.outputSettings"), QObject::tr("Output Settings"))->provider()->setGroup(true);
    IDOSTreeItem* summaryResults = addItem(item, QStringLiteral("case.summaryResults"),
                                           QObject::tr("Summary Results"));
    summaryResults->provider()->setGroup(true);
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
        if (reference != nullptr && reference->provider()->dataSource() != data)
        {
            parent->removeChild(reference);
            reference = nullptr;
        }
        IDOSTreeItemProvider* provider = reference != nullptr ? reference->provider() : data != nullptr && m_registry != nullptr
            ? m_registry->createProvider(data) : nullptr;
        if (provider == nullptr)
        {
            provider = new IDOSTreeItemProvider();
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
        IDOSTreeItem* unresolved = addItem(wells, QStringLiteral("unresolved:") + name, QObject::tr("%1 (unresolved)").arg(name));
        unresolved->provider()->setGroup(true);
    }

}
