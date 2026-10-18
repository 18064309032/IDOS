#include <QObject>

#include "data/fluidrock/idosfluidrocktableobject.h"
#include "data/well/idoswellobject.h"
#include "idoscaseitemref.h"
#include "idosproject.h"

#include "idossimulationcaseobject.h"

IDOSSimulationCaseObject::IDOSSimulationCaseObject(QObject* parent)
    :
    IDOSCaseObject(parent)
{
}

IDOSSimulationCaseObject::~IDOSSimulationCaseObject()
{
}

QString IDOSSimulationCaseObject::typeId() const
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
        QList<IDOSWellObject*> matches;
        for (IDOSDataObject* object : project->objects())
        {
            IDOSWellObject* well = qobject_cast<IDOSWellObject*>(object);
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
QVariantMap IDOSSimulationCaseObject::parsedSourceData() const
{
    return m_parsedSourceData;
}
void IDOSSimulationCaseObject::setParsedSourceData(const QVariantMap& data)
{
    m_parsedSourceData = data;
    emit dataChanged();
}
QVariantList IDOSSimulationCaseObject::curveTables() const
{
    QVariantList tables = m_curveTables;
    const IDOSProject* project = qobject_cast<const IDOSProject*>(parent());
    if (project == nullptr)
    {
        return tables;
    }
    QVariantList referencedTables;
    for (const IDOSCaseItemRef& ref : itemRefs())
    {
        if (ref.role() != QStringLiteral("case.fluidRockTable"))
        {
            continue;
        }
        IDOSFluidRockTableObject* table = qobject_cast<IDOSFluidRockTableObject*>(project->object(ref.objectId()));
        if (table != nullptr)
        {
            referencedTables.append(table->tableData());
        }
    }
    return referencedTables.isEmpty() ? tables : referencedTables;
}
void IDOSSimulationCaseObject::setCurveTables(const QVariantList& tables)
{
    m_curveTables = tables;
    emit dataChanged();
}
QVariantList IDOSSimulationCaseObject::wellControlRows() const
{
    return m_wellControlRows;
}

QVariantMap IDOSSimulationCaseObject::runInfo() const
{
    return m_runInfo;
}

void IDOSSimulationCaseObject::setRunInfo(const QVariantMap& runInfo)
{
    m_runInfo = runInfo;
    emit dataChanged();
}

void IDOSSimulationCaseObject::setWellControlRows(const QVariantList& rows)
{
    m_wellControlRows = rows;
    emit dataChanged();
}

bool IDOSSimulationCaseObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSSimulationCaseObject::clone() const
{
    IDOSSimulationCaseObject* copy = new IDOSSimulationCaseObject();
    copy->setName(name());
    copy->m_pendingWellNames = m_pendingWellNames;
    copy->m_sourceFile = m_sourceFile;
    copy->m_parsedSourceData = m_parsedSourceData;
    copy->m_curveTables = m_curveTables;
    copy->m_wellControlRows = m_wellControlRows;
    copy->setItemRefs(itemRefs());
    return copy;
}

QVariantMap IDOSSimulationCaseObject::projectData() const
{
    QVariantMap data = IDOSCaseObject::projectData();
    data.insert(QStringLiteral("pendingWellNames"), QVariant::fromValue(m_pendingWellNames));
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("parsedSourceData"), QVariant::fromValue(m_parsedSourceData));
    data.insert(QStringLiteral("curveTables"), QVariant::fromValue(m_curveTables));
    data.insert(QStringLiteral("wellControlRows"), QVariant::fromValue(m_wellControlRows));
    data.insert(QStringLiteral("runInfo"), QVariant::fromValue(m_runInfo));
    return data;
}

bool IDOSSimulationCaseObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!IDOSCaseObject::restoreProjectData(data, error))
    {
        return false;
    }
    if (!data.contains(QStringLiteral("pendingWellNames")) || data.value(QStringLiteral("pendingWellNames")).userType() != qMetaTypeId<QStringList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("pendingWellNames"));
        return false;
    }
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceFile"));
        return false;
    }
    if (!data.contains(QStringLiteral("parsedSourceData")) || data.value(QStringLiteral("parsedSourceData")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("parsedSourceData"));
        return false;
    }
    if (!data.contains(QStringLiteral("curveTables")) || data.value(QStringLiteral("curveTables")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("curveTables"));
        return false;
    }
    if (!data.contains(QStringLiteral("wellControlRows")) || data.value(QStringLiteral("wellControlRows")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("wellControlRows"));
        return false;
    }
    if (!data.contains(QStringLiteral("runInfo")) || data.value(QStringLiteral("runInfo")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("runInfo"));
        return false;
    }
    m_pendingWellNames = data.value(QStringLiteral("pendingWellNames")).value<QStringList>();
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_parsedSourceData = data.value(QStringLiteral("parsedSourceData")).value<QVariantMap>();
    m_curveTables = data.value(QStringLiteral("curveTables")).value<QVariantList>();
    m_wellControlRows = data.value(QStringLiteral("wellControlRows")).value<QVariantList>();
    m_runInfo = data.value(QStringLiteral("runInfo")).value<QVariantMap>();
    return true;
}
