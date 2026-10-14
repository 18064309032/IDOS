#ifndef IDOS_FRACTURE_SCHEDULE_H
#define IDOS_FRACTURE_SCHEDULE_H

#include <QStringList>
#include <QVariantList>

#include "idosdataobject.h"

class CORE_EXPORT IDOSFractureSchedule : public IDOSDataObject
{
    Q_OBJECT

  public:
    explicit IDOSFractureSchedule(QObject* parent = nullptr);

    QString typeId() const override;
    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);
    QStringList columns() const;
    void setColumns(const QStringList& columns);
    QStringList units() const;
    void setUnits(const QStringList& units);
    QVariantList samples() const;
    void setSamples(const QVariantList& samples);

  private:
    QString m_sourceFile;
    QStringList m_columns;
    QStringList m_units;
    QVariantList m_samples;
};

#endif // IDOS_FRACTURE_SCHEDULE_H
