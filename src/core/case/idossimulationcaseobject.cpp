#include "idossimulationcaseobject.h"
#include "idoscaseitemref.h"
#include "idosproject.h"
#include "idoswell.h"

IDOSSimulationCaseObject::IDOSSimulationCaseObject(QObject* parent)
    : IDOSCaseObject(parent)
{
}

IDOSSimulationCaseObject::~IDOSSimulationCaseObject()
{
}

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
    if (project == nullptr || m_pendingWellNames.isEmpty())
    {
        return;
    }

    QStringList unresolved;
    QList<IDOSCaseItemRef> refs = itemRefs();
    for (const QString& wellName : m_pendingWellNames)
    {
        QList<IDOSWell*> matches;
        for (IDOSDataObject* object : project->objects())
        {
            IDOSWell* well = qobject_cast<IDOSWell*>(object);
            if (well != nullptr && well->name().trimmed().compare(wellName.trimmed(), Qt::CaseInsensitive) == 0)
            {
                matches.append(well);
            }
        }
        if (matches.size() == 1)
        {
            bool exists = false;
            for (const IDOSCaseItemRef& ref : refs)
            {
                if (ref.role() == QStringLiteral("case.well") && ref.objectId() == matches.first()->objectId())
                {
                    exists = true;
                    break;
                }
            }
            if (!exists)
            {
                refs.append(IDOSCaseItemRef(QStringLiteral("case.well"), matches.first()->objectId()));
            }
        }
        else
        {
            unresolved.append(wellName);
        }
    }
    m_pendingWellNames = unresolved;
    setItemRefs(refs);
}
QStringList IDOSSimulationCaseObject::pendingWellNames() const
{
    return m_pendingWellNames;
}
QString IDOSSimulationCaseObject::sourceFile() const
{
    return m_sourceFile;
}
void IDOSSimulationCaseObject::setSourceFile(const QString& path)
{
    m_sourceFile = path;
    emit dataChanged();
}
