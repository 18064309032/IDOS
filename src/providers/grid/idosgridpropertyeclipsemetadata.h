#ifndef IDOS_GRID_PROPERTY_ECLIPSE_METADATA_H
#define IDOS_GRID_PROPERTY_ECLIPSE_METADATA_H

#include <memory>

#include <QObject>
#include <QString>
#include <QStringList>

#include "idos_providers.h"
#include "providers/idosdataprovidermetadata.h"

class PROVIDERS_EXPORT IDOSGridPropertyEclipseMetadata : public IDOSDataProviderMetadata
{
  public:
    QString id() const override;
    QString displayName() const override;
    QStringList fileExtensions() const override;
    std::unique_ptr<IDOSDataProvider> create() const override;
};

#endif // IDOS_GRID_PROPERTY_ECLIPSE_METADATA_H
