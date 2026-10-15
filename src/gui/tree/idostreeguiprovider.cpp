#include "idosdataobject.h"
#include "idosproject.h"
#include "tree/idostree.h"
#include "tree/idostreeitem.h"

#include "idostreeguiprovider.h"

IDOSTreeGuiProvider::IDOSTreeGuiProvider(QObject* parent)
    :
    QObject(parent)
{
}

IDOSTreeGuiProvider::~IDOSTreeGuiProvider()
{
}

bool IDOSTreeGuiProvider::isCheckable(const IDOSTreeItem* item) const
{
    return item != nullptr && item->isCheckable();
}

Qt::CheckState IDOSTreeGuiProvider::checkState(const IDOSTreeItem* item, const IDOSTreeView* view) const
{
    return isChecked(item, view) ? Qt::Checked : Qt::Unchecked;
}

bool IDOSTreeGuiProvider::isChecked(const IDOSTreeItem* item, const IDOSTreeView* view) const
{
    Q_UNUSED(view);
    if (item != nullptr && item->tree() != nullptr && item->tree()->project() != nullptr)
    {
        IDOSDataObject* data = item->tree()->project()->object(item->dataId());
        if (data != nullptr)
        {
            return data->isVisible();
        }
    }
    return item != nullptr && item->isChecked();
}

bool IDOSTreeGuiProvider::setChecked(IDOSTreeItem* item, bool checked, IDOSTreeView* view)
{
    Q_UNUSED(view);
    if (item == nullptr || item->tree() == nullptr || item->tree()->project() == nullptr)
    {
        return false;
    }
    IDOSDataObject* data = item->tree()->project()->object(item->dataId());
    if (data == nullptr)
    {
        return false;
    }
    data->setVisible(checked);
    item->setChecked(checked);
    emit checkStateChanged();
    return true;
}

QMimeData* IDOSTreeGuiProvider::createMimeData(const QList<IDOSTreeItem*>& items, const IDOSTreeView* view) const
{
    Q_UNUSED(items);
    Q_UNUSED(view);
    return nullptr;
}

bool IDOSTreeGuiProvider::canCut(const QMimeData* data, const IDOSTreeView* view) const
{
    Q_UNUSED(data);
    Q_UNUSED(view);
    return false;
}

bool IDOSTreeGuiProvider::canPaste(const QMimeData* data, const IDOSTreeItem* target,
                                  Qt::DropAction action, const IDOSTreeView* view) const
{
    Q_UNUSED(data);
    Q_UNUSED(target);
    Q_UNUSED(action);
    Q_UNUSED(view);
    return false;
}

bool IDOSTreeGuiProvider::paste(const QMimeData* data, IDOSTreeItem* target,
                               Qt::DropAction action, IDOSTreeView* view)
{
    Q_UNUSED(data);
    Q_UNUSED(target);
    Q_UNUSED(action);
    Q_UNUSED(view);
    return false;
}

void IDOSTreeGuiProvider::populateSelectionMenu(QMenu* menu, IDOSTreeView* view)
{
    Q_UNUSED(menu);
    Q_UNUSED(view);
}
