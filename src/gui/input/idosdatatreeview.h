#ifndef IDOS_DATA_TREE_VIEW_H
#define IDOS_DATA_TREE_VIEW_H

#include "tree/idostreeview.h"

/**
 * @brief 数据树视图。
 */
class GUI_EXPORT IDOSDataTreeView : public IDOSTreeView
{
    Q_OBJECT

public:
    explicit IDOSDataTreeView(QWidget* parent = nullptr);
    ~IDOSDataTreeView() override;
};

#endif // IDOS_DATA_TREE_VIEW_H
