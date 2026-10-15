#include <QObject>

#include "idossimulationsummaryresults.h"

#include "idossimulationsummaryresultsmetadata.h"

QString IDOSSimulationSummaryResultsMetadata::typeId() const
{
    return QStringLiteral("idos.case.summaryResults");
}

QString IDOSSimulationSummaryResultsMetadata::displayName() const
{
    return QObject::tr("ECLIPSE Summary Results");
}

IDOSDataObject* IDOSSimulationSummaryResultsMetadata::create() const
{
    return new IDOSSimulationSummaryResults();
}
