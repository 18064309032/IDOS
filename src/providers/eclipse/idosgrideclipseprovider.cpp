#include <exception>

#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QVector>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include "idosgrid.h"

#include "idosgrideclipseprovider.h"

QList<IDOSDataObject*> IDOSGridEclipseProvider::read(const QString& filePath)
{
    QList<IDOSDataObject*> result;

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
    }
    catch (const std::exception& e)
    {
        setLastError(QObject::tr("OPM EclipseGrid read failed: %1").arg(QString::fromStdString(e.what())));
    }
    return result;
}
