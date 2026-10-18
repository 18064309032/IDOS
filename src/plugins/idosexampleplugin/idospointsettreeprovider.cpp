#include <QAction>
#include <QJsonObject>
#include <QMenu>
#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QVector>
#include <QVector3D>

#include "idospointsetobject.h"
#include "tree/idostreeitem.h"
#include "tree/idostreemodel.h"

#include "idospointsettreeprovider.h"

IDOSPointSetTreeProvider::IDOSPointSetTreeProvider(IDOSDataObject* data)
    :
    IDOSTreeItemProvider(data)
{
}

QMenu* IDOSPointSetTreeProvider::createContextMenu(const QList<IDOSTreeItem*>& selectedItems)
{
    if (object() == nullptr || item() == nullptr || item()->model() == nullptr)
    {
        return nullptr;
    }
    QMenu* menu = new QMenu();
    menu->setProperty("treeSession", item()->model()->sessionId());
    const QList<IDOSTreeItem*> targets = selectedItems.isEmpty() ? QList<IDOSTreeItem*>{item()} : selectedItems;
    if (targets.size() > 1)
    {
        return menu;
    }
    QAction* action = menu->addAction(tr("Delete Point Set"));
    action->setProperty("dataId", object()->objectId());
    action->setObjectName(QStringLiteral("deletePointSetAction"));
    connect(action, &QAction::triggered, this, &IDOSPointSetTreeProvider::onDeletePointSet);
    return menu;
}

void IDOSPointSetTreeProvider::refresh()
{
    IDOSTreeItem* item = this->item();
    IDOSPointSetObject* data = qobject_cast<IDOSPointSetObject*>(object());
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
        child->provider()->setName(
            QObject::tr("Point %1: (%2, %3, %4)").arg(index + 1).arg(point.x()).arg(point.y()).arg(point.z()));
    }
}

void IDOSPointSetTreeProvider::onDeletePointSet()
{
    const QAction* action = qobject_cast<const QAction*>(sender());
    IDOSTreeModel* model = item() != nullptr ? item()->model() : nullptr;
    if (action == nullptr || model == nullptr || model->sessionId().isEmpty() ||
        action->parent()->property("treeSession").toString() != model->sessionId())
    {
        return;
    }
    emit deletePointSetRequested(dataId(), model->sessionId());
}
