#ifndef IDOS_TREE_PART_NODE_H
#define IDOS_TREE_PART_NODE_H

#include "tree/idostreenode.h"
#include "tree/idostreepartkey.h"

/**
 * @brief 指向数据对象内部业务部分的树节点。
 */
class GUI_EXPORT IDOSTreePartNode : public IDOSTreeNode
{
public:
    explicit IDOSTreePartNode(IDOSTreeNode* parent = nullptr);
    ~IDOSTreePartNode() override;

    QString ownerObjectId() const;
    void setOwnerObjectId(const QString& ownerObjectId);

    IDOSTreePartKey partKey() const;
    void setPartKey(const IDOSTreePartKey& partKey);

    QString itemKey() const;
    void setItemKey(const QString& itemKey);

private:
    QString m_ownerObjectId;
    IDOSTreePartKey m_partKey;
    QString m_itemKey;
};

#endif // IDOS_TREE_PART_NODE_H
