#ifndef IDOS_PRODUCTION_HISTORY_H
#define IDOS_PRODUCTION_HISTORY_H

#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "idosdataobject.h"

class CORE_EXPORT IDOSProductionHistory : public IDOSDataObject
{
    Q_OBJECT

  public:
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    explicit IDOSProductionHistory(QObject* parent = nullptr);

    QString typeId() const override;
    bool canClone() const override;
    IDOSDataObject* clone() const override;
    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);
    QStringList columns() const;
    void setColumns(const QStringList& columns);
    QVariantList records() const;
    void setRecords(const QVariantList& records);

  private:
    QString m_sourceFile;
    QStringList m_columns;
    QVariantList m_records;
};

#endif // IDOS_PRODUCTION_HISTORY_H
