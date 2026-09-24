#ifndef IDOS_DATA_TREE_MODEL_H
#define IDOS_DATA_TREE_MODEL_H

#include "tree/idostreemodel.h"

class IDOSTreeProviderRegistry;

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
    bool shouldShowObject(const IDOSDataObject* object) const override;
    void buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object) override;

private:
    IDOSTreeProviderRegistry* m_treeProviderRegistry;
};

#endif // IDOS_DATA_TREE_MODEL_H
