#include "tree/idoscasetreeitemmodel.h"

#include "idoscasetreeview.h"

IDOSCaseTreeView::IDOSCaseTreeView(QWidget* parent)
    :
    IDOSTreeView(parent)
{
    setBlankMenuEnabled(true);
}

void IDOSCaseTreeView::setModel(QAbstractItemModel* model)
{
    if (model != nullptr && qobject_cast<IDOSCaseTreeItemModel*>(model) == nullptr)
    {
        return;
    }
    IDOSTreeView::setModel(model);
}

IDOSCaseTreeItemModel* IDOSCaseTreeView::caseModel() const
{
    return qobject_cast<IDOSCaseTreeItemModel*>(model());
}
