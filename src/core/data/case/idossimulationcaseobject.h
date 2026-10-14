#ifndef IDOS_SIMULATION_CASE_OBJECT_H
#define IDOS_SIMULATION_CASE_OBJECT_H

#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "idoscaseobject.h"

/**
 * @brief 模拟工况对象。
 */
class CORE_EXPORT IDOSSimulationCaseObject : public IDOSCaseObject
{
    Q_OBJECT

  public:
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    explicit IDOSSimulationCaseObject(QObject* parent = nullptr);
    ~IDOSSimulationCaseObject() override;

    QString typeId() const override;
    bool canClone() const override;
    IDOSDataObject* clone() const override;

    QString sourceFile() const;
    void setSourceFile(const QString& path);
    /** Importer-provided source data, including the summary and keyword index. */
    QVariantMap parsedSourceData() const;
    void setParsedSourceData(const QVariantMap& data);
    QVariantList curveTables() const;
    void setCurveTables(const QVariantList& tables);
    QVariantList wellControlRows() const;
    void setWellControlRows(const QVariantList& rows);
    QVariantMap runInfo() const;
    void setRunInfo(const QVariantMap& runInfo);
    QStringList pendingWellNames() const;
    void setPendingWellNames(const QStringList& names);
    void resolveReferences(IDOSProject* project) override;

  private:
    QStringList m_pendingWellNames;
    QString m_sourceFile;
    QVariantMap m_parsedSourceData;
    QVariantList m_curveTables;
    QVariantList m_wellControlRows;
    QVariantMap m_runInfo;
};

#endif // IDOS_SIMULATION_CASE_OBJECT_H
