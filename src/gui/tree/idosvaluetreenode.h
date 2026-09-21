#ifndef IDOS_VALUE_TREE_NODE_H
#define IDOS_VALUE_TREE_NODE_H

#include "idostreenode.h"
#include "idos_gui.h"
#include <QString>

/**
 * @brief 值节点：领域对象内部值数据的树展示节点。
 *
 * 表示领域对象内部的值对象（如井的井头/轨迹段/射孔段）在树上的展示节点。
 * 值对象没有 objectId（值语义、存于领域对象内部），因此节点不继承
 * IDOSObjectTreeNode，通过 ownerObjectId + role + index 间接定位数据。
 * 天然不带可见性勾选框（可见性属于领域对象整体）。
 */
class GUI_EXPORT IDOSValueTreeNode : public IDOSTreeNode
{
public:
    /** 值节点语义角色。 */
    enum class ValueRole
    {
        WellHead,        // 井头（单值）
        WellControl,     // 井控参数（单值）
        SegmentGroup,    // 井轨迹分组节点
        Segment,         // 井轨迹段（列表项）
        CompletionGroup, // 射孔分组节点
        Completion       // 射孔段（列表项）
    };

    /**
     * @brief 构造值节点。
     * @param parent 父节点（由调用方随后 appendChild 挂接）。
     * @param ownerObjectId 所属领域对象的 objectId。
     * @param role 值语义角色。
     * @param index 列表内索引；单值/分组角色传 0。
     */
    IDOSValueTreeNode(IDOSTreeNode* parent, const QString& ownerObjectId,
                      ValueRole role, int index);
    ~IDOSValueTreeNode() override;

    /** 所属领域对象的 objectId。 */
    QString ownerObjectId() const;

    /** 值语义角色。 */
    ValueRole role() const;

    /** 列表内索引（0-based）。 */
    int index() const;

private:
    QString m_ownerObjectId;
    ValueRole m_role;
    int m_index;
};

#endif // IDOS_VALUE_TREE_NODE_H
