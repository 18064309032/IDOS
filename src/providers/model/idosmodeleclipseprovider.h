#ifndef IDOS_MODEL_ECLIPSE_PROVIDER_H
#define IDOS_MODEL_ECLIPSE_PROVIDER_H

#include "idosdataprovider.h"

/**
 * @brief ECLIPSE 主输入文件（.DATA）工况解析 provider。
 *
 * 用 OPM (opm-common) 的 Parser 把 .DATA 解析成 Deck，提取：
 * - TITLE 关键字 → 工况名（缺失则用文件 base name）
 * - SCHEDULE 段所有 WELSPECS 关键字 → 井名清单（pending）
 *
 * 不直接解析 gridId——.DATA 的 GRID 段通常 INCLUDE 一个 .EGRID 文件，
 * 跨文件链路复杂，留后续。井名暂存进 IDOSModel::pendingWellNames，
 * 由 IDOSImportCoordinator::import 完成后调 IDOSModel::resolveReferences
 * 走 Project::objectByName 把井名转 objectId 填进 wellIds。
 */
class PROVIDERS_EXPORT IDOSModelEclipseProvider : public IDOSDataProvider
{
public:
    QList<IDOSDataObject*> read(const QString& filePath) override;
};

#endif // IDOS_MODEL_ECLIPSE_PROVIDER_H
