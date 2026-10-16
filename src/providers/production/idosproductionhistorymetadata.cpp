#include <memory>

#include <QObject>

#include "idosproductionhistoryprovider.h"

#include "idosproductionhistorymetadata.h"

QString IDOSProductionHistoryMetadata::id() const
{
    return QStringLiteral("idos.production.vol");
}

QString IDOSProductionHistoryMetadata::displayName() const
{
    return QObject::tr("Production History (VOL)");
}

QStringList IDOSProductionHistoryMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("*.vol"), QStringLiteral("*.VOL")};
}

std::unique_ptr<IDOSDataProvider> IDOSProductionHistoryMetadata::createProvider() const
{
    return std::unique_ptr<IDOSDataProvider>(new IDOSProductionHistoryProvider());
}

bool IDOSProductionHistoryMetadata::canHandle(const QString& filePath) const
{
    IDOSProductionHistoryProvider provider;
    return provider.canRead(filePath);
}
