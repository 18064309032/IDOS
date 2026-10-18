#include <memory>

#include <QObject>

#include "idosfracturematerialcatalogprovider.h"

#include "idosfracturematerialcatalogmetadata.h"

QString IDOSFractureMaterialCatalogMetadata::id() const
{
    return QStringLiteral("idos.fracture.materialCatalog.tsv");
}

QString IDOSFractureMaterialCatalogMetadata::displayName() const
{
    return QObject::tr("Fracture Material Catalog (TSV)");
}

QStringList IDOSFractureMaterialCatalogMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("*.txt"), QStringLiteral("*.TXT")};
}

std::unique_ptr<IDOSDataProvider> IDOSFractureMaterialCatalogMetadata::create() const
{
    return std::unique_ptr<IDOSDataProvider>(new IDOSFractureMaterialCatalogProvider());
}

bool IDOSFractureMaterialCatalogMetadata::canHandle(const QString& filePath) const
{
    IDOSFractureMaterialCatalogProvider provider;
    return provider.canRead(filePath);
}
