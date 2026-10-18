#ifndef IDOS_FRACTURE_SCHEDULE_OBJECT_H
#define IDOS_FRACTURE_SCHEDULE_OBJECT_H

#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "idosdataobject.h"

class CORE_EXPORT IDOSFractureScheduleObject : public IDOSDataObject
{
    Q_OBJECT

  public:
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    explicit IDOSFractureScheduleObject(QObject* parent = nullptr);

    QString typeId() const override;
    bool canClone() const override;
    IDOSDataObject* clone() const override;
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

#endif // IDOS_FRACTURE_SCHEDULE_OBJECT_H
