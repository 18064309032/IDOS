#ifndef IDOS_DATA_TREE_MODEL_H
#define IDOS_DATA_TREE_MODEL_H

#include "idostreemodel.h"
#include <QHash>

class IDOSTreeProviderRegistry;
class IDOSTreeBuilder;
class IDOSTreeGroupNode;

/**
 * @brief 数据树模型。
 *
 * 数据树负责显示工程中的基础数据对象。
 */
class GUI_EXPORT IDOSDataTreeModel : public IDOSTreeModel
{
    Q_OBJECT

  public:
    explicit IDOSDataTreeModel(QObject* parent = nullptr);
    ~IDOSDataTreeModel() override;

    void setTreeProviderRegistry(IDOSTreeProviderRegistry* registry);

  protected:
    void buildDefaultTree(IDOSTreeNode* rootNode) override;
    bool shouldShowObject(const IDOSDataObject* object) const override;
    void buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object) override;
    IDOSTreeNode* parentNodeForNewObject(const IDOSDataObject* object) const override;

  private:
    /** 建一个分类节点：设标题/图标并登记到 m_groups。返回新建节点供挂子级。 */
    IDOSTreeGroupNode* addGroupNode(IDOSTreeBuilder& builder,
                                    const QString& key,
                                    const QString& title,
                                    const QString& iconPath);

    QHash<QString, IDOSTreeNode*> m_groups;
    IDOSTreeProviderRegistry* m_treeProviderRegistry;
};

#endif // IDOS_DATA_TREE_MODEL_H
