#include <cstddef>
#include <cstdint>
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
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QVector>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/InputErrorAction.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/io/eclipse/EInit.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/EclIOdata.hpp>

#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"

#include "idosgridpropertyeclipseprovider.h"

QStringList IDOSGridPropertyEclipseProvider::propertyKeywords() const
{
    return QStringList{
        QStringLiteral("PORO"), QStringLiteral("PERMX"), QStringLiteral("PERMY"),
        QStringLiteral("PERMZ"), QStringLiteral("MULTPV"), QStringLiteral("SIGMAV"),
        QStringLiteral("MULTX"), QStringLiteral("MULTY"), QStringLiteral("MULTZ"),
        QStringLiteral("PRESSURE"), QStringLiteral("SATNUM"), QStringLiteral("ROCKNUM"),
        QStringLiteral("PVTNUM"), QStringLiteral("EQLNUM"), QStringLiteral("FIPNUM")};
}

QList<IDOSDataObject*> IDOSGridPropertyEclipseProvider::readGridProperties(
    const Opm::Deck& deck, const QString& gridId, int nx, int ny, int nz,
    const std::vector<int>& activeCells)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;
    const int totalCellCount = nx * ny * nz;
    if (gridId.isEmpty() || nx <= 0 || ny <= 0 || nz <= 0 || totalCellCount <= 0)
    {
        return result;
    }

    int activeCellCount = 0;
    for (int activeCell : activeCells)
    {
        if (activeCell != 0)
        {
            ++activeCellCount;
        }
    }
    if (activeCells.empty())
    {
        activeCellCount = totalCellCount;
    }
    for (const QString& propertyKeyword : propertyKeywords())
    {
        const std::vector<const Opm::DeckKeyword*> matches =
            deck.getKeywordList(propertyKeyword.toStdString());
        if (matches.empty())
        {
            continue;
        }

        QVector<double> propertyValues;
        for (const Opm::DeckKeyword* match : matches)
        {
            if (match != nullptr)
            {
                propertyValues = values(*match, false);
            }
        }
        if (propertyValues.size() == activeCellCount && activeCellCount != totalCellCount)
        {
            QVector<double> expandedValues(totalCellCount, 0.0);
            int activeIndex = 0;
            for (int cellIndex = 0; cellIndex < totalCellCount; ++cellIndex)
            {
                if (activeCells.empty() || activeCells.at(static_cast<std::size_t>(cellIndex)) != 0)
                {
                    expandedValues[cellIndex] = propertyValues.at(activeIndex);
                    ++activeIndex;
                }
            }
            propertyValues = expandedValues;
        }
        if (propertyValues.size() != totalCellCount)
        {
            continue;
        }

        IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
        property->setName(propertyKeyword);
        property->setKeyword(propertyKeyword);
        property->setGridId(gridId);
        property->setKind(IDOSGridPropertyObject::Kind::Static);
        property->setValueUnits(IDOSGridPropertyObject::ValueUnits::Unknown);
        property->setDimensions(nx, ny, nz);
        if (!property->setValues(std::move(propertyValues)))
        {
            delete property;
            continue;
        }
        result.append(property);
    }
    return result;
}

QVector<double> IDOSGridPropertyEclipseProvider::values(const Opm::DeckKeyword& keyword,
                                                       bool convertToSi) const
{
    QVector<double> result;
    for (std::size_t recordIndex = 0; recordIndex < keyword.size(); ++recordIndex)
    {
        const Opm::DeckRecord& record = keyword.getRecord(recordIndex);
        for (std::size_t itemIndex = 0; itemIndex < record.size(); ++itemIndex)
        {
            const Opm::DeckItem& item = record.getItem(itemIndex);
            for (std::size_t valueIndex = 0; valueIndex < item.data_size(); ++valueIndex)
            {
                if (!item.hasValue(valueIndex))
                {
                    continue;
                }
                if (item.getType() == Opm::get_type<int>())
                {
                    result.append(static_cast<double>(item.get<int>(valueIndex)));
                }
                else if (item.getType() == Opm::get_type<double>())
                {
                    if (!convertToSi || item.getActiveDimensions().empty())
                    {
                        result.append(item.get<double>(valueIndex));
                    }
                    else
                    {
                        result.append(item.getSIDouble(valueIndex));
                    }
                }
            }
        }
    }
    return result;
}

