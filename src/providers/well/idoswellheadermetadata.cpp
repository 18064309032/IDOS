#include "idoswellheadermetadata.h"
#include "idoswellheaderprovider.h"

#include <QFileInfo>

bool IDOSWellHeaderMetadata::canHandle(const QString& filePath) const
{
    // 文件名含 "wellheader" 即认（不依赖扩展名）
    QFileInfo fi(filePath);
    return fi.fileName().toLower().contains(QStringLiteral("wellheader"));
}

std::unique_ptr<IDOSDataProvider> IDOSWellHeaderMetadata::createProvider() const
{
    return std::make_unique<IDOSWellHeaderProvider>();
}
