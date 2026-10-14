#ifndef IDOS_WELL_HEADER_PROVIDER_H
#define IDOS_WELL_HEADER_PROVIDER_H

#include <QList>
#include <QString>
#include <QStringList>

#include "idos_providers.h"
#include "providers/idosdataprovider.h"

/**
 * @brief 井头汇总表解析 provider（Wellheader.txt）。
 *
 * 解析空白分隔的井头汇总表，为每行新建 IDOSWell 并填充
 * wellHead（X/Y/KB/顶底深）。返回的对象无 parent，调用方
 * 负责按井名合并到 Project。
 */
class PROVIDERS_EXPORT IDOSWellHeaderProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    QList<IDOSDataObject*> readWhitespaceHeader(const QString& filePath);
    bool readPetrel(const QStringList& lines, QList<IDOSDataObject*>* result);
    QStringList splitQuotedFields(const QString& line) const;
};

#endif // IDOS_WELL_HEADER_PROVIDER_H
