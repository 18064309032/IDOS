#ifndef IDOS_TREE_MENU_PROVIDER_H
#define IDOS_TREE_MENU_PROVIDER_H

#include "idos_gui.h"
#include <QObject>
#include <QMenu>

class QTreeView;

/**
 * @brief 树右键菜单工厂基类。
 *
 * 子类实现 createContextMenu()，返回 QMenu*（可为 nullptr）。
 * Provider 只负责造菜单，不负责弹出——弹出由 TreeView 的 contextMenuEvent 处理。
 */
class GUI_EXPORT IDOSTreeMenuProvider : public QObject
{
    Q_OBJECT

public:
    explicit IDOSTreeMenuProvider(QObject* parent = nullptr);
    ~IDOSTreeMenuProvider() override;

public:
    /** 创建菜单（纯虚函数，子类实现）。返回 nullptr 或空菜单则不弹。 */
    virtual QMenu* createContextMenu() = 0;
};

#endif // IDOS_TREE_MENU_PROVIDER_H
