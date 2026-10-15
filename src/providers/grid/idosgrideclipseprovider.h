#ifndef IDOS_GRID_ECLIPSE_PROVIDER_H
#define IDOS_GRID_ECLIPSE_PROVIDER_H

#include <QList>
#include <QString>

#include "idos_providers.h"
#include "providers/idosdataprovider.h"

/**
 * @brief ECLIPSE 角点网格解析 provider（.GRID / .EGRID）。
 *
 * 用 OPM (opm-common) 的 EclipseGrid 直接读取 .EGRID 二进制文件，
 * OPM 内部完成 FILEHEAD/COORD/ZCORN/ACTNUM 的 Fortran record marker
 * 解析与字节序处理。本 provider 完成数组布局转换，并读取同目录同主文件名
 * 的 INIT、UNRST 和 SMSPEC/UNSMRY 配套数据：
 *   COORD: 原始柱线坐标直接保存
 *   ZCORN: OPM ZcornMapper 转换为每单元八角点（VTK 顺序）
 *   ACTNUM: 同为 natural order，直接复制
 *
 * 文件名作为网格名（completeBaseName）。.GRID ASCII 格式暂不支持
 * （OPM EclipseGrid 构造走 .EGRID 路径；如需 .GRID 后续扩展）。
 */
class PROVIDERS_EXPORT IDOSGridEclipseProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    bool readCompanionFiles(const QString& filePath, QList<IDOSDataObject*>* objects);
};

#endif // IDOS_GRID_ECLIPSE_PROVIDER_H

