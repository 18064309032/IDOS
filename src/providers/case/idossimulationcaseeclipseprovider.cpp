#include <QObject>
#include "idossimulationcaseeclipseprovider.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idossimulationcaseobject.h"

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>

#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QVector>
#include <exception>
#include <utility>
#include <vector>


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


