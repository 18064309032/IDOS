#include "idoswelllasprovider.h"

#include "idoswelllasmetadata.h"

std::unique_ptr<IDOSDataProvider> IDOSWellLasMetadata::createProvider() const
{
    return std::make_unique<IDOSWellLasProvider>();
}

QString IDOSWellLasMetadata::id() const
{
    return QStringLiteral("idos.well.las");
}

QString IDOSWellLasMetadata::displayName() const
{
    return QObject::tr("LAS Well");
}

bool IDOSWellLasMetadata::canHandle(const QString& filePath) const
{
    IDOSWellLasProvider provider;
    return provider.canRead(filePath);
}

QStringList IDOSWellLasMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("*.las"), QStringLiteral("*.LAS"), QStringLiteral("*")};
}
