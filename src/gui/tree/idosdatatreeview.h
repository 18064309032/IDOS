#ifndef IDOS_DATA_TREE_VIEW_H
#define IDOS_DATA_TREE_VIEW_H

#include "idostreeview.h"

class IDOSDataTreeItemModel;

class GUI_EXPORT IDOSDataTreeView : public IDOSTreeView
{
    Q_OBJECT

  public:
    explicit IDOSDataTreeView(QWidget* parent = nullptr);
    void setModel(QAbstractItemModel* model) override;
    IDOSDataTreeItemModel* dataModel() const;
};

#endif