void IDOSGridPropertyEclipseProvider::appendProperty(QList<IDOSDataObject*>* result,
                                                     const QString& keyword,
                                                     const QVector<double>& propertyValues) const
{
    if (result == nullptr || keyword.isEmpty() || propertyValues.isEmpty())
    {
        return;
    }

    IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
    property->setName(keyword);
    property->setKeyword(keyword);
    property->setKind(IDOSGridPropertyObject::Kind::Static);
    property->setDimensions(propertyValues.size(), 1, 1);
    if (!property->setValues(propertyValues))
    {
        delete property;
        return;
    }
    result->append(property);
}

QList<IDOSDataObject*> IDOSGridPropertyEclipseProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.isFile() || !fileInfo.isReadable())
    {
        setLastError(QObject::tr("The ECLIPSE grid property file is unavailable or unreadable."));
        return result;
    }

    try
    {
        if (fileInfo.suffix().compare(QStringLiteral("INIT"), Qt::CaseInsensitive) == 0)
        {
            Opm::EclIO::EInit initFile(filePath.toStdString());
            const int activeCellCount = initFile.activeCells();
            const std::vector<Opm::EclIO::EclFile::EclEntry> arrays = initFile.list_arrays();
            for (const Opm::EclIO::EclFile::EclEntry& entry : arrays)
            {
                const QString keyword = QString::fromStdString(std::get<0>(entry)).trimmed();
                const Opm::EclIO::eclArrType type = std::get<1>(entry);
                const std::int64_t arraySize = std::get<2>(entry);
                if (keyword.isEmpty() || arraySize != activeCellCount ||
                    (type != Opm::EclIO::INTE && type != Opm::EclIO::REAL && type != Opm::EclIO::DOUB))
                {
                    continue;
                }

                QVector<double> propertyValues;
                if (type == Opm::EclIO::INTE)
                {
                    const std::vector<int>& values = initFile.getInitData<int>(keyword.toStdString());
                    propertyValues.reserve(static_cast<int>(values.size()));
                    for (int value : values)
                    {
                        propertyValues.append(static_cast<double>(value));
                    }
                }
                else if (type == Opm::EclIO::REAL)
                {
                    const std::vector<float>& values = initFile.getInitData<float>(keyword.toStdString());
                    propertyValues.reserve(static_cast<int>(values.size()));
                    for (float value : values)
                    {
                        propertyValues.append(static_cast<double>(value));
                    }
                }
                else
                {
                    const std::vector<double>& values = initFile.getInitData<double>(keyword.toStdString());
                    propertyValues.reserve(static_cast<int>(values.size()));
                    for (double value : values)
                    {
                        propertyValues.append(value);
                    }
                }
                if (propertyValues.isEmpty())
                {
                    continue;
                }
                appendProperty(&result, keyword, propertyValues);
                IDOSGridPropertyObject* importedProperty = qobject_cast<IDOSGridPropertyObject*>(result.last());
                if (importedProperty != nullptr && type != Opm::EclIO::INTE)
                {
                    importedProperty->setValueUnits(IDOSGridPropertyObject::ValueUnits::Unknown);
                }
                initFile.clearData();
            }
        }
        else
        {
            Opm::Parser parser;
            Opm::ParseContext context;
            context.update(Opm::ParseContext::PARSE_MISSING_INCLUDE, Opm::InputErrorAction::THROW_EXCEPTION);
            Opm::ErrorGuard errors;
            Opm::Deck deck;
            try
            {
                deck = parser.parseFile(filePath.toStdString(), context, errors);
            }
            catch (...)
            {
                errors.clear();
                throw;
            }
            if (errors)
            {
                setLastError(QObject::tr("OPM could not parse the grid property file: %1")
                                 .arg(QString::fromStdString(errors.formattedErrors())));
                errors.clear();
                return result;
            }

            for (const QString& propertyKeyword : propertyKeywords())
            {
                const std::vector<const Opm::DeckKeyword*> matches =
                    deck.getKeywordList(propertyKeyword.toStdString());
                QVector<double> propertyValues;
                for (const Opm::DeckKeyword* match : matches)
                {
                    if (match != nullptr)
                    {
                        propertyValues = values(*match);
                    }
                }
                appendProperty(&result, propertyKeyword, propertyValues);
            }
        }
    }
    catch (const std::exception& error)
    {
        setLastError(QObject::tr("OPM failed to read the grid property file: %1")
                         .arg(QString::fromStdString(error.what())));
        for (IDOSDataObject* object : result)
        {
            delete object;
        }
        result.clear();
        return result;
    }

    if (result.isEmpty())
    {
        setLastError(QObject::tr("The file contains no supported ECLIPSE grid properties."));
    }
    return result;
}

