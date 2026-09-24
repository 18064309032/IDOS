#ifndef IDOS_CASE_TREE_MODEL_H
#define IDOS_CASE_TREE_MODEL_H

#include "tree/idostreemodel.h"

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

protected:
    bool shouldShowObject(const IDOSDataObject* object) const override;
    void buildObjectTree(IDOSTreeNode* parentNode, const IDOSDataObject* object) override;

private:
    IDOSTreeProviderRegistry* m_treeProviderRegistry;
};

#endif // IDOS_CASE_TREE_MODEL_H
