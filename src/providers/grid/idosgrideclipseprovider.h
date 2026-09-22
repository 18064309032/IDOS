#ifndef IDOS_GRID_ECLIPSE_PROVIDER_H
#define IDOS_GRID_ECLIPSE_PROVIDER_H

#include "idosdataprovider.h"

/**
 * @brief ECLIPSE 角点网格解析 provider（.GRID / .EGRID）。
 *
 * 用 OPM (opm-common) 的 EclipseGrid 直接读取 .EGRID 二进制文件，
 * OPM 内部完成 FILEHEAD/COORD/ZCORN/ACTNUM 的 Fortran record marker
 * 解析与字节序处理。本 provider 只做数组布局转换：
 *   COORD: OPM 6 doubles/pillar → IDOSGrid 4 corner XYs/cell
 *   ZCORN: OPM 8-planes-per-k → IDOSGrid 8-values-per-cell
 *   ACTNUM: 同为 natural order，直接复制
 *
 * 文件名作为网格名（completeBaseName）。.GRID ASCII 格式暂不支持
 * （OPM EclipseGrid 构造走 .EGRID 路径；如需 .GRID 后续扩展）。
 */
class PROVIDERS_EXPORT IDOSGridEclipseProvider : public IDOSDataProvider
{
public:
    QList<IDOSDataObject*> read(const QString& filePath) override;
};

#endif // IDOS_GRID_ECLIPSE_PROVIDER_H
