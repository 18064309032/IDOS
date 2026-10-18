#include <memory>

#include "idosgridboundaryprovider.h"

#include "idosgridboundarymetadata.h"

QString IDOSGridBoundaryMetadata::id() const
{
    return QStringLiteral("idos.grid.boundary.xygi");
}

QString IDOSGridBoundaryMetadata::displayName() const
{
    return QObject::tr("Grid Boundary Points");
}

QStringList IDOSGridBoundaryMetadata::fileExtensions() const
{
    return {QStringLiteral("*"), QStringLiteral("*.txt"), QStringLiteral("*.dat")};
}

std::unique_ptr<IDOSDataProvider> IDOSGridBoundaryMetadata::create() const
{
    return std::unique_ptr<IDOSDataProvider>(new IDOSGridBoundaryProvider());
}

bool IDOSGridBoundaryMetadata::canHandle(const QString& filePath) const
{
    IDOSGridBoundaryProvider provider;
    return provider.canRead(filePath);
}
