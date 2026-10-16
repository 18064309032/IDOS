#include <memory>

#include <QObject>

#include "idossimulationsummaryprovider.h"

#include "idossimulationsummaryprovidermetadata.h"

QString IDOSSimulationSummaryProviderMetadata::id() const
{
    return QStringLiteral("idos.case.summary.eclipse");
}

QString IDOSSimulationSummaryProviderMetadata::displayName() const
{
    return QObject::tr("ECLIPSE Summary Results");
}

QStringList IDOSSimulationSummaryProviderMetadata::fileExtensions() const
{
    return {QStringLiteral("*.SMSPEC"), QStringLiteral("*.smspec")};
}

std::unique_ptr<IDOSDataProvider> IDOSSimulationSummaryProviderMetadata::createProvider() const
{
    return std::unique_ptr<IDOSDataProvider>(new IDOSSimulationSummaryProvider());
}

bool IDOSSimulationSummaryProviderMetadata::canHandle(const QString& filePath) const
{
    IDOSSimulationSummaryProvider provider;
    return provider.canRead(filePath);
}
