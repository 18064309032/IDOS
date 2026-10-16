#include "idosgrideclipseprovider.h"

#include "idosgrideclipsemetadata.h"

std::unique_ptr<IDOSDataProvider> IDOSGridEclipseMetadata::createProvider() const
{
    return std::make_unique<IDOSGridEclipseProvider>();
}

QString IDOSGridEclipseMetadata::id() const
{
    return QStringLiteral("idos.grid.eclipse");
}

QString IDOSGridEclipseMetadata::displayName() const
{
    return QObject::tr("ECLIPSE Grid");
}

QStringList IDOSGridEclipseMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("*.GRID"), QStringLiteral("*.EGRID")};
}