QList<IDOSDataObject*> IDOSGridPropertyEclipseProvider::readRestart(const QString& filePath, const IDOSGridObject* grid)
{
    setLastError(QString());
    m_restartReportSteps.clear();
    QList<IDOSDataObject*> result;
    QString error;
    if (grid == nullptr || grid->totalCellCount() <= 0)
    {
        error = QObject::tr("The target grid has no valid dimensions.");
        setLastError(error);
        return {};
    }

    try
    {
        const QByteArray encodedFilePath = QFile::encodeName(QDir::toNativeSeparators(filePath));
        const std::string opmFilePath(encodedFilePath.constData(),
                                       static_cast<std::size_t>(encodedFilePath.size()));
        Opm::EclIO::ERst restartFile(opmFilePath);
        const std::vector<int>& reportSteps = restartFile.listOfReportStepNumbers();
        if (reportSteps.empty())
        {
            error = QObject::tr("The restart file contains no report steps.");
            setLastError(error);
            qDeleteAll(result);
            return {};
        }
        for (int step : reportSteps)
        {
            m_restartReportSteps.append(step);
        }
        QHash<QString, IDOSGridPropertyObject*> propertiesByKeyword;
        const QVector<int>& activeCells = grid->actnum();
        const int totalCellCount = grid->totalCellCount();
        const int activeCellCount = grid->activeCellCount();

        for (const int reportStep : reportSteps)
        {
            if (restartFile.hasArray("INTEHEAD", reportStep))
            {
                const std::vector<int>& header = restartFile.getRestartData<int>("INTEHEAD", reportStep);
                if (header.size() < 12 || header[8] != grid->nx()
                    || header[9] != grid->ny() || header[10] != grid->nz()
                    || header[11] != activeCellCount)
                {
                    setLastError(QObject::tr("The restart property %1 does not match the grid dimensions.")
                                     .arg(QStringLiteral("INTEHEAD")));
                    qDeleteAll(result);
                    return {};
                }
            }
            const std::vector<Opm::EclIO::EclFile::EclEntry> arrays =
                restartFile.listOfRstArrays(reportStep);
            QSet<QString> importedAtReportStep;
            for (const Opm::EclIO::EclFile::EclEntry& entry : arrays)
            {
                const QString keyword = QString::fromStdString(std::get<0>(entry)).trimmed().toUpper();
                if (!isSupportedRestartKeyword(keyword) || importedAtReportStep.contains(keyword))
                {
                    continue;
                }

                QVector<double> values;
                bool validValues = false;
                const Opm::EclIO::eclArrType arrayType = std::get<1>(entry);
                if (arrayType == Opm::EclIO::REAL)
                {
                    const std::vector<float>& sourceValues =
                        restartFile.getRestartData<float>(keyword.toStdString(), reportStep);
                    if (sourceValues.empty())
                    {
                        continue;
                    }
                    validValues = expandRestartValues(sourceValues, activeCells, totalCellCount, activeCellCount, &values);
                }
                else if (arrayType == Opm::EclIO::DOUB)
                {
                    const std::vector<double>& sourceValues =
                        restartFile.getRestartData<double>(keyword.toStdString(), reportStep);
                    if (sourceValues.empty())
                    {
                        continue;
                    }
                    validValues = expandRestartValues(sourceValues, activeCells, totalCellCount, activeCellCount, &values);
                }
                else
                {
                    continue;
                }
                if (!validValues)
                {
                    error = QObject::tr("The restart property %1 does not match the grid dimensions.").arg(keyword);
                    setLastError(error);
                    qDeleteAll(result);
                    return {};
                }

                IDOSGridPropertyObject* property = propertiesByKeyword.value(keyword, nullptr);
                if (property == nullptr)
                {
                    const QString propertyName = keyword;
                    property = new IDOSGridPropertyObject();
                    property->setGridId(grid->objectId());
                    property->setKind(IDOSGridPropertyObject::Kind::Dynamic);
                    property->setKeyword(keyword);
                    property->setDimensions(grid->nx(), grid->ny(), grid->nz());
                    property->setName(propertyName);
                    result.append(property);
                    propertiesByKeyword.insert(keyword, property);
                }

                if (!property->setReportStepValues(reportStep, values))
                {
                    error = QObject::tr("Unable to store report step %1 for property %2.")
                        .arg(reportStep)
                        .arg(keyword);
                    setLastError(error);
                    qDeleteAll(result);
                    return {};
                }
                importedAtReportStep.insert(keyword);
            }
            // Final Qt arrays own the converted values; retain only the OPM file index.
            restartFile.clearData();
        }

        if (result.isEmpty())
        {
            error = QObject::tr("No supported cell properties were found in the restart file.");
            setLastError(error);
            qDeleteAll(result);
            return {};
        }
    }
    catch (const std::exception& exception)
    {
        error = QObject::tr("Unable to read the restart file: %1")
            .arg(QString::fromUtf8(exception.what()));
        setLastError(error);
        qDeleteAll(result);
        return {};
    }
    return result;
}

