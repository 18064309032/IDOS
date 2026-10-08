#ifndef IDOS_TREE_VIEW_H
#define IDOS_TREE_VIEW_H

#include <QSet>
#include <QString>
#include <QTreeView>

#include "idos_gui.h"

class IDOSTreeMenuProvider;
class QMenu;

/**
 * @brief 通用树视图。
 *
 * 基于 QTreeView，支持 MenuProvider 注入。
 * contextMenuEvent 交给 MenuProvider 创建并弹出菜单。
 *
 * 展开状态保持：模型对对象分支做增量重建（摘除旧节点再插入新节点）时，
 * 视图在 rowsAboutToBeRemoved 中按节点稳定键（IDOSTreeNode::nodeKey）
 * 快照被删子树的展开路径，在 rowsInserted 中按路径恢复。
 * 换工程（modelReset）会清空快照，新工程按全折叠处理。
 */
class GUI_EXPORT IDOSTreeView : public QTreeView
{
    Q_OBJECT

  public:
    explicit IDOSTreeView(QWidget* parent = nullptr);
    ~IDOSTreeView() override;

    /** 设置右键菜单提供者。设为 nullptr 则禁用自定义菜单。 */
    void setMenuProvider(IDOSTreeMenuProvider* provider);

    /** 重写：挂载模型信号以实现分支重建时的展开状态保持。 */
    void setModel(QAbstractItemModel* model) override;

  signals:
    /** 菜单即将显示（扩展点，插件可追加 Action）。 */
    void contextMenuAboutToShow(QMenu* menu);

  protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

  private slots:
    void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
    void onRowsInserted(const QModelIndex& parent, int first, int last);
    void onModelAboutToBeReset();

  private:
    /** 从根到该 index 的稳定键路径（跳过无稳定键的中间节点）。 */
    QString nodePath(const QModelIndex& index) const;
    /** 递归快照子树中所有已展开节点的路径。 */
    void collectExpandedPaths(const QModelIndex& index);
    /** 递归恢复：路径命中快照的节点重新展开并消费该快照项（先父后子顺序遍历）。 */
    void restoreExpandedPaths(const QModelIndex& index);

    IDOSTreeMenuProvider* m_menuProvider;
    QSet<QString> m_pendingExpandedPaths;
};

#endif // IDOS_TREE_VIEW_H
