#include "idosgrideclipseprovider.h"
#include "data/grid/idosgrid.h"

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>

#include <QFileInfo>
#include <QString>
#include <QVector>
#include <exception>
#include <utility>
#include <vector>

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
            setLastError(QStringLiteral("Invalid grid dimensions: nx=%1 ny=%2 nz=%3")
                             .arg(static_cast<qulonglong>(nx))
                             .arg(static_cast<qulonglong>(ny))
                             .arg(static_cast<qulonglong>(nz)));
            return result;
        }

        const std::vector<double>& opmCoord = eclipseGrid.getCOORD();
        const std::vector<double>& opmZcorn = eclipseGrid.getZCORN();
        const std::vector<int>& opmActnum = eclipseGrid.getACTNUM();

        // ===== COORD 布局转换 =====
        // OPM: 6*(nx+1)*(ny+1) double —— 每 pillar 顶 XYZ + 底 XYZ，垂直 pillar 顶 XY=底 XY
        // IDOSGrid: 4*(nx+1)*(ny+1)*2 double —— 每 cell 4 组 (SW/SE/NE/NW) 的 XY
        // 组 g 在 cell (i,j) 位置存对应 pillar 的 XY：
        //   g=0→pillar(i,j)  g=1→pillar(i+1,j)  g=2→pillar(i+1,j+1)  g=3→pillar(i,j+1)
        const std::size_t pillarsPerLayer = (nx + 1) * (ny + 1);
        if (opmCoord.size() != 6ULL * pillarsPerLayer)
        {
            setLastError(QStringLiteral("COORD size mismatch: got %1, expected %2")
                             .arg(static_cast<qulonglong>(opmCoord.size()))
                             .arg(static_cast<qulonglong>(6ULL * pillarsPerLayer)));
            return result;
        }

        QVector<double> coord;
        coord.resize(static_cast<int>(4LL * pillarsPerLayer * 2));   // 4 groups × pillars × 2 (x,y)
        coord.fill(0.0);

        // 辅助：取 pillar (pi,pj) 的 XY（顶/底 XY 一致）
        auto pillarXY = [&opmCoord, ny](std::size_t pi, std::size_t pj) -> std::pair<double, double>
        {
            const std::size_t off = 6ULL * (pi * (ny + 1) + pj);
            return { opmCoord[off + 0], opmCoord[off + 1] };
        };

        // 只填 cell 会读到的位置：i ∈ [0,nx), j ∈ [0,ny)
        // 组 g 末行末列（i=nx 或 j=ny）cornerPosition 不会访问，留 0
        const int strideIJ = static_cast<int>((ny + 1) * 2);
        const int strideGroup = static_cast<int>(nx + 1) * strideIJ;
        for (std::size_t i = 0; i < nx; ++i)
        {
            for (std::size_t j = 0; j < ny; ++j)
            {
                const int ii = static_cast<int>(i);
                const int jj = static_cast<int>(j);

                // group 0: SW = pillar (i, j)
                std::pair<double, double> xy0 = pillarXY(i, j);
                int base = 0 * strideGroup + ii * strideIJ + jj * 2;
                coord[base + 0] = xy0.first;
                coord[base + 1] = xy0.second;

                // group 1: SE = pillar (i+1, j)
                std::pair<double, double> xy1 = pillarXY(i + 1, j);
                base = 1 * strideGroup + ii * strideIJ + jj * 2;
                coord[base + 0] = xy1.first;
                coord[base + 1] = xy1.second;

                // group 2: NE = pillar (i+1, j+1)
                std::pair<double, double> xy2 = pillarXY(i + 1, j + 1);
                base = 2 * strideGroup + ii * strideIJ + jj * 2;
                coord[base + 0] = xy2.first;
                coord[base + 1] = xy2.second;

                // group 3: NW = pillar (i, j+1)
                std::pair<double, double> xy3 = pillarXY(i, j + 1);
                base = 3 * strideGroup + ii * strideIJ + jj * 2;
                coord[base + 0] = xy3.first;
                coord[base + 1] = xy3.second;
            }
        }

        // ===== ZCORN 布局转换 =====
        // OPM Eclipse 8-planes-per-k：每 k 层 8 个平面，每平面 nx*ny 值 i-fast
        //   平面 c (c=0..7): 第 c 个角的 Z（顺序 botSW/SE/NE/NW, topSW/SE/NE/NW）
        //   opmIdx(i,j,k,c) = k*8*nx*ny + c*nx*ny + (i + j*nx)
        // IDOSGrid 8-values-per-cell，cell 序 j-fast：
        //   idosIdx(i,j,k,c) = ((i*ny + j + k*nx*ny) * 8) + c
        const std::size_t cellsPerLayer = nx * ny;
        if (opmZcorn.size() != 8ULL * cellsPerLayer * nz)
        {
            setLastError(QStringLiteral("ZCORN size mismatch: got %1, expected %2")
                             .arg(static_cast<qulonglong>(opmZcorn.size()))
                             .arg(static_cast<qulonglong>(8ULL * cellsPerLayer * nz)));
            return result;
        }

        QVector<double> zcorn;
        zcorn.resize(static_cast<int>(8LL * cellsPerLayer * nz));
        for (std::size_t k = 0; k < nz; ++k)
        {
            for (std::size_t j = 0; j < ny; ++j)
            {
                for (std::size_t i = 0; i < nx; ++i)
                {
                    const std::size_t idosCellBase =
                        ((i * ny + j + k * cellsPerLayer) * 8);
                    const std::size_t opmCellBaseNatural =
                        (k * 8 * cellsPerLayer) + (i + j * nx);
                    for (std::size_t c = 0; c < 8; ++c)
                    {
                        const std::size_t opmIdx =
                            opmCellBaseNatural + c * cellsPerLayer;
                        zcorn[static_cast<int>(idosCellBase + c)] = opmZcorn[opmIdx];
                    }
                }
            }
        }

        // ===== ACTNUM 直接复制（同为 Eclipse natural order i-fast）=====
        QVector<int> actnum;
        actnum.reserve(static_cast<int>(opmActnum.size()));
        for (int v : opmActnum) actnum.append(v);

        IDOSGrid* grid = new IDOSGrid();
        grid->setName(fi.completeBaseName());
        grid->setCornerPointData(static_cast<int>(nx), static_cast<int>(ny),
                                 static_cast<int>(nz), coord, zcorn, actnum);
        result.append(grid);
    }
    catch (const std::exception& e)
    {
        setLastError(QStringLiteral("OPM EclipseGrid read failed: %1")
                         .arg(QString::fromStdString(e.what())));
    }

    return result;
}
