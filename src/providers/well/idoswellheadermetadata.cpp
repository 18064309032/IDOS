#include <QFileInfo>

#include "idoswellheaderprovider.h"

#include "idoswellheadermetadata.h"

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

QString IDOSWellHeaderMetadata::id() const
{
    return QStringLiteral("idos.well.header");
}

QString IDOSWellHeaderMetadata::displayName() const
{
    return QObject::tr("Well Header");
}

QStringList IDOSWellHeaderMetadata::fileExtensions() const
{
    return QStringList{QStringLiteral("Wellheader.txt")};
}
