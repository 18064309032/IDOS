#include <QObject>

#include "idoswellcheckshotprovider.h"

#include "idoswellcheckshotmetadata.h"

QString IDOSWellCheckshotMetadata::id() const
{
    return QStringLiteral("idos.well.checkshot");
}

QString IDOSWellCheckshotMetadata::displayName() const
{
    return QObject::tr("Well Checkshots");
}

QStringList IDOSWellCheckshotMetadata::fileExtensions() const
{
    return {QStringLiteral("*.checkshot"), QStringLiteral("*.cs"), QStringLiteral("*.txt"), QStringLiteral("*.dat")};
}

std::unique_ptr<IDOSDataProvider> IDOSWellCheckshotMetadata::create() const
{
    return std::make_unique<IDOSWellCheckshotProvider>();
}

bool IDOSWellCheckshotMetadata::canHandle(const QString& filePath) const
{
    IDOSWellCheckshotProvider provider;
    return provider.canRead(filePath);
}
