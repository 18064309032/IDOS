#include <algorithm>
#include <exception>
#include <utility>
#include <vector>

#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idossimulationcaseobject.h"

#include "idossimulationcaseeclipseprovider.h"

QVariantList IDOSSimulationCaseEclipseProvider::buildDeckKeywordIndex(const Opm::Deck& deck) const
{
    QVariantList result;
    QVector<QVariantMap> keywordLocations;
    keywordLocations.reserve(static_cast<int>(deck.size()));
    QMap<QString, qulonglong> nextLineByFile;
    for (std::size_t index = deck.size(); index > 0; --index)
    {
        const Opm::DeckKeyword& deckKeyword = deck[index - 1];
        const Opm::KeywordLocation& location = deckKeyword.location();
        const QString filePath = QString::fromStdString(deck.makeDeckPath(location.filename));
        const QString fileName = QDir::cleanPath(QFileInfo(filePath).absoluteFilePath());
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
                                                                     const QString& filePath) const
{
    QVariantMap data;
    data.insert(QStringLiteral("summary"), buildSummary(deck, filePath));
    data.insert(QStringLiteral("keywordIndex"), buildDeckKeywordIndex(deck));
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
                                                             const Opm::FieldPropsManager& fieldProps, IDOSGrid* grid,
                                                             const QString& keyword, IDOSGridProperty::Kind kind) const
{
    const std::string keywordText = keyword.toStdString();
    if (!fieldProps.has_double(keywordText))
    {
        return;
    }

    const std::vector<double> values = fieldProps.get_global_double(keywordText);
    IDOSGridProperty* property = new IDOSGridProperty();
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
                                                          const Opm::FieldPropsManager& fieldProps, IDOSGrid* grid,
                                                          const QString& keyword) const
{
    const std::string keywordText = keyword.toStdString();
    if (!fieldProps.has_int(keywordText))
    {
        return;
    }

    const std::vector<int> values = fieldProps.get_global_int(keywordText);
    IDOSGridProperty* property = new IDOSGridProperty();
    property->setName(keyword);
    property->setKeyword(keyword);
    property->setKind(IDOSGridProperty::Kind::Static);
    property->setGridId(grid->objectId());
    property->setDimensions(grid->nx(), grid->ny(), grid->nz());
    if (!property->setValues(intVectorToDoubleQVector(values)))
    {
        delete property;
        return;
    }
    result->append(property);
}

void IDOSSimulationCaseEclipseProvider::appendActnumProperty(QList<IDOSDataObject*>* result, IDOSGrid* grid) const
{
    if (grid == nullptr || grid->actnum().isEmpty())
    {
        return;
    }

    IDOSGridProperty* property = new IDOSGridProperty();
    property->setName(QStringLiteral("ACTNUM"));
    property->setKeyword(QStringLiteral("ACTNUM"));
    property->setKind(IDOSGridProperty::Kind::Static);
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

    IDOSGrid* grid = new IDOSGrid();
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
        appendDoubleProperty(result, fieldProps, grid, keyword, IDOSGridProperty::Kind::Static);
    }

    const QStringList regionKeywords = {QStringLiteral("SATNUM"), QStringLiteral("ROCKNUM"), QStringLiteral("PVTNUM"),
                                        QStringLiteral("EQLNUM"), QStringLiteral("FIPNUM")};
    for (const QString& keyword : regionKeywords)
    {
        appendIntProperty(result, fieldProps, grid, keyword);
    }
}

QList<IDOSDataObject*> IDOSSimulationCaseEclipseProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;

    std::string dataFile = filePath.toStdString();
    Opm::ErrorGuard errors;

    try
    {
        Opm::Parser parser;
        Opm::ParseContext context;
        Opm::Deck deck = parser.parseFile(dataFile, context, errors);
        if (errors)
        {
            const QString message = QString::fromStdString(errors.formattedErrors());
            errors.clear();
            setLastError(QObject::tr("OPM parse .DATA failed: %1").arg(message));
            return result;
        }
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

        IDOSSimulationCaseObject* simulationCase = new IDOSSimulationCaseObject();
        simulationCase->setName(caseName.trimmed());
        simulationCase->setSourceFile(QFileInfo(filePath).absoluteFilePath());
        simulationCase->setPendingWellNames(wellNames);
        simulationCase->setParsedSourceData(buildParsedSourceData(deck, filePath));
        simulationCase->setCurveTables(buildRelativePermeabilityTables(deck));
        simulationCase->setWellControlRows(buildWellControlRows(deck));

        result.append(simulationCase);
        appendGridObjects(&result, simulationCase, deck);
    }
    catch (const std::exception& e)
    {
        for (IDOSDataObject* object : result)
        {
            delete object;
        }
        result.clear();
        errors.clear();
        setLastError(QObject::tr("OPM parse .DATA failed: %1").arg(QString::fromStdString(e.what())));
    }

    return result;
}


