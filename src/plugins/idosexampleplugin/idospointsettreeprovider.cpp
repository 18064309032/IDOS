#include <QAction>
#include <QJsonObject>
#include <QMenu>
#include <QObject>
#include <QVariant>
#include <QVector>
#include <QVector3D>

#include "idospointset.h"
#include "tree/idostreeitem.h"

#include "idospointsettreeprovider.h"

IDOSPointSetTreeProvider::IDOSPointSetTreeProvider(IDOSDataObject* data)
    :
    QObject(nullptr)
    , IDOSTreeItemProvider(data)
{
}

QMenu* IDOSPointSetTreeProvider::createContextMenu()
{
    if (dataSource() == nullptr || item() == nullptr)
    {
        return nullptr;
    }
    QMenu* menu = new QMenu();
    QAction* action = menu->addAction(tr("Delete Point Set"));
    action->setProperty("dataId", dataSource()->objectId());
    action->setObjectName(QStringLiteral("deletePointSetAction"));
    QJsonObject arguments;
    arguments.insert(QStringLiteral("objectId"), dataSource()->objectId());
    action->setProperty("operation", QStringLiteral("command.execute"));
    action->setProperty("commandId", QStringLiteral("pointset.delete"));
    action->setProperty("commandArguments", arguments);
    return menu;
}

void IDOSPointSetTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    IDOSPointSet* data = qobject_cast<IDOSPointSet*>(dataSource());
    if (data == nullptr || item == nullptr)
    {
        return;
    }

    item->provider()->setName(data->name());
    item->provider()->setCheckable(true);
    const QVector<QVector3D> points = data->points();
    for (int index = 0; index < points.size(); ++index)
    {
        const QVector3D point = points.at(index);
        const QString key = QString::fromLatin1("point:%1").arg(index);
        IDOSTreeItem* child = item->findChild(key);
        if (child == nullptr)
        {
            child = new IDOSTreeItem(new IDOSTreeItemProvider());
            item->appendChild(child);
        }
        child->provider()->setKey(QString::fromLatin1("point:%1").arg(index));
        child->provider()->setPartKey(QString::fromLatin1("pointset.point"));
        child->provider()->setItemKey(QString::number(index));
        child->provider()->setDataId(data->objectId());
        child->provider()->setCaseId(item->provider()->caseId());
        child->provider()->setName(QObject::tr("Point %1: (%2, %3, %4)")
                           .arg(index + 1).arg(point.x()).arg(point.y()).arg(point.z()));
    }

}