bool IDOSGridPropertyEclipseProvider::isSupportedRestartKeyword(const QString& keyword) const
{
    const QStringList keywords = QStringList()
        << QStringLiteral("PRESSURE") << QStringLiteral("SWAT") << QStringLiteral("SGAS")
        << QStringLiteral("SOIL") << QStringLiteral("RS") << QStringLiteral("RV")
        << QStringLiteral("PBUB") << QStringLiteral("PDEW") << QStringLiteral("BO")
        << QStringLiteral("BG") << QStringLiteral("BW") << QStringLiteral("KRO")
        << QStringLiteral("KRW") << QStringLiteral("KRG");
    return keywords.contains(keyword);
}

QList<int> IDOSGridPropertyEclipseProvider::restartReportSteps() const
{
    return m_restartReportSteps;
}

template<typename Value>
bool IDOSGridPropertyEclipseProvider::expandRestartValues(const std::vector<Value>& source,
                                                         const QVector<int>& activeCells,
                                                         int totalCellCount, int activeCellCount,
                                                         QVector<double>* values)
{
    const bool full = source.size() == static_cast<std::size_t>(totalCellCount);
    if (!full && source.size() != static_cast<std::size_t>(activeCellCount))
    {
        return false;
    }
    // Allocate the final size exactly; resize() may otherwise over-allocate large arrays.
    *values = QVector<double>(totalCellCount);
    double* target = values->data();
    if (full)
    {
        for (int index = 0; index < totalCellCount; ++index)
        {
            target[index] = static_cast<double>(source[static_cast<std::size_t>(index)]);
        }
    }
    else
    {
        std::size_t activeIndex = 0;
        for (int index = 0; index < totalCellCount; ++index)
        {
            target[index] = activeCells.isEmpty() || activeCells.at(index) != 0
                ? static_cast<double>(source[activeIndex++]) : 0.0;
        }
    }
    return true;
}
