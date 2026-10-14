#include <QObject>

#include "idosfracturematerialcatalog.h"

#include "idosfracturematerialcatalogdatametadata.h"

IDOSFractureMaterialCatalogDataMetadata::IDOSFractureMaterialCatalogDataMetadata(
    IDOSFractureMaterialCatalog::Kind kind)
    :
    m_kind(kind)
{
}

QString IDOSFractureMaterialCatalogDataMetadata::typeId() const
{
    if (m_kind == IDOSFractureMaterialCatalog::Kind::Proppant)
    {
        return QStringLiteral("idos.fracture.proppantCatalog");
    }
    return QStringLiteral("idos.fracture.fluidCatalog");
}

QString IDOSFractureMaterialCatalogDataMetadata::displayName() const
{
    return QObject::tr("Fracture Material Catalog");
}

IDOSDataObject* IDOSFractureMaterialCatalogDataMetadata::create() const
{
    IDOSFractureMaterialCatalog* catalog = new IDOSFractureMaterialCatalog();
    catalog->setKind(m_kind);
    return catalog;
}
