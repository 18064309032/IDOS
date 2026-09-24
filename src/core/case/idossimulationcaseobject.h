#ifndef IDOS_SIMULATION_CASE_OBJECT_H
#define IDOS_SIMULATION_CASE_OBJECT_H

#include "case/idoscaseobject.h"

#include <QStringList>

/**
 * @brief 模拟工况对象。
 */
class CORE_EXPORT IDOSSimulationCaseObject : public IDOSCaseObject
{
    Q_OBJECT

public:
    explicit IDOSSimulationCaseObject(QObject* parent = nullptr);
    ~IDOSSimulationCaseObject() override;

    QString typeId() const override;
    QString caseTypeId() const override;

    void setPendingWellNames(const QStringList& names);
    void resolveReferences(IDOSProject* project) override;

private:
    QStringList m_pendingWellNames;
};

#endif // IDOS_SIMULATION_CASE_OBJECT_H
