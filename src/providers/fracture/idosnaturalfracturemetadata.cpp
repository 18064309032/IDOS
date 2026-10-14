#include <memory>

#include <QObject>

#include "idosnaturalfractureprovider.h"

#include "idosnaturalfracturemetadata.h"

QString IDOSNaturalFractureMetadata::id() const
{
    return QStringLiteral("idos.naturalFracture.ascii");
}

QString IDOSNaturalFractureMetadata::displayName() const
{
    return QObject::tr("ASCII Natural Fractures");
}

QStringList IDOSNaturalFractureMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("*.fab"), QStringLiteral("*.FAB")};
}

std::unique_ptr<IDOSDataProvider> IDOSNaturalFractureMetadata::create() const
{
    return std::unique_ptr<IDOSDataProvider>(new IDOSNaturalFractureProvider());
}

bool IDOSNaturalFractureMetadata::canHandle(const QString& filePath) const
{
    IDOSNaturalFractureProvider provider;
    return provider.canRead(filePath);
}
