#include "idosdataobject.h"
#include "idossimulationcasetreeprovider.h"

#include "idossimulationcasetreemetadata.h"

IDOSSimulationCaseTreeMetadata::IDOSSimulationCaseTreeMetadata(IDOSTreeRegistry* registry)
    :
    m_registry(registry)
{
}

QString IDOSSimulationCaseTreeMetadata::typeId() const
{
    return QStringLiteral("idos.case.simulation");
}

QString IDOSSimulationCaseTreeMetadata::treeName() const
{
    return QStringLiteral("case");
}

QString IDOSSimulationCaseTreeMetadata::groupKey() const
{
    return QStringLiteral("");
}

IDOSTreeProvider* IDOSSimulationCaseTreeMetadata::create(IDOSDataObject* data) const
{
    return new IDOSSimulationCaseTreeProvider(data, m_registry);
}
