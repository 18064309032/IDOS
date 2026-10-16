#include "idosgridpropertyeclipseprovider.h"

#include "idosgridpropertyeclipsemetadata.h"

std::unique_ptr<IDOSDataProvider> IDOSGridPropertyEclipseMetadata::createProvider() const
{
    return std::make_unique<IDOSGridPropertyEclipseProvider>();
}

QString IDOSGridPropertyEclipseMetadata::id() const
{
    return QStringLiteral("idos.gridproperty.eclipse");
}

QString IDOSGridPropertyEclipseMetadata::displayName() const
{
    return QObject::tr("ECLIPSE Grid Properties");
}

QStringList IDOSGridPropertyEclipseMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("*.GRDECL"), QStringLiteral("*.INC"), QStringLiteral("*.INIT")};
}
