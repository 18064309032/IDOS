#include "idoswelllasprovider.h"
#include "data/well/idoswell.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

QList<IDOSDataObject*> IDOSWellLasProvider::read(const QString& filePath)
{
    QList<IDOSDataObject*> result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QStringLiteral("Cannot open file: %1").arg(file.errorString()));
        return result;
    }

    // TODO: 真正的 LAS 2.0 / 3.0 解析
    // 先只建一个空井，让注册链路跑通
    QTextStream stream(&file);
    QString line = stream.readLine();
    if (!line.trimmed().startsWith("~V", Qt::CaseInsensitive) &&
        !line.trimmed().startsWith("~VERSION", Qt::CaseInsensitive))
    {
        setLastError(QStringLiteral("Not a valid LAS file"));
        return result;
    }

    IDOSWell* well = new IDOSWell();
    QFileInfo fi(filePath);
    well->setName(fi.completeBaseName());
    // TODO: 从 LAS Well Section 解析坐标、参考深度、曲线通道

    result.append(well);
    return result;
}
