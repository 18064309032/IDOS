#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QTextStream>

#include "idoswellheaderprovider.h"

#include "idoswellheadermetadata.h"

bool IDOSWellHeaderMetadata::canHandle(const QString& filePath) const
{
    // 文件名含 "wellheader" 即认（不依赖扩展名）
    QFileInfo fi(filePath);
    if (fi.fileName().toLower().contains(QStringLiteral("wellheader")))
    {
        return true;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        if (!line.isEmpty())
        {
            return line.contains(QStringLiteral("WellName"), Qt::CaseInsensitive) &&
                line.contains(QStringLiteral("X-Coord"), Qt::CaseInsensitive) &&
                line.contains(QStringLiteral("Y-Coord"), Qt::CaseInsensitive);
        }
    }
    return false;
}

std::unique_ptr<IDOSDataProvider> IDOSWellHeaderMetadata::create() const
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
