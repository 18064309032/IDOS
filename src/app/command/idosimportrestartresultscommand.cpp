#include <utility>

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QObject>

#include "data/grid/idosgrid.h"
#include "data/grid/idosgridproperty.h"
#include "grid/idosgridpropertyeclipseprovider.h"
#include "idosproject.h"

#include "idosimportrestartresultscommand.h"

IDOSImportRestartResultsCommandMetadata::IDOSImportRestartResultsCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("simulation.results.import"),
                        QObject::tr("Import simulation results"),
                        QObject::tr("Import dynamic properties from a unified Eclipse restart file."),
                        IDOSCommand::Type::Action)
{
}

IDOSImportRestartResultsCommandMetadata::~IDOSImportRestartResultsCommandMetadata() = default;

QJsonObject IDOSImportRestartResultsCommandMetadata::schema() const
{
    QJsonObject grid;
    grid.insert(QStringLiteral("type"), QStringLiteral("string"));
    grid.insert(QStringLiteral("description"), QObject::tr("Grid object ID."));
    QJsonObject file;
    file.insert(QStringLiteral("type"), QStringLiteral("string"));
    file.insert(QStringLiteral("description"), QObject::tr("Unified Eclipse restart file path."));
    QJsonObject properties;
    properties.insert(QStringLiteral("gridId"), grid);
    properties.insert(QStringLiteral("filePath"), file);
    QJsonArray required;
    required.append(QStringLiteral("gridId"));
    required.append(QStringLiteral("filePath"));
    QJsonObject schema;
    schema.insert(QStringLiteral("type"), QStringLiteral("object"));
    schema.insert(QStringLiteral("properties"), properties);
    schema.insert(QStringLiteral("required"), required);
    schema.insert(QStringLiteral("additionalProperties"), false);
    return schema;
}

IDOSCommand* IDOSImportRestartResultsCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSImportRestartResultsCommand(project,
                                               arguments.value(QStringLiteral("gridId")).toString(),
                                               arguments.value(QStringLiteral("filePath")).toString());
}

IDOSImportRestartResultsCommand::IDOSImportRestartResultsCommand(IDOSProject* project,
                                                                 QString gridId,
                                                                 QString filePath)
    :
    IDOSCommand(QStringLiteral("simulation.results.import"),
                Type::Action,
                project,
                QObject::tr("Import simulation results"))
    , m_gridId(std::move(gridId))
    , m_filePath(std::move(filePath))
    , m_firstReportStep(-1)
    , m_lastReportStep(-1)
    , m_reportStepCount(0)
    , m_loaded(false)
{
}

IDOSImportRestartResultsCommand::~IDOSImportRestartResultsCommand()
{
    deleteDetachedProperties();
}

bool IDOSImportRestartResultsCommand::validateCommand(QString& error) const
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (qobject_cast<IDOSGrid*>(targetProject->object(m_gridId)) == nullptr)
    {
        error = QObject::tr("The target grid is unavailable.");
        return false;
    }

    const QFileInfo fileInfo(m_filePath);
    if (!m_loaded && (!fileInfo.isFile() || !fileInfo.isReadable()))
    {
        error = QObject::tr("The restart file is unavailable or unreadable.");
        return false;
    }

    if (fileInfo.suffix().compare(QStringLiteral("UNRST"), Qt::CaseInsensitive) != 0)
    {
        error = QObject::tr("Select a unified Eclipse restart file (.UNRST).");
        return false;
    }

    return true;
}

QJsonObject IDOSImportRestartResultsCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("gridId"), m_gridId);
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("type"), QStringLiteral("idos.grid.property.dynamic"));
    return preview;
}

bool IDOSImportRestartResultsCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (!m_loaded && !loadProperties(error))
    {
        deleteDetachedProperties();
        return false;
    }

    for (const QPointer<IDOSGridProperty>& property : m_properties)
    {
        if (property.isNull() || property->parent() != nullptr)
        {
            error = QObject::tr("A result property is unavailable or already attached.");
            return false;
        }
    }

    targetProject->beginUpdate();
    for (const QPointer<IDOSGridProperty>& property : m_properties)
    {
        targetProject->add(property.data());
    }
    targetProject->endUpdate();

    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("gridId"), m_gridId);
    commandResult.insert(QStringLiteral("firstReportStep"), m_firstReportStep);
    commandResult.insert(QStringLiteral("lastReportStep"), m_lastReportStep);
    commandResult.insert(QStringLiteral("reportStepCount"), m_reportStepCount);
    commandResult.insert(QStringLiteral("propertyCount"), m_properties.size());
    setResult(commandResult);
    return true;
}

bool IDOSImportRestartResultsCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    for (const QPointer<IDOSGridProperty>& property : m_properties)
    {
        if (property.isNull() || targetProject->object(property->objectId()) != property.data())
        {
            error = QObject::tr("A result property is no longer available in the project.");
            return false;
        }
    }

    QList<IDOSGridProperty*> detachedProperties;
    targetProject->beginUpdate();
    for (const QPointer<IDOSGridProperty>& property : m_properties)
    {
        IDOSDataObject* detachedObject = targetProject->take(property->objectId());
        IDOSGridProperty* detachedProperty = qobject_cast<IDOSGridProperty*>(detachedObject);
        if (detachedProperty == nullptr)
        {
            if (detachedObject != nullptr)
            {
                targetProject->add(detachedObject);
            }
            for (IDOSGridProperty* previousProperty : detachedProperties)
            {
                targetProject->add(previousProperty);
            }
            targetProject->endUpdate();
            error = QObject::tr("A result property could not be detached from the project.");
            return false;
        }
        detachedProperties.append(detachedProperty);
    }

    targetProject->endUpdate();
    return true;
}

bool IDOSImportRestartResultsCommand::loadProperties(QString& error)
{
    IDOSProject* targetProject = project();
    const IDOSGrid* grid = targetProject != nullptr
        ? qobject_cast<IDOSGrid*>(targetProject->object(m_gridId)) : nullptr;
    IDOSGridPropertyEclipseProvider provider;
    QList<IDOSDataObject*> objects = provider.readRestart(m_filePath, grid);
    if (objects.isEmpty() || !provider.lastError().isEmpty())
    {
        error = provider.lastError();
        qDeleteAll(objects);
        return false;
    }
    for (IDOSDataObject* object : objects)
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        for (IDOSDataObject* existingObject : targetProject->objects())
        {
            const IDOSGridProperty* existing = qobject_cast<IDOSGridProperty*>(existingObject);
            if (existing != nullptr && existing->gridId() == m_gridId
                && existing->kind() == IDOSGridProperty::Kind::Dynamic
                && (existing->keyword().compare(property->keyword(), Qt::CaseInsensitive) == 0
                    || existing->name() == property->name()))
            {
                error = QObject::tr("Result property %1 already exists. Remove it before importing again.")
                    .arg(property->keyword());
                qDeleteAll(objects);
                return false;
            }
        }
    }
    for (IDOSDataObject* object : objects)
    {
        m_properties.append(QPointer<IDOSGridProperty>(qobject_cast<IDOSGridProperty*>(object)));
    }
    const QList<int> reportSteps = provider.restartReportSteps();
    m_firstReportStep = reportSteps.first();
    m_lastReportStep = reportSteps.last();
    m_reportStepCount = reportSteps.size();
    m_loaded = true;
    return true;
}

void IDOSImportRestartResultsCommand::deleteDetachedProperties()
{
    for (const QPointer<IDOSGridProperty>& property : m_properties)
    {
        if (!property.isNull() && property->parent() == nullptr)
        {
            delete property.data();
        }
    }
    m_properties.clear();
}
