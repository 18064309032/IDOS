#include <algorithm>
#include <cmath>
#include <exception>
#include <filesystem>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QUuid>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/FileDeck.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/InputErrorAction.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Units/Dimension.hpp>

#include "data/case/idoscaseitemref.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "data/well/idoswellobject.h"
#include "idosproject.h"

#include "idossimulationcaseeclipseprovider.h"

bool IDOSSimulationCaseEclipseProvider::writeMemoryData(const IDOSSimulationCaseObject* simulationCase,
                                                        const IDOSProject* project,
                                                        const QString& outputDirectory,
                                                        const QString& fileName,
                                                        QString* error) const
{
    if (simulationCase == nullptr || project == nullptr || error == nullptr)
    {
        return false;
    }

    const QFileInfo sourceInfo(simulationCase->sourceFile());
    if (!sourceInfo.isFile() || !sourceInfo.isReadable())
    {
        *error = QObject::tr("The source DATA file is missing or unreadable.");
        return false;
    }

    try
    {
        std::unique_ptr<Opm::Deck> parsedDeck;
        QString parseError;
        if (!parseDeckFile(sourceInfo.absoluteFilePath(), &parsedDeck, &parseError))
        {
            *error = parseError;
            return false;
        }
        const Opm::Deck& deck = *parsedDeck;
        Opm::Parser parser;
        if (!configureParser(&parser, error))
        {
            return false;
        }

        IDOSGridObject* grid = nullptr;
        QList<IDOSGridPropertyObject*> properties;
        QList<IDOSWellObject*> referencedWells;
        for (const IDOSCaseItemRef& reference : simulationCase->itemRefs())
        {
            IDOSDataObject* object = project->object(reference.objectId());
            if (reference.role() == QStringLiteral("case.grid"))
            {
                IDOSGridObject* referencedGrid = qobject_cast<IDOSGridObject*>(object);
                if (referencedGrid == nullptr || grid != nullptr)
                {
                    *error = QObject::tr("The case must reference exactly one valid grid.");
                    return false;
                }
                grid = referencedGrid;
            }
            else if (reference.role() == QStringLiteral("case.gridProperty"))
            {
                IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
                if (property == nullptr)
                {
                    *error = QObject::tr("A referenced grid property is missing or invalid.");
                    return false;
                }
                properties.append(property);
            }
            else if (reference.role() == QStringLiteral("case.well"))
            {
                IDOSWellObject* well = qobject_cast<IDOSWellObject*>(object);
                if (well == nullptr)
                {
                    *error = QObject::tr("A referenced case well is missing or invalid.");
                    return false;
                }
                referencedWells.append(well);
            }
        }
        if (grid == nullptr || grid->type() != IDOSGridObject::Type::CornerPoint)
        {
            *error = QObject::tr("Memory DATA generation currently requires one corner-point grid.");
            return false;
        }

        if (properties.isEmpty())
        {
            for (IDOSDataObject* object : project->objects())
            {
                IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
                if (property != nullptr && property->gridId() == grid->objectId() &&
                    property->kind() == IDOSGridPropertyObject::Kind::Static)
                {
                    properties.append(property);
                }
            }
        }

        const qint64 nx = grid->nx();
        const qint64 ny = grid->ny();
        const qint64 nz = grid->nz();
        if (nx <= 0 || ny <= 0 || nz <= 0 || nx > std::numeric_limits<int>::max() / ny ||
            nx > std::numeric_limits<int>::max() / 6 / (ny + 1) - 1)
        {
            *error = QObject::tr("The in-memory grid dimensions and arrays are inconsistent.");
            return false;
        }
        const qint64 cellsPerLayer = nx * ny;
        if (cellsPerLayer > std::numeric_limits<int>::max() / nz ||
            cellsPerLayer * nz > std::numeric_limits<int>::max() / 8)
        {
            *error = QObject::tr("The in-memory grid dimensions and arrays are inconsistent.");
            return false;
        }
        const qint64 cellCount = cellsPerLayer * nz;
        const qint64 coordCount = 6 * (nx + 1) * (ny + 1);
        if (coordCount != grid->coord().size() || 8 * cellCount != grid->zcorn().size() ||
            cellCount != grid->actnum().size())
        {
            *error = QObject::tr("The in-memory grid dimensions and arrays are inconsistent.");
            return false;
        }

        const QStringList doubleKeywords = {
            QStringLiteral("PORO"), QStringLiteral("PERMX"), QStringLiteral("PERMY"), QStringLiteral("PERMZ"),
            QStringLiteral("MULTPV"), QStringLiteral("SIGMAV"), QStringLiteral("MULTX"), QStringLiteral("MULTY"),
            QStringLiteral("MULTZ")};
        const QStringList integerKeywords = {
            QStringLiteral("SATNUM"), QStringLiteral("ROCKNUM"), QStringLiteral("PVTNUM"),
            QStringLiteral("EQLNUM"), QStringLiteral("FIPNUM")};
        const QStringList propertyKeywords = doubleKeywords + integerKeywords + QStringList{QStringLiteral("ACTNUM")};
        QMap<QString, IDOSGridPropertyObject*> propertiesByKeyword;
        for (IDOSGridPropertyObject* property : properties)
        {
            const QString keyword = property->keyword().trimmed().toUpper();
            if (property->valueUnits() != IDOSGridPropertyObject::ValueUnits::SI)
            {
                *error = QObject::tr("The unit system of property %1 is unknown; it cannot be exported safely.")
                             .arg(keyword);
                return false;
            }
            if (property->kind() != IDOSGridPropertyObject::Kind::Static || property->gridId() != grid->objectId() ||
                property->numX() != grid->nx() || property->numY() != grid->ny() ||
                property->numZ() != grid->nz() || property->values().size() != cellCount ||
                !propertyKeywords.contains(keyword) ||
                propertiesByKeyword.contains(keyword))
            {
                *error = QObject::tr("A referenced property is unsupported, duplicated or has incompatible dimensions: %1")
                             .arg(keyword);
                return false;
            }
            for (double value : property->values())
            {
                const bool integerProperty = integerKeywords.contains(keyword) || keyword == QStringLiteral("ACTNUM");
                const bool invalidInteger = integerProperty &&
                                            (std::floor(value) != value || value < 0.0 ||
                                             value > std::numeric_limits<int>::max());
                const bool invalidActnum = keyword == QStringLiteral("ACTNUM") && value > 1.0;
                if (!std::isfinite(value) || invalidInteger || invalidActnum)
                {
                    *error = QObject::tr("A property contains an invalid value or an out-of-range integer: %1")
                                 .arg(keyword);
                    return false;
                }
            }
            propertiesByKeyword.insert(keyword, property);
        }

        Opm::FileDeck fileDeck(deck);

        QSet<QString> sourceWellNames;
        const std::vector<const Opm::DeckKeyword*> sourceWellKeywords = deck.getKeywordList("WELSPECS");
        for (const Opm::DeckKeyword* keyword : sourceWellKeywords)
        {
            if (keyword == nullptr)
            {
                continue;
            }
            for (std::size_t recordIndex = 0; recordIndex < keyword->size(); ++recordIndex)
            {
                const Opm::DeckRecord& record = keyword->getRecord(recordIndex);
                if (record.size() > 0 && record.getItem(0).data_size() > 0 && record.getItem(0).hasValue(0))
                {
                    sourceWellNames.insert(QString::fromStdString(record.getItem(0).getTrimmedString(0))
                                               .toCaseFolded());
                }
            }
        }
        QStringList wellsMissingFromSource;
        for (IDOSWellObject* well : referencedWells)
        {
            if (!sourceWellNames.contains(well->name().toCaseFolded()))
            {
                wellsMissingFromSource.append(well->name());
            }
        }
        if (!wellsMissingFromSource.isEmpty())
        {
            wellsMissingFromSource.removeDuplicates();
            *error = QObject::tr("The following wells are linked to the case but are not defined in the source DATA file: %1. "
                                 "They cannot be exported until their grid locations, completion layers and schedule controls are provided.")
                         .arg(wellsMissingFromSource.join(QStringLiteral(", ")));
            return false;
        }

        const QStringList replacedKeywords = doubleKeywords + integerKeywords +
                                             QStringList{QStringLiteral("DIMENS"), QStringLiteral("COORD"),
                                                         QStringLiteral("ZCORN"), QStringLiteral("ACTNUM"),
                                                         QStringLiteral("MULT_XYZ")};
        for (const QString& name : replacedKeywords)
        {
            std::vector<Opm::FileDeck::Index> matches;
            Opm::FileDeck::Index index = fileDeck.start();
            while (index != fileDeck.stop())
            {
                if (QString::fromStdString(fileDeck[index].name()) == name)
                {
                    matches.push_back(index);
                }
                ++index;
            }
            for (std::vector<Opm::FileDeck::Index>::reverse_iterator iterator = matches.rbegin();
                 iterator != matches.rend(); ++iterator)
            {
                fileDeck.erase(*iterator);
            }
        }

        const Opm::UnitSystem& units = deck.getActiveUnitSystem();
        Opm::DeckKeyword dimensionsKeyword(parser.getKeyword("DIMENS"));
        Opm::DeckRecord dimensionsRecord;
        dimensionsRecord.addItem(Opm::DeckItem("NX", grid->nx()));
        dimensionsRecord.addItem(Opm::DeckItem("NY", grid->ny()));
        dimensionsRecord.addItem(Opm::DeckItem("NZ", grid->nz()));
        dimensionsKeyword.addRecord(std::move(dimensionsRecord));
        std::optional<Opm::FileDeck::Index> runspecIndex = fileDeck.find("RUNSPEC");
        std::optional<Opm::FileDeck::Index> gridIndex = fileDeck.find("GRID");
        std::optional<Opm::FileDeck::Index> propsIndex = fileDeck.find("PROPS");
        std::optional<Opm::FileDeck::Index> regionsIndex = fileDeck.find("REGIONS");
        if (!runspecIndex.has_value() || !gridIndex.has_value() || !propsIndex.has_value())
        {
            *error = QObject::tr("The source DATA file is missing RUNSPEC, GRID or PROPS section.");
            return false;
        }
        Opm::FileDeck::Index insertionIndex = runspecIndex.value();
        ++insertionIndex;
        fileDeck.insert(insertionIndex, dimensionsKeyword);

        std::vector<double> coordValues;
        coordValues.reserve(static_cast<std::size_t>(grid->coord().size()));
        for (double value : grid->coord())
        {
            coordValues.push_back(value);
        }
        std::vector<double> zcornValues;
        zcornValues.resize(static_cast<std::size_t>(grid->zcorn().size()));
        const Opm::ZcornMapper zcornMapper(static_cast<std::size_t>(grid->nx()),
                                           static_cast<std::size_t>(grid->ny()),
                                           static_cast<std::size_t>(grid->nz()));
        const int opmCorners[8] = {0, 1, 3, 2, 4, 5, 7, 6};
        for (int k = 0; k < grid->nz(); ++k)
        {
            for (int j = 0; j < grid->ny(); ++j)
            {
                for (int i = 0; i < grid->nx(); ++i)
                {
                    const qint64 cellBase = (i + j * grid->nx() + k * cellsPerLayer) * 8;
                    for (int corner = 0; corner < 8; ++corner)
                    {
                        zcornValues[zcornMapper.index(static_cast<std::size_t>(i),
                                                      static_cast<std::size_t>(j),
                                                      static_cast<std::size_t>(k), opmCorners[corner])] =
                            grid->zcorn().at(static_cast<int>(cellBase + corner));
                    }
                }
            }
        }
        Opm::DeckKeyword coordKeyword(parser.getKeyword("COORD"), coordValues, units, deck.getDefaultUnitSystem());
        Opm::DeckKeyword zcornKeyword(parser.getKeyword("ZCORN"), zcornValues, units, deck.getDefaultUnitSystem());
        std::vector<int> actnumValues = grid->actnum().toStdVector();
        IDOSGridPropertyObject* actnumProperty = propertiesByKeyword.value(QStringLiteral("ACTNUM"), nullptr);
        if (actnumProperty != nullptr)
        {
            actnumValues.clear();
            actnumValues.reserve(static_cast<std::size_t>(actnumProperty->values().size()));
            for (double value : actnumProperty->values())
            {
                actnumValues.push_back(static_cast<int>(value));
            }
        }
        Opm::DeckKeyword actnumKeyword(parser.getKeyword("ACTNUM"), actnumValues);
        gridIndex = fileDeck.find("GRID");
        if (!gridIndex.has_value())
        {
            *error = QObject::tr("The GRID section could not be located after replacing source keywords.");
            return false;
        }
        insertionIndex = gridIndex.value();
        ++insertionIndex;
        fileDeck.insert(insertionIndex, coordKeyword);
        ++insertionIndex;
        fileDeck.insert(insertionIndex, zcornKeyword);
        ++insertionIndex;
        fileDeck.insert(insertionIndex, actnumKeyword);
        ++insertionIndex;

        propsIndex = fileDeck.find("PROPS");
        if (!propsIndex.has_value())
        {
            *error = QObject::tr("The PROPS section could not be located after replacing source keywords.");
            return false;
        }
        QStringList familyKeywordNames;
        for (const QString& keyword : doubleKeywords)
        {
            IDOSGridPropertyObject* property = propertiesByKeyword.value(keyword, nullptr);
            if (property == nullptr)
            {
                continue;
            }
            std::vector<double> values;
            values.reserve(static_cast<std::size_t>(property->values().size()));
            for (double value : property->values())
            {
                values.push_back(value);
            }
            Opm::DeckKeyword propertyKeyword(parser.getKeyword(keyword.toStdString()), values, units,
                                             deck.getDefaultUnitSystem());
            if (QString::fromStdString(propertyKeyword.name()) == keyword)
            {
                fileDeck.insert(insertionIndex, propertyKeyword);
            }
            else
            {
                fileDeck.insert(insertionIndex, propertyKeyword);
                familyKeywordNames.append(keyword);
            }
            ++insertionIndex;
        }

        bool hasIntegerProperties = false;
        for (const QString& keyword : integerKeywords)
        {
            hasIntegerProperties = hasIntegerProperties || propertiesByKeyword.contains(keyword);
        }
        if (hasIntegerProperties)
        {
            regionsIndex = fileDeck.find("REGIONS");
            if (!regionsIndex.has_value())
            {
                *error = QObject::tr("The source DATA file must contain a REGIONS section for integer region properties.");
                return false;
            }
            insertionIndex = regionsIndex.value();
            ++insertionIndex;
            for (const QString& keyword : integerKeywords)
            {
                IDOSGridPropertyObject* property = propertiesByKeyword.value(keyword, nullptr);
                if (property == nullptr)
                {
                    continue;
                }
                std::vector<int> values;
                values.reserve(static_cast<std::size_t>(property->values().size()));
                for (double value : property->values())
                {
                    values.push_back(static_cast<int>(value));
                }
                Opm::DeckKeyword propertyKeyword(parser.getKeyword(keyword.toStdString()), values);
                fileDeck.insert(insertionIndex, propertyKeyword);
                ++insertionIndex;
            }
        }

        if (!QDir().mkpath(outputDirectory))
        {
            *error = QObject::tr("Unable to create the DATA output directory.");
            return false;
        }
        fileDeck.dump(outputDirectory.toStdString(), fileName.toStdString(), Opm::FileDeck::OutputMode::INLINE);
        const QString outputPath = QDir(outputDirectory).absoluteFilePath(fileName);
        if (!QFileInfo(outputPath).isFile())
        {
            *error = QObject::tr("OPM did not create the generated DATA file.");
            return false;
        }
        if (!familyKeywordNames.isEmpty())
        {
            QFile generatedFile(outputPath);
            if (!generatedFile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                *error = QObject::tr("Unable to update aliased grid property keywords in the generated DATA file.");
                return false;
            }
            QStringList generatedLines = QString::fromUtf8(generatedFile.readAll()).split(QLatin1Char('\n'));
            generatedFile.close();
            int familyKeywordIndex = 0;
            for (QString& line : generatedLines)
            {
                if (line.trimmed().compare(QStringLiteral("MULT_XYZ"), Qt::CaseInsensitive) == 0 &&
                    familyKeywordIndex < familyKeywordNames.size())
                {
                    line = familyKeywordNames.at(familyKeywordIndex);
                    ++familyKeywordIndex;
                }
            }
            if (familyKeywordIndex != familyKeywordNames.size() ||
                !generatedFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
            {
                *error = QObject::tr("Unable to update aliased grid property keywords in the generated DATA file.");
                return false;
            }
            generatedFile.write(generatedLines.join(QLatin1Char('\n')).toUtf8());
            generatedFile.close();
        }
        std::unique_ptr<Opm::Deck> validatedDeck;
        QString validationError;
        if (!parseDeckFile(outputPath, &validatedDeck, &validationError))
        {
            QFile::remove(outputPath);
            *error = QObject::tr("The generated DATA file failed validation: %1")
                         .arg(validationError);
            return false;
        }
        QFile validationFile(outputPath);
        if (!validationFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QFile::remove(outputPath);
            *error = QObject::tr("The generated DATA file could not be read for property validation.");
            return false;
        }
        const QStringList generatedLines = QString::fromUtf8(validationFile.readAll()).split(QLatin1Char('\n'));
        QSet<QString> generatedKeywords;
        for (const QString& line : generatedLines)
        {
            const QString keyword = line.trimmed().toUpper();
            if (!keyword.isEmpty())
            {
                generatedKeywords.insert(keyword);
            }
        }
        QStringList missingProperties;
        for (QMap<QString, IDOSGridPropertyObject*>::const_iterator iterator = propertiesByKeyword.constBegin();
             iterator != propertiesByKeyword.constEnd(); ++iterator)
        {
            if (!generatedKeywords.contains(iterator.key()))
            {
                missingProperties.append(iterator.key());
            }
        }
        if (!missingProperties.isEmpty())
        {
            QFile::remove(outputPath);
            *error = QObject::tr("The generated DATA file is missing referenced grid properties: %1")
                         .arg(missingProperties.join(QStringLiteral(", ")));
            return false;
        }
        return true;
    }
    catch (const std::exception& exception)
    {
        *error = QString::fromUtf8(exception.what());
        return false;
    }
}

QVariantList IDOSSimulationCaseEclipseProvider::buildDeckKeywordIndex(const Opm::Deck& deck,
                                                                       const QString& sourceFilePath,
                                                                       const QString& parserFilePath) const
{
    QVariantList result;
    QVector<QVariantMap> keywordLocations;
    keywordLocations.reserve(static_cast<int>(deck.size()));
    QMap<QString, qulonglong> nextLineByFile;
    for (std::size_t index = deck.size(); index > 0; --index)
    {
        const Opm::DeckKeyword& deckKeyword = deck[index - 1];
        const Opm::KeywordLocation& location = deckKeyword.location();
        const QString filePath = QString::fromStdString(location.filename);
        QString fileName = QDir::cleanPath(QFileInfo(filePath).absoluteFilePath());
        const QString normalizedParserPath = QDir::cleanPath(QFileInfo(parserFilePath).absoluteFilePath());
        if (QString::compare(fileName, normalizedParserPath, Qt::CaseInsensitive) == 0)
        {
            fileName = QDir::cleanPath(QFileInfo(sourceFilePath).absoluteFilePath());
        }
        const qulonglong line = static_cast<qulonglong>(location.lineno);
        const qulonglong nextLine = nextLineByFile.value(fileName, 0);

        QVariantMap keyword;
        keyword.insert(QStringLiteral("name"), QString::fromStdString(deckKeyword.name()));
        keyword.insert(QStringLiteral("file"), fileName);
        keyword.insert(QStringLiteral("line"), line);
        keyword.insert(QStringLiteral("endLine"), nextLine > line ? nextLine - 1 : (nextLine == line ? line : 0));
        keywordLocations.append(keyword);
        nextLineByFile.insert(fileName, line);
    }

    std::reverse(keywordLocations.begin(), keywordLocations.end());
    for (const QVariantMap& keyword : keywordLocations)
    {
        result.append(keyword);
    }
    return result;
}

QVariantMap IDOSSimulationCaseEclipseProvider::buildParsedSourceData(const Opm::Deck& deck,
                                                                     const QString& filePath,
                                                                     const QString& parserFilePath) const
{
    QVariantMap data;
    data.insert(QStringLiteral("summary"), buildSummary(deck, filePath));
    data.insert(QStringLiteral("keywordIndex"), buildDeckKeywordIndex(deck, filePath, parserFilePath));
    return data;
}

QVariantList IDOSSimulationCaseEclipseProvider::buildRelativePermeabilityTables(const Opm::Deck& deck) const
{
    const QStringList keywords = {
        QStringLiteral("SWOF"), QStringLiteral("SGOF"), QStringLiteral("SWFN"), QStringLiteral("SGFN")};
    const QMap<QString, QStringList> columnsByKeyword = {
        {QStringLiteral("SWOF"), QStringList() << QStringLiteral("Sw") << QStringLiteral("Krw")
                                                << QStringLiteral("Krow") << QStringLiteral("Pcow")},
        {QStringLiteral("SGOF"), QStringList() << QStringLiteral("Sg") << QStringLiteral("Krg")
                                                << QStringLiteral("Krog") << QStringLiteral("Pcg")},
        {QStringLiteral("SWFN"), QStringList() << QStringLiteral("Sw") << QStringLiteral("Krw")
                                                << QStringLiteral("Pcow")},
        {QStringLiteral("SGFN"), QStringList() << QStringLiteral("Sg") << QStringLiteral("Krg")
                                                << QStringLiteral("Pcg")}};
    QVariantList result;
    QMap<QString, int> tableIndices;
    for (const QString& keywordName : keywords)
    {
        const std::vector<const Opm::DeckKeyword*> matches = deck.getKeywordList(keywordName.toStdString());
        for (const Opm::DeckKeyword* keyword : matches)
        {
            if (keyword == nullptr)
            {
                continue;
            }

            QVariantList rows;
            for (std::size_t recordIndex = 0; recordIndex < keyword->size(); ++recordIndex)
            {
                const Opm::DeckRecord& record = keyword->getRecord(recordIndex);
                QVariantList row;
                const QStringList columns = columnsByKeyword.value(keywordName);
                for (int columnIndex = 0; columnIndex < columns.size(); ++columnIndex)
                {
                    if (static_cast<std::size_t>(columnIndex) >= record.size())
                    {
                        row.append(QVariant());
                        continue;
                    }

                    const Opm::DeckItem& item = record.getItem(static_cast<std::size_t>(columnIndex));
                    if (item.data_size() == 0 || !item.hasValue(0))
                    {
                        row.append(QVariant());
                    }
                    else
                    {
                        row.append(item.getSIDouble(0));
                    }
                }
                if (!row.isEmpty())
                {
                    rows.append(row);
                }
            }
            if (rows.isEmpty())
            {
                continue;
            }

            QVariantMap table;
            table.insert(QStringLiteral("keyword"), keywordName);
            table.insert(QStringLiteral("tableIndex"), tableIndices.value(keywordName, 0) + 1);
            table.insert(QStringLiteral("columns"), columnsByKeyword.value(keywordName));
            table.insert(QStringLiteral("rows"), rows);
            tableIndices.insert(keywordName, tableIndices.value(keywordName, 0) + 1);
            result.append(table);
        }
    }
    return result;
}

QVariantList IDOSSimulationCaseEclipseProvider::deckItemValues(const Opm::DeckItem& item) const
{
    QVariantList values;
    for (std::size_t valueIndex = 0; valueIndex < item.data_size(); ++valueIndex)
    {
        values.append(deckItemValue(item, valueIndex));
    }
    return values;
}

QVariant IDOSSimulationCaseEclipseProvider::deckItemValue(const Opm::DeckItem& item,
                                                          std::size_t valueIndex) const
{
    if (!item.hasValue(valueIndex))
    {
        return QVariant();
    }
    if (item.getType() == Opm::get_type<int>())
    {
        return item.get<int>(valueIndex);
    }
    if (item.getType() == Opm::get_type<double>())
    {
        if (item.getActiveDimensions().empty())
        {
            return item.get<double>(valueIndex);
        }
        return item.getSIDouble(valueIndex);
    }
    if (item.getType() == Opm::get_type<std::string>())
    {
        return QString::fromStdString(item.getTrimmedString(valueIndex));
    }
    if (item.getType() == Opm::get_type<Opm::RawString>())
    {
        return QString::fromStdString(item.get<Opm::RawString>(valueIndex));
    }
    if (item.getType() == Opm::get_type<Opm::UDAValue>())
    {
        const Opm::UDAValue value = item.get<Opm::UDAValue>(valueIndex);
        if (!value.is_defined())
        {
            return QVariant();
        }
        if (value.is_numeric())
        {
            return value.getSI();
        }
        return QString::fromStdString(value.get<std::string>());
    }
    return QString::fromStdString(item.getTrimmedString(valueIndex));
}

QVariantList IDOSSimulationCaseEclipseProvider::buildWellControlRows(const Opm::Deck& deck) const
{
    const QStringList controlKeywords = {
        QStringLiteral("WELSPECS"), QStringLiteral("WCONPROD"), QStringLiteral("WCONINJE")};
    QVariantList rows;
    int scheduleStep = 0;
    QString stepType = QStringLiteral("initial");
    QString stepValue;
    for (std::size_t keywordIndex = 0; keywordIndex < deck.size(); ++keywordIndex)
    {
        const Opm::DeckKeyword& keyword = deck[keywordIndex];
        const QString keywordName = QString::fromStdString(keyword.name());
        if (keywordName == QStringLiteral("DATES"))
        {
            for (std::size_t recordIndex = 0; recordIndex < keyword.size(); ++recordIndex)
            {
                const Opm::DeckRecord& record = keyword.getRecord(recordIndex);
                if (record.size() < 3)
                {
                    continue;
                }
                const QString day = deckItemValues(record.getItem(0)).value(0).toString();
                const QString month = deckItemValues(record.getItem(1)).value(0).toString();
                const QString year = deckItemValues(record.getItem(2)).value(0).toString();
                ++scheduleStep;
                stepType = QStringLiteral("date");
                stepValue = QStringLiteral("%1 %2 %3").arg(day).arg(month).arg(year);
            }
            continue;
        }
        if (keywordName == QStringLiteral("TSTEP"))
        {
            for (std::size_t recordIndex = 0; recordIndex < keyword.size(); ++recordIndex)
            {
                const Opm::DeckRecord& record = keyword.getRecord(recordIndex);
                if (record.size() == 0)
                {
                    continue;
                }
                const QVariantList dayValues = deckItemValues(record.getItem(0));
                for (const QVariant& dayValue : dayValues)
                {
                    ++scheduleStep;
                    stepType = QStringLiteral("days");
                    stepValue = dayValue.toString();
                }
            }
            continue;
        }
        if (!controlKeywords.contains(keywordName))
        {
            continue;
        }

        for (std::size_t recordIndex = 0; recordIndex < keyword.size(); ++recordIndex)
        {
            const Opm::DeckRecord& record = keyword.getRecord(recordIndex);
            QVariantMap fields;
            for (std::size_t itemIndex = 0; itemIndex < record.size(); ++itemIndex)
            {
                const Opm::DeckItem& item = record.getItem(itemIndex);
                const QVariantList values = deckItemValues(item);
                fields.insert(QString::fromStdString(item.name()), values.size() == 1 ? values.first() : QVariant(values));
            }

            QVariantMap row;
            row.insert(QStringLiteral("keyword"), keywordName);
            row.insert(QStringLiteral("step"), scheduleStep);
            row.insert(QStringLiteral("stepType"), stepType);
            row.insert(QStringLiteral("stepValue"), stepValue);
            row.insert(QStringLiteral("fields"), fields);
            rows.append(row);
        }
    }
    return rows;
}

QVariantMap IDOSSimulationCaseEclipseProvider::buildSummary(const Opm::Deck& deck, const QString& filePath) const
{
    const QStringList sectionKeywords = {
        QStringLiteral("RUNSPEC"), QStringLiteral("GRID"), QStringLiteral("EDIT"),
        QStringLiteral("PROPS"), QStringLiteral("REGIONS"), QStringLiteral("SOLUTION"),
        QStringLiteral("SUMMARY"), QStringLiteral("SCHEDULE")};
    const QStringList countedKeywords = {
        QStringLiteral("WELSPECS"), QStringLiteral("COMPDAT"), QStringLiteral("DATES"),
        QStringLiteral("TSTEP"), QStringLiteral("WCONPROD"), QStringLiteral("WCONINJE")};
    QVariantMap sectionCounts;
    for (const QString& keyword : sectionKeywords)
    {
        const std::vector<const Opm::DeckKeyword*> matches = deck.getKeywordList(keyword.toStdString());
        if (!matches.empty())
        {
            sectionCounts.insert(keyword, static_cast<int>(matches.size()));
        }
    }

    QVariantMap keywordCounts;
    for (const QString& keyword : countedKeywords)
    {
        const std::vector<const Opm::DeckKeyword*> matches = deck.getKeywordList(keyword.toStdString());
        int recordCount = 0;
        for (const Opm::DeckKeyword* match : matches)
        {
            if (match != nullptr)
            {
                recordCount += static_cast<int>(match->size());
            }
        }
        keywordCounts.insert(keyword, recordCount);
    }

    QStringList wellNames;
    const std::vector<const Opm::DeckKeyword*> wellKeywords = deck.getKeywordList("WELSPECS");
    for (const Opm::DeckKeyword* keyword : wellKeywords)
    {
        if (keyword == nullptr)
        {
            continue;
        }
        for (std::size_t recordIndex = 0; recordIndex < keyword->size(); ++recordIndex)
        {
            const Opm::DeckRecord& record = keyword->getRecord(recordIndex);
            if (record.size() == 0)
            {
                continue;
            }
            const std::string wellName = record.getItem(0).getTrimmedString(0);
            if (!wellName.empty())
            {
                wellNames.append(QString::fromStdString(wellName));
            }
        }
    }
    wellNames.removeDuplicates();

    QString caseName;
    const std::vector<const Opm::DeckKeyword*> titleKeywords = deck.getKeywordList("TITLE");
    if (!titleKeywords.empty() && titleKeywords.front() != nullptr && titleKeywords.front()->size() > 0)
    {
        const Opm::DeckRecord& record = titleKeywords.front()->getRecord(0);
        if (record.size() > 0)
        {
            const Opm::DeckItem& titleItem = record.getItem(0);
            QStringList titleParts;
            for (std::size_t index = 0; index < titleItem.data_size(); ++index)
            {
                titleParts.append(QString::fromStdString(titleItem.getTrimmedString(index)));
            }
            caseName = titleParts.join(QLatin1Char(' ')).trimmed();
        }
    }
    if (caseName.isEmpty())
    {
        caseName = QFileInfo(filePath).completeBaseName();
    }

    QVariantMap summary;
    summary.insert(QStringLiteral("caseName"), caseName);
    summary.insert(QStringLiteral("filePath"), QFileInfo(filePath).absoluteFilePath());
    summary.insert(QStringLiteral("sectionCounts"), sectionCounts);
    summary.insert(QStringLiteral("keywordCounts"), keywordCounts);
    summary.insert(QStringLiteral("wellNames"), wellNames);
    return summary;
}

bool IDOSSimulationCaseEclipseProvider::convertEclipseGrid(const Opm::EclipseGrid& eclipseGrid, QVector<double>* coord,
                                                           QVector<double>* zcorn, QVector<int>* actnum,
                                                           QString* error) const
{
    const std::size_t nx = eclipseGrid.getNX();
    const std::size_t ny = eclipseGrid.getNY();
    const std::size_t nz = eclipseGrid.getNZ();
    if (nx == 0 || ny == 0 || nz == 0)
    {
        *error = QObject::tr("Invalid grid dimensions: nx=%1 ny=%2 nz=%3")
                     .arg(static_cast<qulonglong>(nx))
                     .arg(static_cast<qulonglong>(ny))
                     .arg(static_cast<qulonglong>(nz));
        return false;
    }

    const std::vector<double>& opmCoord = eclipseGrid.getCOORD();
    const std::vector<double>& opmZcorn = eclipseGrid.getZCORN();
    const std::vector<int>& opmActnum = eclipseGrid.getACTNUM();
    const std::size_t pillarsPerLayer = (nx + 1) * (ny + 1);
    if (opmCoord.size() != 6ULL * pillarsPerLayer)
    {
        *error = QObject::tr("COORD size mismatch: got %1, expected %2")
                     .arg(static_cast<qulonglong>(opmCoord.size()))
                     .arg(static_cast<qulonglong>(6ULL * pillarsPerLayer));
        return false;
    }

    *coord = doubleVectorToQVector(opmCoord);
    const std::size_t cellsPerLayer = nx * ny;
    if (opmZcorn.size() != 8ULL * cellsPerLayer * nz)
    {
        *error = QObject::tr("ZCORN size mismatch: got %1, expected %2")
                     .arg(static_cast<qulonglong>(opmZcorn.size()))
                     .arg(static_cast<qulonglong>(8ULL * cellsPerLayer * nz));
        return false;
    }
    zcorn->resize(static_cast<int>(opmZcorn.size()));
    const Opm::ZcornMapper mapper = eclipseGrid.zcornMapper();
    const int opmCorners[8] = {0, 1, 3, 2, 4, 5, 7, 6};
    for (std::size_t k = 0; k < nz; ++k)
    {
        for (std::size_t j = 0; j < ny; ++j)
        {
            for (std::size_t i = 0; i < nx; ++i)
            {
                const std::size_t base = (i + j * nx + k * cellsPerLayer) * 8;
                for (int c = 0; c < 8; ++c)
                {
                    (*zcorn)[static_cast<int>(base + c)] = opmZcorn[mapper.index(i, j, k, opmCorners[c])];
                }
            }
        }
    }
    actnum->clear();
    actnum->reserve(static_cast<int>(opmActnum.size()));
    for (int value : opmActnum)
    {
        actnum->append(value);
    }
    return true;
}
QVector<double> IDOSSimulationCaseEclipseProvider::doubleVectorToQVector(const std::vector<double>& values) const
{
    QVector<double> result;
    result.reserve(static_cast<int>(values.size()));
    for (double value : values)
    {
        result.append(value);
    }
    return result;
}

QVector<double> IDOSSimulationCaseEclipseProvider::intVectorToDoubleQVector(const std::vector<int>& values) const
{
    QVector<double> result;
    result.reserve(static_cast<int>(values.size()));
    for (int value : values)
    {
        result.append(static_cast<double>(value));
    }
    return result;
}

void IDOSSimulationCaseEclipseProvider::appendDoubleProperty(QList<IDOSDataObject*>* result,
                                                             const Opm::FieldPropsManager& fieldProps, IDOSGridObject* grid,
                                                             const QString& keyword, IDOSGridPropertyObject::Kind kind) const
{
    const std::string keywordText = keyword.toStdString();
    if (!fieldProps.has_double(keywordText))
    {
        return;
    }

    const std::vector<double> values = fieldProps.get_global_double(keywordText);
    IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
    property->setName(keyword);
    property->setKeyword(keyword);
    property->setKind(kind);
    property->setGridId(grid->objectId());
    property->setDimensions(grid->nx(), grid->ny(), grid->nz());
    if (!property->setValues(doubleVectorToQVector(values)))
    {
        delete property;
        return;
    }
    result->append(property);
}

void IDOSSimulationCaseEclipseProvider::appendIntProperty(QList<IDOSDataObject*>* result,
                                                          const Opm::FieldPropsManager& fieldProps, IDOSGridObject* grid,
                                                          const QString& keyword) const
{
    const std::string keywordText = keyword.toStdString();
    if (!fieldProps.has_int(keywordText))
    {
        return;
    }

    const std::vector<int> values = fieldProps.get_global_int(keywordText);
    IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
    property->setName(keyword);
    property->setKeyword(keyword);
    property->setKind(IDOSGridPropertyObject::Kind::Static);
    property->setGridId(grid->objectId());
    property->setDimensions(grid->nx(), grid->ny(), grid->nz());
    if (!property->setValues(intVectorToDoubleQVector(values)))
    {
        delete property;
        return;
    }
    result->append(property);
}

void IDOSSimulationCaseEclipseProvider::appendActnumProperty(QList<IDOSDataObject*>* result, IDOSGridObject* grid) const
{
    if (grid == nullptr || grid->actnum().isEmpty())
    {
        return;
    }

    IDOSGridPropertyObject* property = new IDOSGridPropertyObject();
    property->setName(QStringLiteral("ACTNUM"));
    property->setKeyword(QStringLiteral("ACTNUM"));
    property->setKind(IDOSGridPropertyObject::Kind::Static);
    property->setGridId(grid->objectId());
    property->setDimensions(grid->nx(), grid->ny(), grid->nz());
    QVector<double> values;
    values.reserve(grid->actnum().size());
    const QVector<int>& actnum = grid->actnum();
    for (int index = 0; index < actnum.size(); ++index)
    {
        values.append(static_cast<double>(actnum.at(index)));
    }
    if (!property->setValues(values))
    {
        delete property;
        return;
    }
    result->append(property);
}

void IDOSSimulationCaseEclipseProvider::appendGridObjects(QList<IDOSDataObject*>* result,
                                                          IDOSSimulationCaseObject* simulationCase,
                                                          const Opm::Deck& deck) const
{
    if (!Opm::EclipseGrid::hasCornerPointKeywords(deck) && !Opm::EclipseGrid::hasCartesianKeywords(deck))
    {
        return;
    }

    Opm::EclipseState state(deck);
    const Opm::EclipseGrid& eclipseGrid = state.getInputGrid();
    QVector<double> coord;
    QVector<double> zcorn;
    QVector<int> actnum;
    QString error;
    if (!convertEclipseGrid(eclipseGrid, &coord, &zcorn, &actnum, &error))
    {
        return;
    }

    IDOSGridObject* grid = new IDOSGridObject();
    grid->setName(QObject::tr("Main Grid"));
    grid->setCornerPointData(static_cast<int>(eclipseGrid.getNX()), static_cast<int>(eclipseGrid.getNY()),
                             static_cast<int>(eclipseGrid.getNZ()), coord, zcorn, actnum);
    simulationCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.grid"), grid->objectId()));
    result->append(grid);
    appendActnumProperty(result, grid);

    const Opm::FieldPropsManager& fieldProps = state.globalFieldProps();
    const QStringList staticDoubleKeywords = {
        QStringLiteral("PORO"),  QStringLiteral("PERMX"),  QStringLiteral("PERMY"),
        QStringLiteral("PERMZ"), QStringLiteral("MULTPV"), QStringLiteral("SIGMAV"),
        QStringLiteral("MULTX"), QStringLiteral("MULTY"),  QStringLiteral("MULTZ")};
    for (const QString& keyword : staticDoubleKeywords)
    {
        appendDoubleProperty(result, fieldProps, grid, keyword, IDOSGridPropertyObject::Kind::Static);
    }

    const QStringList regionKeywords = {QStringLiteral("SATNUM"), QStringLiteral("ROCKNUM"), QStringLiteral("PVTNUM"),
                                        QStringLiteral("EQLNUM"), QStringLiteral("FIPNUM")};
    for (const QString& keyword : regionKeywords)
    {
        appendIntProperty(result, fieldProps, grid, keyword);
    }

    for (IDOSDataObject* object : *result)
    {
        IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
        if (property != nullptr && property->gridId() == grid->objectId())
        {
            simulationCase->addItemRef(IDOSCaseItemRef(QStringLiteral("case.gridProperty"),
                                                       property->objectId()));
        }
    }
}

bool IDOSSimulationCaseEclipseProvider::parseDeckFile(const QString& filePath,
                                                       std::unique_ptr<Opm::Deck>* deck,
                                                       QString* error,
                                                       QString* parserFilePath) const
{
    if (deck == nullptr || error == nullptr)
    {
        return false;
    }

    QString temporaryPath;
    QString compatibilityError;
    if (!createNumresCompatibilityFile(filePath, &temporaryPath, &compatibilityError))
    {
        *error = compatibilityError;
        return false;
    }

    const QString parserPath = temporaryPath.isEmpty() ? filePath : temporaryPath;
    Opm::Parser parser;
    if (!configureParser(&parser, error))
    {
        removeTemporaryParserFile(temporaryPath);
        return false;
    }

    Opm::ParseContext context;
    context.update(Opm::ParseContext::PARSE_MISSING_INCLUDE, Opm::InputErrorAction::THROW_EXCEPTION);
    context.setPreserveUnknownKeywords(true);
    Opm::ErrorGuard errors;
    std::unique_ptr<Opm::Deck> parsedDeck;
    try
    {
        parsedDeck.reset(new Opm::Deck(parser.parseFile(parserPath.toStdString(), context, errors)));
    }
    catch (const std::exception& exception)
    {
        removeTemporaryParserFile(temporaryPath);
        *error = QObject::tr("OPM failed to parse the DATA file: %1")
                     .arg(QString::fromUtf8(exception.what()));
        errors.clear();
        return false;
    }
    catch (...)
    {
        removeTemporaryParserFile(temporaryPath);
        *error = QObject::tr("OPM failed to parse the DATA file due to an unknown error.");
        errors.clear();
        return false;
    }
    removeTemporaryParserFile(temporaryPath);
    if (errors)
    {
        *error = QString::fromStdString(errors.formattedErrors());
        errors.clear();
        return false;
    }

    *deck = std::move(parsedDeck);
    if (parserFilePath != nullptr)
    {
        *parserFilePath = parserPath;
    }
    return true;
}

bool IDOSSimulationCaseEclipseProvider::configureParser(Opm::Parser* parser, QString* error) const
{
    if (parser == nullptr || error == nullptr)
    {
        return false;
    }

    const QString keywordFilePath = QDir(QCoreApplication::applicationDirPath())
                                        .filePath(QStringLiteral("opm/extra-keywords.json"));
    if (!QFileInfo::exists(keywordFilePath))
    {
        return true;
    }

    try
    {
        parser->addParserKeywords(std::filesystem::path(keywordFilePath.toStdWString()));
    }
    catch (const std::exception& exception)
    {
        *error = QObject::tr("The OPM custom keyword file could not be loaded: %1")
                     .arg(QString::fromUtf8(exception.what()));
        return false;
    }

    return true;
}

bool IDOSSimulationCaseEclipseProvider::createNumresCompatibilityFile(const QString& sourcePath,
                                                                       QString* temporaryPath,
                                                                       QString* error) const
{
    if (temporaryPath == nullptr || error == nullptr)
    {
        return false;
    }

    QFile sourceFile(sourcePath);
    if (!sourceFile.open(QIODevice::ReadOnly))
    {
        *error = QObject::tr("The source DATA file cannot be opened for compatibility parsing.");
        return false;
    }

    const QByteArray sourceBytes = sourceFile.readAll();
    const QList<QByteArray> lines = sourceBytes.split('\n');
    int runSpecLine = -1;
    int specGridLine = -1;
    QStringList specGridInlineValues;
    bool hasCoordSys = false;
    bool hasNumRes = false;
    for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
    {
        const QString line = QString::fromLatin1(lines.at(lineIndex).trimmed());
        if (line.isEmpty() || line.startsWith(QStringLiteral("--")))
        {
            continue;
        }
        const QStringList tokens = line.split(QRegularExpression(QStringLiteral("\\s+")),
                                              Qt::SkipEmptyParts);
        if (tokens.isEmpty())
        {
            continue;
        }
        const QString keyword = tokens.first().toUpper();
        if (keyword == QStringLiteral("RUNSPEC"))
        {
            runSpecLine = lineIndex;
        }
        else if (keyword == QStringLiteral("COORDSYS"))
        {
            hasCoordSys = true;
        }
        else if (keyword == QStringLiteral("NUMRES"))
        {
            hasNumRes = true;
        }
        else if (keyword == QStringLiteral("SPECGRID"))
        {
            specGridLine = lineIndex;
            specGridInlineValues = tokens.mid(1);
        }
    }

    if (!hasCoordSys || hasNumRes)
    {
        temporaryPath->clear();
        error->clear();
        return true;
    }

    if (runSpecLine < 0 || specGridLine < 0)
    {
        *error = QObject::tr("NUMRES could not be inferred because RUNSPEC or SPECGRID is missing.");
        return false;
    }

    QStringList specGridValues = specGridInlineValues;
    for (int lineIndex = specGridLine + 1; specGridValues.isEmpty() && lineIndex < lines.size(); ++lineIndex)
    {
        const QString line = QString::fromLatin1(lines.at(lineIndex).trimmed());
        if (line.isEmpty() || line.startsWith(QStringLiteral("--")))
        {
            continue;
        }
        specGridValues = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        break;
    }

    if (specGridValues.size() < 4)
    {
        *error = QObject::tr("NUMRES could not be inferred from the SPECGRID record.");
        return false;
    }

    QString numResValue = specGridValues.at(3);
    numResValue.remove(QLatin1Char('/'));
    bool validNumRes = false;
    const int numRes = numResValue.toInt(&validNumRes);
    if (!validNumRes || numRes < 1)
    {
        *error = QObject::tr("NUMRES could not be inferred from the SPECGRID record.");
        return false;
    }

    const QFileInfo sourceInfo(sourcePath);
    const QString temporaryName = QDir(sourceInfo.absolutePath()).filePath(
        QStringLiteral(".idos_numres_%1.DATA").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));
    QFile temporaryFile(temporaryName);
    if (!temporaryFile.open(QIODevice::WriteOnly | QIODevice::NewOnly))
    {
        *error = QObject::tr("A temporary DATA file could not be created for compatibility parsing.");
        return false;
    }

    const QByteArray lineEnding = sourceBytes.contains("\r\n") ? QByteArray("\r\n") : QByteArray("\n");
    for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
    {
        temporaryFile.write(lines.at(lineIndex));
        if (lineIndex + 1 < lines.size())
        {
            temporaryFile.write("\n");
        }
        if (lineIndex == runSpecLine)
        {
            temporaryFile.write("NUMRES");
            temporaryFile.write(lineEnding);
            temporaryFile.write(QStringLiteral("    %1 /").arg(numRes).toLatin1());
            temporaryFile.write(lineEnding);
        }
    }
    if (temporaryFile.error() != QFileDevice::NoError)
    {
        temporaryFile.close();
        QFile::remove(temporaryName);
        *error = QObject::tr("The temporary DATA file could not be written for compatibility parsing.");
        return false;
    }

    temporaryFile.close();
    *temporaryPath = temporaryName;
    return true;
}

