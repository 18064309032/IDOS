#include <array>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/InputErrorAction.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/io/eclipse/EInit.hpp>

#include "case/idossimulationsummaryprovider.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "logger/idoslogger.h"
#include "idosgridpropertyeclipseprovider.h"

#include "idosgrideclipseprovider.h"

QList<IDOSDataObject*> IDOSGridEclipseProvider::read(const QString& filePath)
{
    QElapsedTimer phase;
    phase.start();
    QList<IDOSDataObject*> result;
    setLastError(QString());

    QFileInfo fi(filePath);
    std::string dataFile = filePath.toStdString();

    try
    {
        std::unique_ptr<Opm::EclipseGrid> eclipseGrid;
        std::unique_ptr<Opm::Deck> grdeclDeck;
        if (fi.suffix().compare(QStringLiteral("GRDECL"), Qt::CaseInsensitive) == 0)
        {
            Opm::Parser parser;
            Opm::ParseContext context;
            context.update(Opm::ParseContext::PARSE_MISSING_INCLUDE, Opm::InputErrorAction::THROW_EXCEPTION);
            context.update(Opm::ParseContext::PARSE_UNKNOWN_KEYWORD, Opm::InputErrorAction::IGNORE);
            Opm::ErrorGuard errors;
            grdeclDeck = std::make_unique<Opm::Deck>(parser.parseFile(dataFile, context, errors));
            if (errors)
            {
                setLastError(QObject::tr("OPM could not parse the GRDECL grid file: %1")
                                 .arg(QString::fromStdString(errors.formattedErrors())));
                errors.clear();
                return result;
            }
            reportReadPhase(QStringLiteral("grid-grdecl-parse"), phase.nsecsElapsed());
            phase.restart();
            std::vector<int> actnumValues;
            const std::vector<const Opm::DeckKeyword*> actnumKeywords = grdeclDeck->getKeywordList("ACTNUM");
            for (const Opm::DeckKeyword* keyword : actnumKeywords)
            {
                if (keyword == nullptr)
                {
                    continue;
                }
                for (std::size_t recordIndex = 0; recordIndex < keyword->size(); ++recordIndex)
                {
                    const Opm::DeckRecord& record = keyword->getRecord(recordIndex);
                    for (std::size_t itemIndex = 0; itemIndex < record.size(); ++itemIndex)
                    {
                        const Opm::DeckItem& item = record.getItem(itemIndex);
                        if (item.getType() != Opm::get_type<int>())
                        {
                            setLastError(QObject::tr("The GRDECL ACTNUM keyword must contain integer values."));
                            return result;
                        }
                        for (std::size_t valueIndex = 0; valueIndex < item.data_size(); ++valueIndex)
                        {
                            actnumValues.push_back(item.hasValue(valueIndex) ? item.get<int>(valueIndex) : 1);
                        }
                    }
                }
            }
            const int* actnum = actnumValues.empty() ? nullptr : actnumValues.data();
            eclipseGrid = std::make_unique<Opm::EclipseGrid>(*grdeclDeck, actnum);
        }
        else
        {
            // OPM reads binary EGRID/GRID records, including byte-order detection.
            eclipseGrid = std::make_unique<Opm::EclipseGrid>(dataFile);
        }

        reportReadPhase(QStringLiteral("grid-opm-construction"), phase.nsecsElapsed());
        phase.restart();
        const std::size_t nx = eclipseGrid->getNX();
        const std::size_t ny = eclipseGrid->getNY();
        const std::size_t nz = eclipseGrid->getNZ();
        if (nx == 0 || ny == 0 || nz == 0)
        {
            setLastError(QObject::tr("Invalid grid dimensions: nx=%1 ny=%2 nz=%3")
                             .arg(static_cast<qulonglong>(nx))
                             .arg(static_cast<qulonglong>(ny))
                             .arg(static_cast<qulonglong>(nz)));
            return result;
        }

        const std::vector<double>& opmCoord = eclipseGrid->getCOORD();
        const std::vector<double>& opmZcorn = eclipseGrid->getZCORN();
        const std::vector<int>& opmActnum = eclipseGrid->getACTNUM();

        const std::size_t pillarsPerLayer = (nx + 1) * (ny + 1);
        if (opmCoord.size() != 6ULL * pillarsPerLayer)
        {
            setLastError(QObject::tr("COORD size mismatch: got %1, expected %2")
                             .arg(static_cast<qulonglong>(opmCoord.size()))
                             .arg(static_cast<qulonglong>(6ULL * pillarsPerLayer)));
            return result;
        }

        QVector<double> coord(static_cast<int>(opmCoord.size()));
        if (!opmCoord.empty())
        {
            std::memcpy(coord.data(), opmCoord.data(), opmCoord.size() * sizeof(double));
        }
        const std::size_t cellsPerLayer = nx * ny;
        if (opmZcorn.size() != 8ULL * cellsPerLayer * nz)
        {
            setLastError(QObject::tr("ZCORN size mismatch: got %1, expected %2")
                             .arg(static_cast<qulonglong>(opmZcorn.size()))
                             .arg(static_cast<qulonglong>(8ULL * cellsPerLayer * nz)));
            return result;
        }

        IDOSGridObject* grid = new IDOSGridObject();
        grid->setName(fi.completeBaseName());
        result.append(grid);
        reportReadPhase(QStringLiteral("grid-coordinate-copy"), phase.nsecsElapsed());
        phase.restart();
        if (grdeclDeck != nullptr)
        {
            IDOSGridPropertyEclipseProvider propertyProvider;
            result.append(propertyProvider.readGridProperties(
                *grdeclDeck, grid->objectId(), static_cast<int>(nx), static_cast<int>(ny),
                static_cast<int>(nz), opmActnum));
            grdeclDeck.reset();
        }

        reportReadPhase(QStringLiteral("grid-deck-properties"), phase.nsecsElapsed());
        phase.restart();
        QVector<double> zcorn(static_cast<int>(opmZcorn.size()));
        double* targetCorners = zcorn.data();
        const double* sourceCorners = opmZcorn.data();
        const Opm::ZcornMapper mapper = eclipseGrid->zcornMapper();
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
                        targetCorners[base + c] = sourceCorners[mapper.index(i, j, k, opmCorners[c])];
                    }
                }
            }
        }
        QVector<int> actnum(static_cast<int>(opmActnum.size()));
        if (!opmActnum.empty())
        {
            std::memcpy(actnum.data(), opmActnum.data(), opmActnum.size() * sizeof(int));
        }
        grid->setCornerPointData(static_cast<int>(nx), static_cast<int>(ny), static_cast<int>(nz),
                                 std::move(coord), std::move(zcorn), std::move(actnum));
        reportReadPhase(QStringLiteral("grid-array-conversion"), phase.nsecsElapsed());
        phase.restart();
        eclipseGrid.reset();
        reportReadPhase(QStringLiteral("grid-opm-release"), phase.nsecsElapsed());
        phase.restart();
        if (!readCompanionFiles(filePath, &result))
        {
            qDeleteAll(result);
            result.clear();
        }
        reportReadPhase(QStringLiteral("grid-companions-total"), phase.nsecsElapsed());
    }
    catch (const std::exception& e)
    {
        qDeleteAll(result);
        result.clear();
        setLastError(QObject::tr("OPM EclipseGrid read failed: %1").arg(QString::fromStdString(e.what())));
    }
    return result;
}

