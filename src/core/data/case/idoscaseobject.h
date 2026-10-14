#ifndef IDOS_CASE_OBJECT_H
#define IDOS_CASE_OBJECT_H

#include <QList>
#include <QVariantMap>

#include "idoscaseitemref.h"
#include "idosdataobject.h"

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
    bool validateProjectReferences(const IDOSProject* project, QString& error) const override;
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    explicit IDOSCaseObject(QObject* parent = nullptr);
    ~IDOSCaseObject() override;

    QList<IDOSCaseItemRef> itemRefs() const;
    void setItemRefs(const QList<IDOSCaseItemRef>& refs);
    void addItemRef(const IDOSCaseItemRef& ref);
    void clearItemRefs();

  private:
    QList<IDOSCaseItemRef> m_itemRefs;
};

#endif // IDOS_CASE_OBJECT_H
