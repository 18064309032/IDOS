#ifndef IDOS_WELL_PATH_PROVIDER_H
#define IDOS_WELL_PATH_PROVIDER_H

#include <QMap>
#include <QStringList>
#include <QVector>

#include "data/well/idoswellpathpoint.h"
#include "data/well/idoswellspatialreference.h"
#include "idos_providers.h"
#include "providers/idosdataprovider.h"

class PROVIDERS_EXPORT IDOSWellPathProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;
    bool canRead(const QString& filePath) const;

  private:
    bool readTabularTrajectory(const QString& filePath, QList<IDOSDataObject*>* result);
    bool parseHeader(const QStringList& fields, QMap<QString, int>* columns) const;
    bool appendWell(const QString& wellName, const QVector<IDOSWellPathPoint>& points,
                    const QStringList& comments, QList<IDOSDataObject*>* result) const;
    void clearResult(QList<IDOSDataObject*>* result) const;
    IDOSWellSpatialReference spatialReferenceFromComments(const QStringList& comments) const;
};

#endif // IDOS_WELL_PATH_PROVIDER_H
