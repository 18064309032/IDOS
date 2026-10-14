#ifndef IDOS_PRODUCTION_HISTORY_H
#define IDOS_PRODUCTION_HISTORY_H

#include <QStringList>
#include <QVariantList>

#include "idosdataobject.h"

class CORE_EXPORT IDOSProductionHistory : public IDOSDataObject
{
    Q_OBJECT

  public:
    explicit IDOSProductionHistory(QObject* parent = nullptr);

    QString typeId() const override;
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
