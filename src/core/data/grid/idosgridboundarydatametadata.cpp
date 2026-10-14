#include <QObject>

#include "idosgridboundary.h"

#include "idosgridboundarydatametadata.h"

QString IDOSGridBoundaryDataMetadata::typeId() const
{
    return QStringLiteral("idos.grid.boundary");
}

QString IDOSGridBoundaryDataMetadata::displayName() const
{
    return QObject::tr("Grid Boundary");
}

IDOSDataObject* IDOSGridBoundaryDataMetadata::create() const
{
    return new IDOSGridBoundary();
}
