#ifndef IDOS_TREE_NODE_H
#define IDOS_TREE_NODE_H

#include "idos_gui.h"
#include <QVector>
#include <QString>
#include <QIcon>

/**
 * @brief 树节点基类。
 *
 * 实现树结构的父子关系管理、名称和图标。
 * 不继承 QObject，值语义，由外部 QAbstractItemModel 持有所有权。
 * 子类可扩展携带具体业务数据（如 IDOSDataObject*）。
 */
class GUI_EXPORT IDOSTreeNode
{
public:
    explicit IDOSTreeNode(IDOSTreeNode* parent = nullptr);
    virtual ~IDOSTreeNode();

    // ===== 树结构 =====

    /** 父节点，根节点返回 nullptr。 */
    IDOSTreeNode* parent() const;

    /** 子节点数量。 */
    int childCount() const;

    /** 按索引获取子节点。 */
    IDOSTreeNode* child(int row) const;

    /** 自身在父节点中的索引。 */
    int row() const;

    /** 是否为分组节点（用于 Model Tree 的 Grid 组、Well 组等）。 */
    bool isGroup() const;
    void setGroup(bool group);

    // ===== 显示字段 =====

    /** 显示名称。 */
    QString name() const;
    void setName(const QString& name);

    /** 图标。 */
    QIcon icon() const;
    void setIcon(const QIcon& icon);

    // ===== 修改 =====

    /** 追加子节点，接管所有权。 */
    void appendChild(IDOSTreeNode* child);

    /** 按索引移除子节点，释放所有权。 */
    void removeChild(int row);

    /** 清空所有子节点。 */
    void clearChildren();

protected:
    QVector<IDOSTreeNode*> m_children;
    IDOSTreeNode* m_parent;
    QString m_name;
    QIcon m_icon;
    bool m_isGroup;
};

#endif // IDOS_TREE_NODE_H
