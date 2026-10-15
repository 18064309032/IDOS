#include <array>
#include <exception>
#include <string>
#include <vector>

#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/io/eclipse/EInit.hpp>

#include "case/idossimulationsummaryprovider.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridproperty.h"
#include "grid/idosgridpropertyeclipseprovider.h"

#include "idosgrideclipseprovider.h"

QList<IDOSDataObject*> IDOSGridEclipseProvider::read(const QString& filePath)
{
    QList<IDOSDataObject*> result;
    setLastError(QString());

    QFileInfo fi(filePath);
    std::string dataFile = filePath.toStdString();

    try
    {
        // OPM 直接读 .EGRID（内部完成 FILEHEAD/COORD/ZCORN/ACTNUM 解析
        // 与字节序检测；二进制 record marker 包装由 OPM 处理）
        Opm::EclipseGrid eclipseGrid(dataFile);

        const std::size_t nx = eclipseGrid.getNX();
        const std::size_t ny = eclipseGrid.getNY();
        const std::size_t nz = eclipseGrid.getNZ();
        if (nx == 0 || ny == 0 || nz == 0)
        {
            setLastError(QObject::tr("Invalid grid dimensions: nx=%1 ny=%2 nz=%3")
                             .arg(static_cast<qulonglong>(nx))
                             .arg(static_cast<qulonglong>(ny))
                             .arg(static_cast<qulonglong>(nz)));
            return result;
        }

        const std::vector<double>& opmCoord = eclipseGrid.getCOORD();
        const std::vector<double>& opmZcorn = eclipseGrid.getZCORN();
        const std::vector<int>& opmActnum = eclipseGrid.getACTNUM();

        const std::size_t pillarsPerLayer = (nx + 1) * (ny + 1);
        if (opmCoord.size() != 6ULL * pillarsPerLayer)
        {
            setLastError(QObject::tr("COORD size mismatch: got %1, expected %2")
                             .arg(static_cast<qulonglong>(opmCoord.size()))
                             .arg(static_cast<qulonglong>(6ULL * pillarsPerLayer)));
            return result;
        }

        QVector<double> coord;
        coord.reserve(static_cast<int>(opmCoord.size()));
        for (double value : opmCoord)
        {
            coord.append(value);
        }
        const std::size_t cellsPerLayer = nx * ny;
        if (opmZcorn.size() != 8ULL * cellsPerLayer * nz)
        {
            setLastError(QObject::tr("ZCORN size mismatch: got %1, expected %2")
                             .arg(static_cast<qulonglong>(opmZcorn.size()))
                             .arg(static_cast<qulonglong>(8ULL * cellsPerLayer * nz)));
            return result;
        }
        QVector<double> zcorn(static_cast<int>(opmZcorn.size()));
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
                        zcorn[static_cast<int>(base + c)] = opmZcorn[mapper.index(i, j, k, opmCorners[c])];
                    }
                }
            }
        }
        QVector<int> actnum;
        actnum.reserve(static_cast<int>(opmActnum.size()));
        for (int value : opmActnum)
        {
            actnum.append(value);
        }
        IDOSGrid* grid = new IDOSGrid();
        grid->setName(fi.completeBaseName());
        grid->setCornerPointData(static_cast<int>(nx), static_cast<int>(ny), static_cast<int>(nz), coord, zcorn, actnum);
        result.append(grid);
        if (!readCompanionFiles(filePath, &result))
        {
            qDeleteAll(result);
            result.clear();
        }
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
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(objects->first());
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
                    setLastError(QObject::tr("The companion INIT properties do not match the grid dimensions."));
                    return false;
                }
                imported = provider.read(companionPath);
            }
            error = provider.lastError();
        }
        if (!error.isEmpty())
        {
            qDeleteAll(imported);
            setLastError(QObject::tr("Unable to import companion file %1: %2")
                             .arg(companionPath, error));
            return false;
        }
        if (suffix == QStringLiteral("INIT"))
        {
            for (IDOSDataObject* object : imported)
            {
                IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
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
                    qDeleteAll(imported);
                    setLastError(QObject::tr("The companion INIT properties do not match the grid dimensions."));
                    return false;
                }
                property->setGridId(grid->objectId());
                property->setDimensions(grid->nx(), grid->ny(), grid->nz());
                if (!property->setValues(values))
                {
                    qDeleteAll(imported);
                    setLastError(QObject::tr("The companion INIT properties do not match the grid dimensions."));
                    return false;
                }
            }
        }
        objects->append(imported);
    }
    if (companions.contains(QStringLiteral("UNSMRY"))
        && !companions.contains(QStringLiteral("SMSPEC")))
    {
        setLastError(QObject::tr("The companion UNSMRY file requires a matching SMSPEC file."));
        return false;
    }
    return true;
}
