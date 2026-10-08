#include <QObject>

#include "idoswellpathprovider.h"

#include "idoswellpathmetadata.h"

QString IDOSWellPathMetadata::id() const
{
    return QStringLiteral("idos.well.path");
}
QString IDOSWellPathMetadata::displayName() const
{
    return QObject::tr("Petrel Well Trajectory");
}
QStringList IDOSWellPathMetadata::fileExtensions() const
{
    return {QStringLiteral("*.dev"), QStringLiteral("*.DEV")};
}
std::unique_ptr<IDOSDataProvider> IDOSWellPathMetadata::createProvider() const
{
    return std::make_unique<IDOSWellPathProvider>();
}
