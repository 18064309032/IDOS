#include <QObject>

#include "idosproductionhistory.h"

#include "idosproductionhistorydatametadata.h"

QString IDOSProductionHistoryDataMetadata::typeId() const
{
    return QStringLiteral("idos.production.history");
}

QString IDOSProductionHistoryDataMetadata::displayName() const
{
    return QObject::tr("Production History");
}

IDOSDataObject* IDOSProductionHistoryDataMetadata::create() const
{
    return new IDOSProductionHistory();
}
