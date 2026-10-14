#include <QObject>

#include "idosgridsurface.h"

#include "idosgridsurfacedatametadata.h"

QString IDOSGridSurfaceDataMetadata::typeId() const
{
    return QStringLiteral("idos.grid.surface");
}

QString IDOSGridSurfaceDataMetadata::displayName() const
{
    return QObject::tr("Grid Surface");
}

IDOSDataObject* IDOSGridSurfaceDataMetadata::create() const
{
    return new IDOSGridSurface();
}
