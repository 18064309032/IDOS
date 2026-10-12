#include <cstddef>
#include <exception>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QVector>

#include <opm/io/eclipse/EclFile.hpp>
#include <opm/io/eclipse/ERst.hpp>

#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"

#include "idosimportrestartresultscommand.h"

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
    if (!fileInfo.isFile() || !fileInfo.isReadable())
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
    IDOSGrid* grid = targetProject != nullptr
        ? qobject_cast<IDOSGrid*>(targetProject->object(m_gridId))
        : nullptr;
    if (grid == nullptr || grid->totalCellCount() <= 0)
    {
        error = QObject::tr("The target grid has no valid dimensions.");
        return false;
    }

    try
    {
        const QByteArray encodedFilePath = QFile::encodeName(QDir::toNativeSeparators(m_filePath));
        const std::string opmFilePath(encodedFilePath.constData(),
                                       static_cast<std::size_t>(encodedFilePath.size()));
        Opm::EclIO::ERst restartFile(opmFilePath);
        const std::vector<int>& reportSteps = restartFile.listOfReportStepNumbers();
        if (reportSteps.empty())
        {
            error = QObject::tr("The restart file contains no report steps.");
            return false;
        }
        m_firstReportStep = reportSteps.front();
        m_lastReportStep = reportSteps.back();
        m_reportStepCount = static_cast<int>(reportSteps.size());
        QHash<QString, IDOSGridProperty*> propertiesByKeyword;
        const QVector<int>& activeCells = grid->actnum();
        const int totalCellCount = grid->totalCellCount();

        for (const int reportStep : reportSteps)
        {
            restartFile.loadReportStepNumber(reportStep);
            const std::vector<Opm::EclIO::EclFile::EclEntry> arrays =
                restartFile.listOfRstArrays(reportStep);
            QSet<QString> importedAtReportStep;
            for (const Opm::EclIO::EclFile::EclEntry& entry : arrays)
            {
                const QString keyword = QString::fromStdString(std::get<0>(entry)).trimmed().toUpper();
                if (!isSupportedKeyword(keyword) || importedAtReportStep.contains(keyword))
                {
                    continue;
                }

                QVector<double> rawValues;
                const Opm::EclIO::eclArrType arrayType = std::get<1>(entry);
                if (arrayType == Opm::EclIO::REAL)
                {
                    const std::vector<float>& sourceValues =
                        restartFile.getRestartData<float>(keyword.toStdString(), reportStep);
                    rawValues.reserve(static_cast<int>(sourceValues.size()));
                    for (float value : sourceValues)
                    {
                        rawValues.append(static_cast<double>(value));
                    }
                }
                else if (arrayType == Opm::EclIO::DOUB)
                {
                    const std::vector<double>& sourceValues =
                        restartFile.getRestartData<double>(keyword.toStdString(), reportStep);
                    rawValues.reserve(static_cast<int>(sourceValues.size()));
                    for (double value : sourceValues)
                    {
                        rawValues.append(value);
                    }
                }
                else
                {
                    continue;
                }

                if (rawValues.isEmpty())
                {
                    continue;
                }

                QVector<double> values(totalCellCount, 0.0);
                if (rawValues.size() == totalCellCount)
                {
                    values = rawValues;
                }
                else if (rawValues.size() == grid->activeCellCount())
                {
                    int activeIndex = 0;
                    for (int cellIndex = 0; cellIndex < totalCellCount; ++cellIndex)
                    {
                        if (activeCells.isEmpty() || activeCells.at(cellIndex) != 0)
                        {
                            values[cellIndex] = rawValues.at(activeIndex);
                            ++activeIndex;
                        }
                    }
                }
                else
                {
                    continue;
                }

                IDOSGridProperty* property = propertiesByKeyword.value(keyword, nullptr);
                if (property == nullptr)
                {
                    const QString propertyName = keyword;
                    for (IDOSDataObject* object : targetProject->objects())
                    {
                        IDOSGridProperty* existingProperty = qobject_cast<IDOSGridProperty*>(object);
                        if (existingProperty != nullptr && existingProperty->gridId() == m_gridId
                            && existingProperty->kind() == IDOSGridProperty::Kind::Dynamic
                            && (existingProperty->keyword().compare(keyword, Qt::CaseInsensitive) == 0
                                || existingProperty->name() == propertyName))
                        {
                            error = QObject::tr("Result property %1 already exists. Remove it before importing again.")
                                .arg(keyword);
                            return false;
                        }
                    }

                    property = new IDOSGridProperty();
                    property->setGridId(m_gridId);
                    property->setKind(IDOSGridProperty::Kind::Dynamic);
                    property->setKeyword(keyword);
                    property->setDimensions(grid->nx(), grid->ny(), grid->nz());
                    property->setName(propertyName);
                    m_properties.append(QPointer<IDOSGridProperty>(property));
                    propertiesByKeyword.insert(keyword, property);
                }

                if (!property->setReportStepValues(reportStep, values))
                {
                    error = QObject::tr("Unable to store report step %1 for property %2.")
                        .arg(reportStep)
                        .arg(keyword);
                    return false;
                }
                importedAtReportStep.insert(keyword);
            }
        }

        if (m_properties.isEmpty())
        {
            error = QObject::tr("No supported cell properties were found in the restart file.");
            return false;
        }
    }
    catch (const std::exception& exception)
    {
        deleteDetachedProperties();
        error = QObject::tr("Unable to read the restart file: %1")
            .arg(QString::fromUtf8(exception.what()));
        return false;
    }

    m_loaded = true;
    return true;
}

bool IDOSImportRestartResultsCommand::isSupportedKeyword(const QString& keyword) const
{
    const QStringList keywords = QStringList()
        << QStringLiteral("PRESSURE") << QStringLiteral("SWAT") << QStringLiteral("SGAS")
        << QStringLiteral("SOIL") << QStringLiteral("RS") << QStringLiteral("RV")
        << QStringLiteral("PBUB") << QStringLiteral("PDEW") << QStringLiteral("BO")
        << QStringLiteral("BG") << QStringLiteral("BW") << QStringLiteral("KRO")
        << QStringLiteral("KRW") << QStringLiteral("KRG");
    return keywords.contains(keyword);
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
