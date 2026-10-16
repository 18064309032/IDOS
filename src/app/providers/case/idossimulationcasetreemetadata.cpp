#include "idosdataobject.h"
#include "idossimulationcasetreeprovider.h"

#include "idossimulationcasetreemetadata.h"

IDOSSimulationCaseTreeMetadata::IDOSSimulationCaseTreeMetadata(IDOSTreeItemProviderRegistry* registry)
    :
    m_registry(registry)
{
}

QString IDOSSimulationCaseTreeMetadata::id() const
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

IDOSTreeItemProvider* IDOSSimulationCaseTreeMetadata::createProvider(IDOSDataObject* data) const
{
    return new IDOSSimulationCaseTreeProvider(data, m_registry);
}
