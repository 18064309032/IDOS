#include "tree/idosdatatreeitemmodel.h"

#include "idosdatatreeview.h"

IDOSDataTreeView::IDOSDataTreeView(QWidget* parent)
    :
    IDOSTreeView(parent)
{
    setBlankMenuEnabled(false);
}

void IDOSDataTreeView::setModel(QAbstractItemModel* model)
{
    if (model != nullptr && qobject_cast<IDOSDataTreeItemModel*>(model) == nullptr)
    {
        return;
    }
    IDOSTreeView::setModel(model);
}

IDOSDataTreeItemModel* IDOSDataTreeView::dataModel() const
{
    return qobject_cast<IDOSDataTreeItemModel*>(model());
}
