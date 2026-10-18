#include <QFileInfo>
#include <QJsonArray>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QVariantList>

#include "data/case/idossimulationcaseobject.h"
#include "data/fluidrock/idosfluidrocktableobject.h"
#include "fluidrock/idosfluidrocktextprovider.h"
#include "idosdataobject.h"
#include "idosproject.h"

#include "idosimportfluidrocktablescommand.h"

IDOSImportFluidRockTablesCommand::IDOSImportFluidRockTablesCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("data.fluidRock.import"), Type::Action, project, QObject::tr("Import fluid and rock tables"))
    , m_filePath(arguments.value(QStringLiteral("filePath")).toString())
    , m_loaded(false)
    , m_fluidRockCategory(arguments.value(QStringLiteral("category")).toString())
{
}

IDOSImportFluidRockTablesCommand::~IDOSImportFluidRockTablesCommand()
{
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            delete object.data();
        }
    }
}

bool IDOSImportFluidRockTablesCommand::validateCommand(QString& error) const
{
    if (project() == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (m_fluidRockCategory != QStringLiteral("fluid") && m_fluidRockCategory != QStringLiteral("rockPhysics"))
    {
        error = QObject::tr("The fluid or rock table category is invalid.");
        return false;
    }

    if (m_filePath.trimmed().isEmpty() || (!m_loaded && !QFileInfo::exists(m_filePath)))
    {
        error = QObject::tr("The import file is unavailable.");
        return false;
    }

    return true;
}

QJsonObject IDOSImportFluidRockTablesCommand::buildPreview() const
{
    QJsonObject preview;
    preview.insert(QStringLiteral("filePath"), m_filePath);
    preview.insert(QStringLiteral("category"), m_fluidRockCategory);
    return preview;
}

bool IDOSImportFluidRockTablesCommand::apply(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    if (!m_loaded && !loadObjects(error))
    {
        return false;
    }

    QSet<QString> importedIds;
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (object.isNull() || object->parent() != nullptr || object->objectId().isEmpty() ||
            targetProject->object(object->objectId()) != nullptr || importedIds.contains(object->objectId()))
        {
            error = QObject::tr("The import target changed or is unavailable.");
            return false;
        }
        importedIds.insert(object->objectId());
    }

    targetProject->beginUpdate();

    QStringList objectIds;
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() == nullptr)
        {
            targetProject->add(object.data());
            objectIds.append(object->objectId());
        }
    }
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() != nullptr)
        {
            object->resolveReferences(targetProject);
        }
    }

    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("filePath"), m_filePath);
    commandResult.insert(QStringLiteral("objectCount"), objectIds.size());
    QJsonArray ids;
    for (const QString& objectId : objectIds)
    {
        ids.append(objectId);
    }
    commandResult.insert(QStringLiteral("objectIds"), ids);
    setResult(commandResult);
    targetProject->endUpdate();
    return true;
}

bool IDOSImportFluidRockTablesCommand::revert(QString& error)
{
    IDOSProject* targetProject = project();
    if (targetProject == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }

    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (object.isNull() || targetProject->object(object->objectId()) != object.data())
        {
            error = QObject::tr("The import target changed or is unavailable.");
            return false;
        }
    }

    targetProject->beginUpdate();
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (!object.isNull() && object->parent() != nullptr)
        {
            targetProject->take(object->objectId());
        }
    }
    targetProject->endUpdate();
    return true;
}

