#ifndef IDOS_TREE_GROUP_NODE_H
#define IDOS_TREE_GROUP_NODE_H

#include "idostreenode.h"

/**
 * @brief 通用分组树节点。
 */
class GUI_EXPORT IDOSTreeGroupNode : public IDOSTreeNode
{
  public:
    explicit IDOSTreeGroupNode(IDOSTreeNode* parent = nullptr);
    ~IDOSTreeGroupNode() override;

    QString groupKey() const;
    void setGroupKey(const QString& groupKey);

    QString nodeKey() const override;

  private:
    QString m_groupKey;
};

#endif // IDOS_TREE_GROUP_NODE_H
