#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QTextStream>
#include <QVariantMap>

#include "data/fracture/idosfracturematerialcatalogobject.h"

#include "idosfracturematerialcatalogprovider.h"

bool IDOSFractureMaterialCatalogProvider::canRead(const QString& filePath) const
{
    if (QFileInfo(filePath).suffix().compare(QStringLiteral("txt"), Qt::CaseInsensitive) != 0)
    {
        return false;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    const QString header = stream.readLine().trimmed();
    return header.startsWith(QStringLiteral("\u6db2\u4f53\u540d\u79f0\t")) ||
           header.startsWith(QStringLiteral("\u652f\u6491\u5242\u540d\u79f0\t"));
}

QList<IDOSDataObject*> IDOSFractureMaterialCatalogProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        setLastError(QObject::tr("Cannot open fracture material catalog: %1").arg(file.errorString()));
        return result;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    const QStringList columns = stream.readLine().trimmed().split(QLatin1Char('\t'));
    const QStringList fluidColumns{QStringLiteral("\u6db2\u4f53\u540d\u79f0"),
                                   QStringLiteral("\u6db2\u4f53\u7c7b\u578b"),
                                   QStringLiteral("\u6db2\u4f53\u5bc6\u5ea6(kg/m^3)"),
                                   QStringLiteral("\u7a20\u5ea6\u7cfb\u6570(Pa.s)"),
                                   QStringLiteral("\u6d41\u6027\u6307\u6570"),
                                   QStringLiteral("\u6700\u5927\u7c98\u5ea6(mPa.s)")};
    const QStringList proppantColumns{QStringLiteral("\u652f\u6491\u5242\u540d\u79f0"),
                                      QStringLiteral("\u652f\u6491\u5242\u578b\u53f7"),
                                      QStringLiteral("\u5e73\u5747\u7c92\u5f84(mm)"),
                                      QStringLiteral("\u771f\u5b9e\u5bc6\u5ea6(kg/m^3)")};
    IDOSFractureMaterialCatalogObject::Kind kind = IDOSFractureMaterialCatalogObject::Kind::Fluid;
    if (columns == proppantColumns)
    {
        kind = IDOSFractureMaterialCatalogObject::Kind::Proppant;
    }
    else if (columns != fluidColumns)
    {
        setLastError(QObject::tr("The fracture material catalog has an unsupported header."));
        return result;
    }
    QVariantList records;
    int lineNumber = 1;
    while (!stream.atEnd())
    {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;
        if (line.isEmpty())
        {
            continue;
        }
        const QStringList fields = line.split(QLatin1Char('\t'));
        if (fields.size() != columns.size())
        {
            setLastError(QObject::tr("Line %1: material record has the wrong number of columns.")
                             .arg(lineNumber));
            return result;
        }
        QVariantMap record;
        for (int index = 0; index < fields.size(); ++index)
        {
            const QString text = fields.at(index).trimmed();
            bool valid = false;
            const double value = text.toDouble(&valid);
            if (!text.isEmpty() && valid && std::isfinite(value))
            {
                record.insert(columns.at(index), value);
            }
            else
            {
                record.insert(columns.at(index), text);
            }
        }
        records.append(record);
    }
    if (records.isEmpty())
    {
        setLastError(QObject::tr("The fracture material catalog contains no records."));
        return result;
    }
    IDOSFractureMaterialCatalogObject* catalog = new IDOSFractureMaterialCatalogObject();
    catalog->setKind(kind);
    catalog->setName(QFileInfo(filePath).completeBaseName());
    catalog->setSourceFile(filePath);
    catalog->setColumns(columns);
    catalog->setRecords(records);
    result.append(catalog);
    return result;
}