bool IDOSImportFluidRockTablesCommand::loadObjects(QString& error)
{
    IDOSFluidRockTextProvider provider;
    QList<IDOSDataObject*> objects = provider.read(m_filePath);
    if (objects.isEmpty() || !provider.lastError().isEmpty())
    {
        error = provider.lastError().isEmpty()
            ? QObject::tr("The import file did not contain supported objects.")
            : provider.lastError();
        qDeleteAll(objects);
        return false;
    }

    IDOSSimulationCaseObject* importedTables = nullptr;
    for (IDOSDataObject* object : objects)
    {
        importedTables = qobject_cast<IDOSSimulationCaseObject*>(object);
        if (importedTables != nullptr)
        {
            break;
        }
    }
    if (importedTables == nullptr || importedTables->curveTables().isEmpty())
    {
        error = QObject::tr("The file contains no supported fluid or rock tables.");
        qDeleteAll(objects);
        return false;
    }
    const QVariantList importedCurveTables = importedTables->curveTables();
    QMap<QString, int> tableIndices;
    for (IDOSDataObject* object : project()->objects())
    {
        IDOSFluidRockTableObject* existingTable = qobject_cast<IDOSFluidRockTableObject*>(object);
        if (existingTable != nullptr)
        {
            const QVariantMap table = existingTable->tableData();
            const QString keyword = table.value(QStringLiteral("keyword")).toString().toUpper();
            tableIndices.insert(keyword, qMax(tableIndices.value(keyword),
                                              table.value(QStringLiteral("tableIndex")).toInt()));
        }
    }
    for (const QVariant& tableValue : importedCurveTables)
    {
        QVariantMap tableData = tableValue.toMap();
        const QString keyword = tableData.value(QStringLiteral("keyword")).toString().toUpper();
        const bool isFluidTable = isFluidKeyword(keyword);
        if ((m_fluidRockCategory == QStringLiteral("fluid") && !isFluidTable) ||
            (m_fluidRockCategory == QStringLiteral("rockPhysics") && isFluidTable))
        {
            continue;
        }
        const int tableIndex = tableIndices.value(keyword) + 1;
        tableIndices.insert(keyword, tableIndex);
        tableData.insert(QStringLiteral("keyword"), keyword);
        tableData.insert(QStringLiteral("tableIndex"), tableIndex);
        const IDOSFluidRockTableObject::Category category = isFluidTable
            ? IDOSFluidRockTableObject::Category::Fluid : IDOSFluidRockTableObject::Category::RockPhysics;
        IDOSFluidRockTableObject* table = new IDOSFluidRockTableObject(category);
        table->setTableData(tableData);
        table->setSourceFile(m_filePath);
        table->setName(QObject::tr("%1 — Table %2").arg(keyword).arg(tableIndex));
        m_objects.append(QPointer<IDOSDataObject>(table));
    }
    qDeleteAll(objects);
    if (m_objects.isEmpty())
    {
        error = m_fluidRockCategory == QStringLiteral("fluid")
            ? QObject::tr("The file contains no supported fluid tables.")
            : QObject::tr("The file contains no supported rock physics tables.");
        return false;
    }
    m_loaded = true;
    return true;
}

bool IDOSImportFluidRockTablesCommand::isFluidKeyword(const QString& keyword) const
{
    const QStringList fluidKeywords = {
        QStringLiteral("DENSITY"), QStringLiteral("PVTO"), QStringLiteral("PVDO"),
        QStringLiteral("PVDG"), QStringLiteral("PVTG"), QStringLiteral("PVZG"),
        QStringLiteral("PVTW")
    };
    return fluidKeywords.contains(keyword.toUpper());
}

IDOSImportFluidRockTablesCommandMetadata::IDOSImportFluidRockTablesCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("data.fluidRock.import"),
                          QObject::tr("Import fluid and rock tables"),
                          QObject::tr("Import ECLIPSE fluid and rock tables into project data."),
                          IDOSCommand::Type::Action)
{
}

IDOSImportFluidRockTablesCommandMetadata::~IDOSImportFluidRockTablesCommandMetadata() = default;

QJsonObject IDOSImportFluidRockTablesCommandMetadata::schema() const
{
    QJsonObject properties;
    properties.insert(QStringLiteral("filePath"), stringPropertySchema(QObject::tr("Path of the file to import.")));
    QJsonObject categorySchema = stringPropertySchema(
        QObject::tr("Table category to import: fluid or rockPhysics."));
    QJsonArray categories;
    categories.append(QStringLiteral("fluid"));
    categories.append(QStringLiteral("rockPhysics"));
    categorySchema.insert(QStringLiteral("enum"), categories);
    properties.insert(QStringLiteral("category"), categorySchema);
    QJsonArray required;
    required.append(QStringLiteral("filePath"));
    required.append(QStringLiteral("category"));
    return schemaWithProperties(properties, required);
}

IDOSCommand* IDOSImportFluidRockTablesCommandMetadata::create(const QJsonObject& arguments,
                                                               IDOSProject* project) const
{
    return new IDOSImportFluidRockTablesCommand(project, arguments);
}
