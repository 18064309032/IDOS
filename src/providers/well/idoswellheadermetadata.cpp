#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QTextStream>

#include "idoswellheaderprovider.h"

#include "idoswellheadermetadata.h"

bool IDOSWellHeaderMetadata::canHandle(const QString& filePath) const
{
    QFileInfo fi(filePath);
    if (fi.fileName().toLower().contains(QStringLiteral("wellheader")))
    {
        return true;
    }
    if (fi.fileName().compare(QStringLiteral("well header.txt"), Qt::CaseInsensitive) == 0)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return false;
        }
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        const QStringList columns = stream.readLine().trimmed().split(
            QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        return columns.size() == 6 && columns.at(0).compare(QStringLiteral("well"), Qt::CaseInsensitive) == 0 &&
               columns.at(1).compare(QStringLiteral("name"), Qt::CaseInsensitive) == 0 &&
               columns.at(2).compare(QStringLiteral("x"), Qt::CaseInsensitive) == 0 &&
               columns.at(3).compare(QStringLiteral("y"), Qt::CaseInsensitive) == 0 &&
               columns.at(4).compare(QStringLiteral("md"), Qt::CaseInsensitive) == 0 &&
               columns.at(5).compare(QStringLiteral("kb"), Qt::CaseInsensitive) == 0;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    bool petrelVersion = false;
    bool inPetrelHeader = false;
    QStringList petrelColumns;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
        {
            continue;
        }
        if (line.compare(QStringLiteral("VERSION 1"), Qt::CaseInsensitive) == 0)
        {
            petrelVersion = true;
            continue;
        }
        if (!petrelVersion)
        {
            return line.contains(QStringLiteral("WellName"), Qt::CaseInsensitive) &&
                line.contains(QStringLiteral("X-Coord"), Qt::CaseInsensitive) &&
                line.contains(QStringLiteral("Y-Coord"), Qt::CaseInsensitive);
        }
        if (line.compare(QStringLiteral("BEGIN HEADER"), Qt::CaseInsensitive) == 0)
        {
            inPetrelHeader = true;
            continue;
        }
        if (line.compare(QStringLiteral("END HEADER"), Qt::CaseInsensitive) == 0)
        {
            return petrelColumns.contains(QStringLiteral("NAME")) &&
                petrelColumns.contains(QStringLiteral("SURFACE X")) &&
                petrelColumns.contains(QStringLiteral("SURFACE Y"));
        }
        if (inPetrelHeader)
        {
            QString column = line;
            const int commaIndex = column.indexOf(QLatin1Char(','));
            if (commaIndex >= 0)
            {
                column = column.mid(commaIndex + 1).trimmed();
            }
            petrelColumns.append(column.toUpper());
        }
    }
    return false;
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
