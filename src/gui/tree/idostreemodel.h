#ifndef IDOS_TREE_MODEL_H
#define IDOS_TREE_MODEL_H

#include "idosobjecttreenode.h"
#include "idos_core.h"
#include <QAbstractItemModel>
#include <QHash>

class IDOSProject;
class IDOSDataObject;
class IDOSTreeItemProvider;

/**
 * @brief 树域：本模型实例展示 Project 中哪一类对象。
 *
 * 输入树与模型树是 IDOSTreeModel 的两个子类实例（IDOSInputTreeModel /
 * IDOSModelTreeModel），按类型注册表中的 IDOSObjectCategory 分流，
 * 对象不需要移动或复制——两棵树是同一份 objects 的两种过滤视图。
 */
enum class TreeScope
{
    All,     ///< 全部对象（不过滤）
    Input,   ///< 仅输入数据（按 inputGroup 归组显示）
    Model    ///< 仅模型对象
};

/**
 * @brief 数据对象树的 QAbstractItemModel（通用机制层）。
 *
 * 显示 Project 中的 IDOSDataObject（按 TreeScope 过滤），子节点结构由按
 * typeId 注册的 IDOSTreeItemProvider 决定（查表分发，基类不认识任何具体
 * 类型——内置 provider 由各子类注册，插件在运行时注册自己的）。
 *
 * 对象节点（IDOSObjectTreeNode）存 objectId，通过 Project 查询；
 * 值节点（IDOSValueTreeNode）为纯展示节点。
 *
 * 扩展方式：输入树/模型树行为差异（过滤谓词、分组规则、交互）在子类中
 * 重写 protected 虚函数实现，机制层零复制。
 */
class GUI_EXPORT IDOSTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit IDOSTreeModel(QObject* parent = nullptr);
    ~IDOSTreeModel() override;

    /** 设置工程，开始监听。 */
    void setProject(IDOSProject* project);

    /** 树域（Input / Model / All）。切换后整树重建。 */
    void setTreeScope(TreeScope scope);
    TreeScope treeScope() const { return m_treeScope; }

    /**
     * @brief 注册节点构建 provider，接管所有权。
     * @param provider 同 typeId 重复注册时覆盖旧的。
     */
    void registerItemProvider(IDOSTreeItemProvider* provider);

    /** 按 typeId 查找 provider；未注册返回 nullptr。 */
    IDOSTreeItemProvider* providerFor(const QString& typeId) const;

    /** 当前工程（未设置时为 nullptr）。 */
    IDOSProject* project() const { return m_project; }

    // ===== QAbstractItemModel 接口 =====

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /** 从 QModelIndex 取出对应的树节点（可能是对象节点、值节点或分组节点）。 */
    IDOSTreeNode* nodeFromIndex(const QModelIndex& index) const;

    /** 从 QModelIndex 取出对应的领域对象（仅对象节点有效，其余返回 nullptr）。 */
    IDOSDataObject* objectFromIndex(const QModelIndex& index) const;

protected:
    /** 域过滤谓词（子类可重定义，如输入树的固定目录骨架）。 */
    virtual bool shouldShow(IDOSDataObject* obj) const;

    /** Input 域下对象应归入的分组名（子类可重定义）；非 Input 域或未设分组返回空。 */
    virtual QString groupLabelOf(IDOSDataObject* obj) const;

    /** 重建指定容器的子树（容器子对象增删时调用）。 */
    void rebuildContainerSubtree(const QString& containerObjectId);

private slots:
    void onObjectAdded(const QString& objectId);
    void onObjectRemoved(const QString& objectId);
    void onObjectChanged(const QString& objectId);

private:
    void rebuildTree();
    IDOSObjectTreeNode* findNodeByObjectId(const QString& objectId) const;
    IDOSObjectTreeNode* findNodeByObjectId(IDOSTreeNode* branch, const QString& objectId) const;

    /** 为对象节点挂接 provider 构建的子树（不带行操作，用于初次插入）。 */
    void buildObjectSubtree(IDOSObjectTreeNode* node, IDOSDataObject* obj);

    /** 整体重建对象节点的子树（带 beginRemoveRows/beginInsertRows）。 */
    void rebuildSubtree(IDOSObjectTreeNode* node, IDOSDataObject* obj);

    /** 删除对象后重查所有对象子树：被引用它的容器子树需重建（影子节点清灰）。 */
    void refreshReferencingSubtrees();
    void refreshNeedsRebuild(IDOSTreeNode* branch);

    IDOSProject* m_project;
    IDOSObjectTreeNode* m_rootNode;
    TreeScope m_treeScope = TreeScope::All;
    /** Input 域的分组节点缓存（分组名 → 节点），节点父为根。 */
    QHash<QString, IDOSObjectTreeNode*> m_groupNodes;
    QHash<QString, IDOSTreeItemProvider*> m_providers;   // model 持有并析构
};

#endif // IDOS_TREE_MODEL_H
