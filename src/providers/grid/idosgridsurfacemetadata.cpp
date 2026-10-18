#include <memory>

#include "idosgridsurfaceprovider.h"

#include "idosgridsurfacemetadata.h"

QString IDOSGridSurfaceMetadata::id() const
{
    return QStringLiteral("idos.grid.surface.zims");
}

QString IDOSGridSurfaceMetadata::displayName() const
{
    return QObject::tr("ZIMS Grid Surface");
}

QStringList IDOSGridSurfaceMetadata::fileExtensions() const
{
    return {QStringLiteral("*"), QStringLiteral("*.zims"), QStringLiteral("*.ZIMS")};
}

std::unique_ptr<IDOSDataProvider> IDOSGridSurfaceMetadata::create() const
{
    return std::unique_ptr<IDOSDataProvider>(new IDOSGridSurfaceProvider());
}

bool IDOSGridSurfaceMetadata::canHandle(const QString& filePath) const
{
    IDOSGridSurfaceProvider provider;
    return provider.canRead(filePath);
}
