#ifndef IDOS_CASE_OBJECT_H
#define IDOS_CASE_OBJECT_H

#include "data/idosdataobject.h"
#include "case/idoscaseitemref.h"

#include <QList>

/**
 * @brief 工况对象基类。
 *
 * 工况对象是工程中的组织型数据对象，用于保存某个业务案例下引用的输入数据、
 * 参数数据、结果数据和分析视图。具体业务可派生模拟工况、裂缝工况或地质力学工况。
 */
class CORE_EXPORT IDOSCaseObject : public IDOSDataObject
{
    Q_OBJECT

public:
    explicit IDOSCaseObject(QObject* parent = nullptr);
    ~IDOSCaseObject() override;

    /**
     * @brief 获取工况类型标识。
     * @return 稳定工况类型标识，例如 "idos.case.simulation"。
     */
    virtual QString caseTypeId() const = 0;

    QList<IDOSCaseItemRef> itemRefs() const;
    void setItemRefs(const QList<IDOSCaseItemRef>& refs);
    void addItemRef(const IDOSCaseItemRef& ref);
    void clearItemRefs();

private:
    QList<IDOSCaseItemRef> m_itemRefs;
};

#endif // IDOS_CASE_OBJECT_H
