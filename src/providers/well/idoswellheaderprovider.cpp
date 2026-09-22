#include "idoswellheaderprovider.h"
#include "data/well/idoswell.h"
#include "data/well/idoswellhead.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

QList<IDOSDataObject*> IDOSWellHeaderProvider::read(const QString& filePath)
{
    QList<IDOSDataObject*> result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QStringLiteral("Cannot open file: %1").arg(file.errorString()));
        return result;
    }

    QTextStream stream(&file);

    // 跳过表头行
    stream.readLine();

    int importedCount = 0;

    while (!stream.atEnd())
    {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        // 空白分隔（一个或多个空格/制表符）
        QStringList fields = line.split(QRegularExpression(QStringLiteral("\\s+")),
                                        Qt::SkipEmptyParts);
        // 列：WellName X Y Top_Depth Bottom_Depth KB Symbol
        if (fields.size() < 6)
        {
            continue;   // 跳过不完整行
        }

        bool okX = false, okY = false, okTop = false, okBottom = false, okKB = false;
        QString wellName = fields[0];
        double x = fields[1].toDouble(&okX);
        double y = fields[2].toDouble(&okY);
        double topDepth = fields[3].toDouble(&okTop);
        double bottomDepth = fields[4].toDouble(&okBottom);
        double kb = fields[5].toDouble(&okKB);

        if (!okX || !okY || !okTop || !okBottom || !okKB)
        {
            continue;   // 跳过数值解析失败行
        }

        // 建 Well、填井头；所有权交调用方
        IDOSWell* well = new IDOSWell();
        well->setName(wellName);

        IDOSWellHead head;
        head.setSurfaceX(x);
        head.setSurfaceY(y);
        head.setSurfaceElevation(kb);
        head.setTopDepth(topDepth);
        head.setBottomDepth(bottomDepth);
        well->setWellHead(head);

        result.append(well);
        ++importedCount;
    }

    if (importedCount == 0)
    {
        setLastError(QStringLiteral("No valid well header rows parsed from %1")
                         .arg(QFileInfo(filePath).fileName()));
    }

    return result;
}
