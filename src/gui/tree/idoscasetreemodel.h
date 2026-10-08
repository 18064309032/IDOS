#ifndef IDOS_CASE_TREE_MODEL_H
#define IDOS_CASE_TREE_MODEL_H

#include <QList>

#include "idostreemodel.h"

class IDOSTreeProviderRegistry;

/**
 * @brief 工况树模型。
 *
 * 工况树负责显示工况对象及其引用的数据对象。
 */
class GUI_EXPORT IDOSCaseTreeModel : public IDOSTreeModel
{
    Q_OBJECT

  public:
    explicit IDOSCaseTreeModel(QObject* parent = nullptr);
    ~IDOSCaseTreeModel() override;

    void setTreeProviderRegistry(IDOSTreeProviderRegistry* registry);
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    Q_SIGNAL void itemCheckedChanged(const QString& objectId, bool checked);

  protected:
    bool shouldShowObject(const IDOSDataObject* object) const override;
    void buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object) override;
    void refreshReferencingBranches(const QString& objectId, bool objectRemoved) override;
    void refreshReferencingBranchesBatch(const QStringList& changedIds,
                                         const QStringList& removedIds) override;

  private slots:
    void onCheckStateChanged(const QModelIndex& index, bool checked);

  private:
    IDOSTreeProviderRegistry* m_treeProviderRegistry;

    // 父子联动：属性依附网格几何
    /** 勾选属性时，自动勾选其所属网格节点（保证网格 renderObject 存在且 UI 一致）。 */
    void autoCheckParentGrid(IDOSTreeNode* propertyNode);
    /** 取消勾选网格时，连带取消其下所有已勾选属性节点。 */
    void uncheckChildProperties(IDOSTreeNode* gridNode);
    void collectCheckedGridProperties(IDOSTreeNode* branch, QList<IDOSTreeNode*>& out) const;
};

#endif // IDOS_CASE_TREE_MODEL_H


