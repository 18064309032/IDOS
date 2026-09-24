#include "case/idossimulationcaseobject.h"
#include "case/idoscaseitemref.h"
#include "idosproject.h"

IDOSSimulationCaseObject::IDOSSimulationCaseObject(QObject* parent)
    : IDOSCaseObject(parent)
{
}

IDOSSimulationCaseObject::~IDOSSimulationCaseObject() = default;

QString IDOSSimulationCaseObject::typeId() const
{
    return QStringLiteral("idos.case");
}

QString IDOSSimulationCaseObject::caseTypeId() const
{
    return QStringLiteral("idos.case.simulation");
}

void IDOSSimulationCaseObject::setPendingWellNames(const QStringList& names)
{
    m_pendingWellNames = names;
}

void IDOSSimulationCaseObject::resolveReferences(IDOSProject* project)
{
    if (project == nullptr || m_pendingWellNames.isEmpty()) return;

    QStringList unresolved;
    for (const QString& wellName : m_pendingWellNames)
    {
        IDOSDataObject* well = project->objectByName(wellName);
        if (well != nullptr) addItemRef(IDOSCaseItemRef(QStringLiteral("case.well"), well->objectId()));
        else unresolved.append(wellName);
    }
    m_pendingWellNames = unresolved;
}
