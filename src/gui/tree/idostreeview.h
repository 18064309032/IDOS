#ifndef IDOS_TREE_VIEW_H
#define IDOS_TREE_VIEW_H

#include "idos_gui.h"
#include <QTreeView>

class IDOSTreeMenuProvider;

/**
 * @brief 通用树视图。
 *
 * 基于 QTreeView，支持 MenuProvider 注入。
 * contextMenuEvent 交给 MenuProvider 创建并弹出菜单。
 */
class GUI_EXPORT IDOSTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit IDOSTreeView(QWidget* parent = nullptr);
    ~IDOSTreeView() override;

    /** 设置右键菜单提供者。设为 nullptr 则禁用自定义菜单。 */
    void setMenuProvider(IDOSTreeMenuProvider* provider);

signals:
    /** 菜单即将显示（扩展点，插件可追加 Action）。 */
    void contextMenuAboutToShow(QMenu* menu);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    IDOSTreeMenuProvider* m_menuProvider;
};

#endif // IDOS_TREE_VIEW_H
