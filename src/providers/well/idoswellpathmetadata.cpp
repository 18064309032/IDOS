#include <QObject>

#include "idoswellpathprovider.h"

#include "idoswellpathmetadata.h"

QString IDOSWellPathMetadata::id() const
{
    return QStringLiteral("idos.well.path");
}
QString IDOSWellPathMetadata::displayName() const
{
    return QObject::tr("Well Trajectory");
}
QStringList IDOSWellPathMetadata::fileExtensions() const
{
    return {QStringLiteral("*.dev"), QStringLiteral("*.DEV"), QStringLiteral("*.trj"), QStringLiteral("*.TRJ"),
            QStringLiteral("*.txt"), QStringLiteral("*.dat"), QStringLiteral("*")};
}
std::unique_ptr<IDOSDataProvider> IDOSWellPathMetadata::createProvider() const
{
    return std::make_unique<IDOSWellPathProvider>();
}

bool IDOSWellPathMetadata::canHandle(const QString& filePath) const
{
    IDOSWellPathProvider provider;
    return provider.canRead(filePath);
}
