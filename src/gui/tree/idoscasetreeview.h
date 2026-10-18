#ifndef IDOS_CASE_TREE_VIEW_H
#define IDOS_CASE_TREE_VIEW_H

#include "idostreeview.h"

class IDOSCaseTreeItemModel;

class GUI_EXPORT IDOSCaseTreeView : public IDOSTreeView
{
    Q_OBJECT

  public:
    explicit IDOSCaseTreeView(QWidget* parent = nullptr);
    void setModel(QAbstractItemModel* model) override;
    IDOSCaseTreeItemModel* caseModel() const;
};

#endif