bool IDOSGridEclipseProvider::readCompanionFiles(const QString& filePath,
                                               QList<IDOSDataObject*>* objects)
{
    IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(objects->first());
    const QFileInfo gridFile(filePath);
    const QFileInfoList files = gridFile.dir().entryInfoList(QDir::Files, QDir::Name);
    QMap<QString, QString> companions;
    for (const QFileInfo& file : files)
    {
        if (file.completeBaseName().compare(gridFile.completeBaseName(), Qt::CaseInsensitive) != 0)
        {
            continue;
        }
        const QString suffix = file.suffix().toUpper();
        if (suffix == QStringLiteral("INIT") || suffix == QStringLiteral("UNRST")
            || suffix == QStringLiteral("SMSPEC") || suffix == QStringLiteral("UNSMRY"))
        {
            companions.insert(suffix, file.absoluteFilePath());
        }
    }

    const QStringList suffixes = {QStringLiteral("INIT"), QStringLiteral("UNRST"),
                                  QStringLiteral("SMSPEC")};
    for (const QString& suffix : suffixes)
    {
        if (!companions.contains(suffix))
        {
            continue;
        }
        QElapsedTimer companionTimer;
        companionTimer.start();
        const QString companionPath = companions.value(suffix);
        QList<IDOSDataObject*> imported;
        QString error;
        if (suffix == QStringLiteral("SMSPEC"))
        {
            IDOSSimulationSummaryProvider provider;
            imported = provider.read(companionPath);
            error = provider.lastError();
        }
        else
        {
            IDOSGridPropertyEclipseProvider provider;
            if (suffix == QStringLiteral("UNRST"))
            {
                imported = provider.readRestart(companionPath, grid);
            }
            else
            {
                Opm::EclIO::EInit initFile(companionPath.toStdString());
                const std::array<int, 3>& dimensions = initFile.grid_dimension();
                if (dimensions[0] != grid->nx() || dimensions[1] != grid->ny()
                    || dimensions[2] != grid->nz() || initFile.activeCells() != grid->activeCellCount())
                {
                    IDOS_WARN(QObject::tr("Skipping incompatible companion file %1: grid is %2 x %3 x %4 with %5 active cells, while INIT is %6 x %7 x %8 with %9 active cells.")
                                   .arg(companionPath)
                                   .arg(grid->nx())
                                   .arg(grid->ny())
                                   .arg(grid->nz())
                                   .arg(grid->activeCellCount())
                                   .arg(dimensions[0])
                                   .arg(dimensions[1])
                                   .arg(dimensions[2])
                                   .arg(initFile.activeCells()));
                    continue;
                }
                imported = provider.read(companionPath);
            }
            error = provider.lastError();
        }
        reportReadPhase(QStringLiteral("companion-read-") + suffix, companionTimer.nsecsElapsed());
        if (!error.isEmpty())
        {
            qDeleteAll(imported);
            IDOS_WARN(QObject::tr("Skipping companion file %1 because it could not be imported: %2")
                           .arg(companionPath, error));
            continue;
        }
        reportReadPhase(QStringLiteral("companion-") + suffix, companionTimer.nsecsElapsed());
        if (suffix == QStringLiteral("INIT"))
        {
            bool propertiesMatchGrid = true;
            for (IDOSDataObject* object : imported)
            {
                IDOSGridPropertyObject* property = qobject_cast<IDOSGridPropertyObject*>(object);
                if (property == nullptr)
                {
                    continue;
                }
                QVector<double> values = property->values();
                if (values.size() == grid->activeCellCount()
                    && values.size() != grid->totalCellCount())
                {
                    QVector<double> expanded(grid->totalCellCount(), 0.0);
                    int activeIndex = 0;
                    const QVector<int>& actnum = grid->actnum();
                    for (int index = 0; index < expanded.size(); ++index)
                    {
                        if (actnum.isEmpty() || actnum.at(index) != 0)
                        {
                            expanded[index] = values.at(activeIndex);
                            ++activeIndex;
                        }
                    }
                    values = expanded;
                }
                if (values.size() != grid->totalCellCount())
                {
                    propertiesMatchGrid = false;
                    break;
                }
                property->setGridId(grid->objectId());
                property->setDimensions(grid->nx(), grid->ny(), grid->nz());
                if (!property->setValues(values))
                {
                    propertiesMatchGrid = false;
                    break;
                }
            }
            if (!propertiesMatchGrid)
            {
                qDeleteAll(imported);
                IDOS_WARN(QObject::tr("Skipping INIT properties from %1 because their array sizes do not match the grid dimensions.")
                               .arg(companionPath));
                continue;
            }
        }
        objects->append(imported);
    }
    if (companions.contains(QStringLiteral("UNSMRY"))
        && !companions.contains(QStringLiteral("SMSPEC")))
    {
        IDOS_WARN(QObject::tr("Skipping companion UNSMRY file because no matching SMSPEC file was found."));
    }
    return true;
}
