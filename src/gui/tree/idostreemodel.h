#ifndef IDOS_TREE_MODEL_H
#define IDOS_TREE_MODEL_H

#include <QAbstractItemModel>
#include <QSet>
#include <QString>
#include <QStringList>

#include "idos_gui.h"
#include "idostreenode.h"

class IDOSDataObject;
class IDOSObjectTreeNode;
class IDOSProject;

/**
 * @brief 通用树模型基类。
 *
 * 只负责 Qt Model/View 适配、工程对象变化监听和根节点生命周期。
 */
class GUI_EXPORT IDOSTreeModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    explicit IDOSTreeModel(QObject* parent = nullptr);
    ~IDOSTreeModel() override;

    void setProject(IDOSProject* project);
    IDOSProject* project() const;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    IDOSTreeNode* nodeFromIndex(const QModelIndex& index) const;
    IDOSDataObject* objectFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromObjectId(const QString& objectId) const;

  protected:
    virtual void buildDefaultTree(IDOSTreeNode* rootNode);
    virtual bool shouldShowObject(const IDOSDataObject* object) const = 0;
    virtual void buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object) = 0;

    /**
     * 新加入工程且本树应显示的对象，其分支插入到哪个节点下。
     * 默认挂根节点；数据树按 provider 的 groupKey 挂到对应业务分组。
     * 返回 nullptr 表示挂根节点。
     */
    virtual IDOSTreeNode* parentNodeForNewObject(const IDOSDataObject* object) const;

    /**
     * 对象增删改后，刷新依赖该对象的其他分支（如工况树中引用了它的工况子树）。
     * @param objectRemoved 为 true 时对象已从工程移除（无法再查询其内容）。
     */
    virtual void refreshReferencingBranches(const QString& objectId, bool objectRemoved);

    /**
     * 批量事务结束后，一次性刷新所有受影响的引用分支。
     * 默认实现退化为逐个调用单对象版本，子类可重写为聚合算法
     * （如工况树：一批属性只重建每个受影响工况一次，避免 O(N²)）。
     * @param changedIds 新增或变化的对象 id；@param removedIds 被删除的对象 id。
     */
    virtual void refreshReferencingBranchesBatch(const QStringList& changedIds,
                                                 const QStringList& removedIds);

    /** 在整棵树中按 objectId 查找对象分支节点（ObjectTreeNode 全树唯一）。 */
    IDOSObjectTreeNode* findObjectNode(const QString& objectId) const;

    /**
     * 原地重建对象分支：摘除旧节点、经 provider 重新构建后插回原行，
     * 保持分支顺序与勾选状态（对象节点与引用节点的勾选均会保留）。
     */
    void rebuildObjectBranch(IDOSObjectTreeNode* objectNode, const IDOSDataObject* object);

    /** 由节点反查 QModelIndex（用于联动逻辑更新勾选 UI）。 */
    QModelIndex indexOfNode(IDOSTreeNode* node) const;

  private slots:
    void onObjectAdded(const QString& objectId);
    void onObjectRemoved(const QString& objectId);
    void onObjectDataChanged(const QString& objectId);
    void onObjectsAdded(const QStringList& objectIds);
    void onObjectsRemoved(const QStringList& objectIds);
    void onObjectsDataChanged(const QStringList& objectIds);
    void onObjectVisibilityChanged(const QString& objectId, bool visible);
    void onObjectsVisibilityChanged(const QStringList& objectIds);

  private:
    void rebuildTree();
    /** 单个新对象的自身分支处理（同 id 替换转原地重建），不含引用分支刷新。 */
    void processAddedObject(const QString& objectId);
    /** 单个已删对象的自身分支局部删除。 */
    void processRemovedObject(const QString& objectId);
    /** 单个变化对象的自身分支原地重建。 */
    void processChangedObject(const QString& objectId);
    IDOSObjectTreeNode* findObjectNodeRecursive(IDOSTreeNode* branch, const QString& objectId) const;
    void collectCheckedKeys(IDOSTreeNode* branch, QSet<QString>& checkedKeys) const;
    void applyCheckedKeys(IDOSTreeNode* branch, const QSet<QString>& checkedKeys) const;
    QString checkKeyOf(const IDOSTreeNode* node) const;

    /**
     * 勾选某属性节点时，取消整树中其他已勾选的网格属性节点（单选互斥）。
     * 保证渲染窗口同一时刻只显示一个属性场，避免颜色映射互相覆盖。
     * 仅对 IDOSGridProperty 叶子节点生效；几何对象（网格/井本体）勾选不受影响。
     */
    void uncheckOtherProperties(IDOSTreeNode* exceptNode);
    void collectCheckedProperties(IDOSTreeNode* branch,
                                  IDOSTreeNode* exceptNode,
                                  QList<IDOSTreeNode*>& out) const;
    void updateCheckStateForObject(const QString& objectId);
    void updateCheckStateForObject(IDOSTreeNode* branch, const QString& objectId);
    /**
     * @brief 统一获取节点关联的数据对象。
     * 支持 IDOSObjectTreeNode（数据本体树，直接持 objectId）
     * 与 IDOSTreeReferenceNode（工况引用树，通过 IDOSCaseItemRef 间接引用 objectId）。
     * 用于互斥逻辑等需要按对象类型分派的场景。
     */
    IDOSDataObject* objectOfNode(IDOSTreeNode* node) const;

    IDOSProject* m_project;
    IDOSTreeNode* m_rootNode;
};

#endif // IDOS_TREE_MODEL_H
