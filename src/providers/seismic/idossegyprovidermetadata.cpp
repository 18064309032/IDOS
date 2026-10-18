#include <memory>

#include "idossegyprovider.h"

#include "idossegyprovidermetadata.h"

QString IDOSSegyProviderMetadata::id() const
{
    return QStringLiteral("idos.seismic.segy");
}

QString IDOSSegyProviderMetadata::displayName() const
{
    return QObject::tr("SEG-Y Seismic Volume");
}

QStringList IDOSSegyProviderMetadata::fileExtensions() const
{
    return {QStringLiteral("*.sgy"), QStringLiteral("*.SGY"),
            QStringLiteral("*.segy"), QStringLiteral("*.SEGY")};
}

std::unique_ptr<IDOSDataProvider> IDOSSegyProviderMetadata::create() const
{
    return std::unique_ptr<IDOSDataProvider>(new IDOSSegyProvider());
}

bool IDOSSegyProviderMetadata::canHandle(const QString& filePath) const
{
    IDOSSegyProvider provider;
    return provider.canRead(filePath);
}