void IDOSSimulationCaseEclipseProvider::removeTemporaryParserFile(const QString& temporaryPath) const
{
    if (!temporaryPath.isEmpty())
    {
        QFile::remove(temporaryPath);
    }
}

QList<IDOSDataObject*> IDOSSimulationCaseEclipseProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;

    try
    {
        std::unique_ptr<Opm::Deck> parsedDeck;
        QString parserFilePath;
        QString parseError;
        if (!parseDeckFile(filePath, &parsedDeck, &parseError, &parserFilePath))
        {
            setLastError(QObject::tr("OPM parse .DATA failed: %1").arg(parseError));
            return result;
        }
        const Opm::Deck& deck = *parsedDeck;
        if (!deck.hasKeyword("RUNSPEC") || !deck.hasKeyword("SCHEDULE"))
        {
            setLastError(QObject::tr("A simulation case requires RUNSPEC and SCHEDULE sections."));
            return result;
        }

        // ① 工况名：TITLE 关键字首条记录的 item 0；缺失回退到文件名
        QString caseName;
        std::vector<const Opm::DeckKeyword*> titles = deck.getKeywordList("TITLE");
        if (!titles.empty())
        {
            const Opm::DeckKeyword* titleKw = titles.front();
            if (titleKw->size() > 0)
            {
                const Opm::DeckRecord& rec = titleKw->getRecord(0);
                if (rec.size() > 0)
                {
                    const Opm::DeckItem& titleItem = rec.getItem(0);
                    QStringList titleParts;
                    for (std::size_t index = 0; index < titleItem.data_size(); ++index)
                    {
                        titleParts.append(QString::fromStdString(titleItem.getTrimmedString(index)));
                    }
                    caseName = titleParts.join(QLatin1Char(' '));
                }
            }
        }
        if (caseName.isEmpty())
        {
            caseName = QFileInfo(filePath).completeBaseName();
        }

        // ② 井名清单：遍历所有 WELSPECS 关键字（SCHEDULE 段可多次出现）
        QStringList wellNames;
        std::vector<const Opm::DeckKeyword*> welspecs = deck.getKeywordList("WELSPECS");
        for (const Opm::DeckKeyword* kw : welspecs)
        {
            if (kw == nullptr)
            {
                continue;
            }
            for (std::size_t r = 0; r < kw->size(); ++r)
            {
                const Opm::DeckRecord& rec = kw->getRecord(r);
                if (rec.size() == 0)
                {
                    continue;
                }
                std::string name = rec.getItem(0).getTrimmedString(0);
                if (!name.empty())
                {
                    wellNames.append(QString::fromStdString(name));
                }
            }
        }
        wellNames.removeDuplicates();

        std::unique_ptr<IDOSSimulationCaseObject> simulationCase(new IDOSSimulationCaseObject());
        simulationCase->setName(caseName.trimmed());
        simulationCase->setSourceFile(QFileInfo(filePath).absoluteFilePath());
        simulationCase->setPendingWellNames(wellNames);
        simulationCase->setParsedSourceData(buildParsedSourceData(deck, filePath, parserFilePath));
        simulationCase->setCurveTables(buildRelativePermeabilityTables(deck));
        simulationCase->setWellControlRows(buildWellControlRows(deck));

        result.append(simulationCase.release());
        appendGridObjects(&result, qobject_cast<IDOSSimulationCaseObject*>(result.first()), deck);
    }
    catch (const std::exception& e)
    {
        for (IDOSDataObject* object : result)
        {
            delete object;
        }
        result.clear();
        setLastError(QObject::tr("OPM parse .DATA failed: %1").arg(QString::fromStdString(e.what())));
    }

    return result;
}


