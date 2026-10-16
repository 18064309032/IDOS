#include <QIcon>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "data/grid/idosgrid.h"
#include "data/grid/idosgridfilter.h"
#include "data/grid/idosgridproperty.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "tree/idostreeitem.h"

#include "idosgridtreeprovider.h"

IDOSGridTreeProvider::IDOSGridTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeItemProvider(data)
{
}

IDOSTreeItem* IDOSGridTreeProvider::addItem(IDOSTreeItem* parent, const QString& key, const QString& title,
                                                    IDOSDataObject* data)
{
    IDOSTreeItem* item = parent->findChild(key);
    if (item == nullptr)
    {
        item = new IDOSTreeItem(new IDOSGridTreeProvider(data != nullptr ? data : dataSource()));
        parent->appendChild(item);
    }

    item->provider()->setKey(key);
    item->provider()->setName(title);
    item->provider()->setDataId(parent->provider()->dataId());
    item->provider()->setCaseId(parent->provider()->caseId());
    item->provider()->setPartKey(key);
    return item;
}

void IDOSGridTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(dataSource());
    if (grid == nullptr || item == nullptr ||
        item->provider()->isGroup())
    {
        return;
    }

    item->provider()->setName(grid->name());
    item->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-grid.svg")));
    item->provider()->setCheckable(true);
    IDOSTreeItem* staticGroup = addItem(item, QStringLiteral("grid.staticProperties"), QObject::tr("Static Properties"));
    staticGroup->provider()->setGroup(true);
    IDOSTreeItem* regions = addItem(item, QStringLiteral("grid.regions"), QObject::tr("Regions"));
    regions->provider()->setGroup(true);
    IDOSTreeItem* dynamicGroup = addItem(item, QStringLiteral("grid.dynamicProperties"), QObject::tr("Dynamic Properties"));
    dynamicGroup->provider()->setGroup(true);
    IDOSTreeItem* filters = addItem(item, QStringLiteral("grid.filters"), QObject::tr("Filters"));
    filters->provider()->setGroup(true);
    const IDOSProject* project = qobject_cast<const IDOSProject*>(grid->parent());
    if (project == nullptr)
    {

        return;
    }
    const QStringList regionKeywords = {QStringLiteral("SATNUM"), QStringLiteral("ROCKNUM"),
                                       QStringLiteral("PVTNUM"), QStringLiteral("EQLNUM"), QStringLiteral("FIPNUM")};
    for (IDOSDataObject* candidate : project->objects())
    {
        IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(candidate);
        if (filter != nullptr && filter->gridId() == grid->objectId())
        {
            IDOSTreeItem* leaf = addItem(filters, QStringLiteral("filter:") + filter->objectId(), filter->name(), filter);
            leaf->provider()->setDataId(filter->objectId());
            leaf->provider()->setReference(true);
            leaf->provider()->setPartKey(QString());
            leaf->provider()->setCheckable(true);
        }
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(candidate);
        if (property == nullptr || property->gridId() != grid->objectId())
        {
            continue;
        }
        IDOSTreeItem* category = property->kind() == IDOSGridProperty::Kind::Dynamic ? dynamicGroup
                                : regionKeywords.contains(property->keyword().toUpper()) ? regions : staticGroup;
        const QString propertyName = property->name();
        IDOSTreeItem* leaf = addItem(category, QStringLiteral("property:") + property->objectId(), propertyName, property);
        leaf->provider()->setDataId(property->objectId());
        leaf->provider()->setReference(true);
        leaf->provider()->setPartKey(QString());
        leaf->provider()->setCheckable(true);
    }

}

bool IDOSGridTreeProvider::dependsOn(const QStringList& dataIds) const
{
    return !dataIds.isEmpty();
}
