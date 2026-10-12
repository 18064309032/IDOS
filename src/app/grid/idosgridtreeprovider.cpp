#include <QIcon>
#include <QObject>
#include <QString>
#include <QStringList>

#include "idosdataobject.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idostreeitem.h"

#include "idosgridtreeprovider.h"

IDOSGridTreeProvider::IDOSGridTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeProvider(data)
{
}

IDOSTreeItem* IDOSGridTreeProvider::addItem(IDOSTreeItem* parent, const QString& key, const QString& title)
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

void IDOSGridTreeProvider::populate(IDOSTreeItem* item)
{
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(dataSource());
    if (grid == nullptr || item == nullptr)
    {
        return;
    }
    item->clearChildren();
    item->setName(grid->name());
    item->setIcon(QIcon(QStringLiteral(":/images/gui-grid.svg")));
    item->setCheckable(true);
    item->setChecked(grid->isVisible());
    IDOSTreeItem* staticGroup = addItem(item, QStringLiteral("grid.staticProperties"), QObject::tr("Static Properties"));
    staticGroup->setGroup(true);
    IDOSTreeItem* regions = addItem(item, QStringLiteral("grid.regions"), QObject::tr("Regions"));
    regions->setGroup(true);
    IDOSTreeItem* dynamicGroup = addItem(item, QStringLiteral("grid.dynamicProperties"), QObject::tr("Dynamic Properties"));
    dynamicGroup->setGroup(true);
    const IDOSProject* project = qobject_cast<const IDOSProject*>(grid->parent());
    if (project == nullptr)
    {
        return;
    }
    const QStringList regionKeywords = {QStringLiteral("SATNUM"), QStringLiteral("ROCKNUM"),
                                       QStringLiteral("PVTNUM"), QStringLiteral("EQLNUM"), QStringLiteral("FIPNUM")};
    for (IDOSDataObject* candidate : project->objects())
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(candidate);
        if (property == nullptr || property->gridId() != grid->objectId())
        {
            continue;
        }
        IDOSTreeItem* category = property->kind() == IDOSGridProperty::Kind::Dynamic ? dynamicGroup
                                : regionKeywords.contains(property->keyword().toUpper()) ? regions : staticGroup;
        const QString propertyName = property->kind() == IDOSGridProperty::Kind::Dynamic
                && !property->keyword().isEmpty()
            ? property->keyword()
            : property->name();
        IDOSTreeItem* leaf = addItem(category, QStringLiteral("property:") + property->objectId(), propertyName);
        leaf->setDataId(property->objectId());
        leaf->setReference(true);
        leaf->setPartKey(QString());
        leaf->setCheckable(true);
        leaf->setChecked(property->isVisible());
    }
}
