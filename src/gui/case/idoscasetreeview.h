#ifndef IDOS_CASE_TREE_VIEW_H
#define IDOS_CASE_TREE_VIEW_H

#include "tree/idostreeview.h"

/**
 * @brief 工况树视图。
 */
class GUI_EXPORT IDOSCaseTreeView : public IDOSTreeView
{
    Q_OBJECT

public:
    explicit IDOSCaseTreeView(QWidget* parent = nullptr);
    ~IDOSCaseTreeView() override;
};

#endif // IDOS_CASE_TREE_VIEW_H
