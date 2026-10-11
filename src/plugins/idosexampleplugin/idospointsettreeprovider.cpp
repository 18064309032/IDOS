#include <QObject>
#include <QVector>
#include <QVector3D>

#include "idospointset.h"
#include "idostreeitem.h"

#include "idospointsettreeprovider.h"

IDOSPointSetTreeProvider::IDOSPointSetTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeProvider(data)
{
}

void IDOSPointSetTreeProvider::populate(IDOSTreeItem* item)
{
    IDOSPointSet* data = qobject_cast<IDOSPointSet*>(dataSource());
    if (data == nullptr || item == nullptr)
    {
        return;
    }
    item->clearChildren();
    item->setName(data->name());
    item->setCheckable(true);
    item->setChecked(data->isVisible());
    const QVector<QVector3D> points = data->points();
    for (int index = 0; index < points.size(); ++index)
    {
        const QVector3D point = points.at(index);
        IDOSTreeItem* child = new IDOSTreeItem();
        child->setKey(QString::fromLatin1("point:%1").arg(index));
        child->setPartKey(QString::fromLatin1("pointset.point"));
        child->setItemKey(QString::number(index));
        child->setDataId(data->objectId());
        child->setCaseId(item->caseId());
        child->setName(QObject::tr("Point %1: (%2, %3, %4)")
                           .arg(index + 1).arg(point.x()).arg(point.y()).arg(point.z()));
        item->appendChild(child);
    }
}
