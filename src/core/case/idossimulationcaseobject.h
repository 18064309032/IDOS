#ifndef IDOS_SIMULATION_CASE_OBJECT_H
#define IDOS_SIMULATION_CASE_OBJECT_H

#include <QStringList>

#include "idoscaseobject.h"

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

    QString sourceFile() const;
    void setSourceFile(const QString& path);
    QStringList pendingWellNames() const;
    void setPendingWellNames(const QStringList& names);
    void resolveReferences(IDOSProject* project) override;

  private:
    QStringList m_pendingWellNames;
    QString m_sourceFile;
};

#endif // IDOS_SIMULATION_CASE_OBJECT_H
