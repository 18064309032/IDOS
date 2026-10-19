#ifndef IDOS_GRID_PROPERTY_METADATA_H
#define IDOS_GRID_PROPERTY_METADATA_H

#include <QList>
#include <QVector>

#include "data/idosdatametadata.h"

class IDOSGridPropertyObject;

class CORE_EXPORT IDOSGridPropertyMetadata : public IDOSDataMetadata
{
  public:
    QString typeId() const override;
    QString displayName() const override;
    IDOSDataObject* create() const override;
    bool writeProjectData(qint64 group, const IDOSDataObject* object, QString& error) const override;
    IDOSDataObject* readProjectData(qint64 group, QString& error) const override;

  private:
    static bool writeDoubleVector(qint64 group, const char* name, const QVector<double>& values);
    static bool writeReportSteps(qint64 group, const IDOSGridPropertyObject* object);
    static bool readDoubleVector(qint64 group, const char* name, QVector<double>& values);
    static bool readReportSteps(qint64 group, IDOSGridPropertyObject* object, QString& error);
};

#endif
