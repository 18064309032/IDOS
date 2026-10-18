#include <QObject>

#include "idosfracturematerialcatalogobject.h"

#include "idosfracturematerialcatalogdatametadata.h"

IDOSFractureMaterialCatalogDataMetadata::IDOSFractureMaterialCatalogDataMetadata(
    IDOSFractureMaterialCatalogObject::Kind kind)
    :
    m_kind(kind)
{
}

QString IDOSFractureMaterialCatalogDataMetadata::typeId() const
{
    if (m_kind == IDOSFractureMaterialCatalogObject::Kind::Proppant)
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
    IDOSFractureMaterialCatalogObject* catalog = new IDOSFractureMaterialCatalogObject();
    catalog->setKind(m_kind);
    return catalog;
}
