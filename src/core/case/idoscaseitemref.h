#ifndef IDOS_CASE_ITEM_REF_H
#define IDOS_CASE_ITEM_REF_H

#include "idos_core.h"

#include <QString>

/**
 * @brief 工况中的数据引用项。
 *
 * 工况对象通过引用项组织井、网格、属性、裂缝和计算结果等数据。
 * 引用项只保存稳定标识，不复制被引用的数据。
 */
class CORE_EXPORT IDOSCaseItemRef
{
public:
    IDOSCaseItemRef();
    IDOSCaseItemRef(const QString& role, const QString& objectId);

    QString refId() const;
    void setRefId(const QString& refId);

    QString role() const;
    void setRole(const QString& role);

    QString objectId() const;
    void setObjectId(const QString& objectId);

    QString partKey() const;
    void setPartKey(const QString& partKey);

    QString itemKey() const;
    void setItemKey(const QString& itemKey);

private:
    QString m_refId;
    QString m_role;
    QString m_objectId;
    QString m_partKey;
    QString m_itemKey;
};

#endif // IDOS_CASE_ITEM_REF_H
