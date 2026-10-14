#include <cmath>

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include "idossimulationcaseobject.h"

#include "idosfluidrocktextprovider.h"

QStringList IDOSFluidRockTextProvider::supportedKeywords() const
{
    return QStringList{QStringLiteral("DENSITY"), QStringLiteral("PVTO"), QStringLiteral("PVDO"),
                       QStringLiteral("PVDG"), QStringLiteral("PVTG"), QStringLiteral("PVZG"),
                       QStringLiteral("PVTW"), QStringLiteral("SWOF"), QStringLiteral("SGOF"),
                       QStringLiteral("SWFN"), QStringLiteral("SGFN"), QStringLiteral("SOF2"),
                       QStringLiteral("SOF3"), QStringLiteral("SLGOF"), QStringLiteral("SGWFN"),
                       QStringLiteral("ROCK")};
}

QStringList IDOSFluidRockTextProvider::columns(const QString& keyword) const
{
    if (keyword == QStringLiteral("DENSITY"))
    {
        return QStringList{QStringLiteral("Oil density"), QStringLiteral("Water density"),
                           QStringLiteral("Gas density")};
    }
    if (keyword == QStringLiteral("PVTO"))
    {
        return QStringList{QStringLiteral("Rs"), QStringLiteral("Pressure"), QStringLiteral("Bo"),
                           QStringLiteral("Viscosity")};
    }
    if (keyword == QStringLiteral("PVDO"))
    {
        return QStringList{QStringLiteral("Pressure"), QStringLiteral("Bo"), QStringLiteral("Viscosity")};
    }
    if (keyword == QStringLiteral("PVDG"))
    {
        return QStringList{QStringLiteral("Pressure"), QStringLiteral("Bg"), QStringLiteral("Viscosity")};
    }
    if (keyword == QStringLiteral("PVTG"))
    {
        return QStringList{QStringLiteral("Rv"), QStringLiteral("Pressure"), QStringLiteral("Bg"),
                           QStringLiteral("Viscosity")};
    }
    if (keyword == QStringLiteral("PVZG"))
    {
        return QStringList{QStringLiteral("Pressure"), QStringLiteral("Z factor"),
                           QStringLiteral("Viscosity")};
    }
    if (keyword == QStringLiteral("PVTW"))
    {
        return QStringList{QStringLiteral("Reference pressure"), QStringLiteral("Bw"),
                           QStringLiteral("Compressibility"), QStringLiteral("Viscosity"),
                           QStringLiteral("Viscosibility")};
    }
    if (keyword == QStringLiteral("SWOF"))
    {
        return QStringList{QStringLiteral("Sw"), QStringLiteral("Krw"), QStringLiteral("Krow"),
                           QStringLiteral("Pcow")};
    }
    if (keyword == QStringLiteral("SGOF"))
    {
        return QStringList{QStringLiteral("Sg"), QStringLiteral("Krg"), QStringLiteral("Krog"),
                           QStringLiteral("Pcg")};
    }
    if (keyword == QStringLiteral("SWFN"))
    {
        return QStringList{QStringLiteral("Sw"), QStringLiteral("Krw"), QStringLiteral("Pcow")};
    }
    if (keyword == QStringLiteral("SGFN"))
    {
        return QStringList{QStringLiteral("Sg"), QStringLiteral("Krg"), QStringLiteral("Pcg")};
    }
    if (keyword == QStringLiteral("SOF2"))
    {
        return QStringList{QStringLiteral("So"), QStringLiteral("Kro"), QStringLiteral("Krow")};
    }
    if (keyword == QStringLiteral("SOF3"))
    {
        return QStringList{QStringLiteral("So"), QStringLiteral("Kro"), QStringLiteral("Krow"),
                           QStringLiteral("Krog")};
    }
    if (keyword == QStringLiteral("SLGOF"))
    {
        return QStringList{QStringLiteral("Sl"), QStringLiteral("Krow"), QStringLiteral("Krog"),
                           QStringLiteral("Pcgo")};
    }
    if (keyword == QStringLiteral("SGWFN"))
    {
        return QStringList{QStringLiteral("Sg"), QStringLiteral("Krg"), QStringLiteral("Pcgw")};
    }
    return QStringList{QStringLiteral("Pressure"), QStringLiteral("Compressibility")};
}

bool IDOSFluidRockTextProvider::parseTableFile(const QString& filePath, QVariantList* tables,
                                               QString* error) const
{
    if (tables == nullptr || error == nullptr)
    {
        return false;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        *error = QObject::tr("The fluid and rock table file could not be opened.");
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    const QRegularExpression whitespace(QStringLiteral("\\s+"));
    const QStringList keywords = supportedKeywords();
    QString currentKeyword;
    QVariantList rows;
    int tableIndex = 0;
    bool terminated = true;
    while (!stream.atEnd())
    {
        const QString sourceLine = stream.readLine();
        QString line = sourceLine;
        const int commentIndex = line.indexOf(QStringLiteral("--"));
        if (commentIndex >= 0)
        {
            line = line.left(commentIndex);
        }
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) ||
            line.startsWith(QStringLiteral("//")))
        {
            continue;
        }

        const QStringList tokens = line.split(whitespace, Qt::SkipEmptyParts);
        const QString firstToken = tokens.isEmpty() ? QString() : tokens.first().toUpper();
        if (tokens.size() == 1 && keywords.contains(firstToken))
        {
            if (!currentKeyword.isEmpty() && !rows.isEmpty())
            {
                *error = QObject::tr("The %1 table is missing its terminating slash.").arg(currentKeyword);
                return false;
            }
            currentKeyword = firstToken;
            rows.clear();
            terminated = false;
            continue;
        }
        if (currentKeyword.isEmpty())
        {
            continue;
        }

        QStringList valueTokens = tokens;
        const bool endsWithSlash = valueTokens.last().endsWith(QLatin1Char('/'));
        if (endsWithSlash)
        {
            valueTokens.last().chop(1);
            if (valueTokens.last().isEmpty())
            {
                valueTokens.removeLast();
            }
        }
        const bool slashOnly = valueTokens.isEmpty() ||
            (valueTokens.size() == 1 && valueTokens.first() == QStringLiteral("/"));
        if (slashOnly)
        {
            if ((currentKeyword == QStringLiteral("PVTO") || currentKeyword == QStringLiteral("PVTG")) &&
                !rows.isEmpty())
            {
                QVariantMap table;
                table.insert(QStringLiteral("keyword"), currentKeyword);
                table.insert(QStringLiteral("tableIndex"), ++tableIndex);
                table.insert(QStringLiteral("columns"), columns(currentKeyword));
                table.insert(QStringLiteral("rows"), rows);
                tables->append(table);
                rows.clear();
                currentKeyword.clear();
                terminated = true;
            }
            else
            {
                if (!rows.isEmpty())
                {
                    QVariantMap table;
                    table.insert(QStringLiteral("keyword"), currentKeyword);
                    table.insert(QStringLiteral("tableIndex"), ++tableIndex);
                    table.insert(QStringLiteral("columns"), columns(currentKeyword));
                    table.insert(QStringLiteral("rows"), rows);
                    tables->append(table);
                }
                rows.clear();
                currentKeyword.clear();
                terminated = true;
            }
            continue;
        }

        const bool pvtoContinuation = (currentKeyword == QStringLiteral("PVTO") ||
            currentKeyword == QStringLiteral("PVTG")) && !rows.isEmpty() &&
            valueTokens.size() == columns(currentKeyword).size() - 1;
        if (pvtoContinuation)
        {
            valueTokens.prepend(QString());
        }
        QVariantList row;
        for (const QString& token : valueTokens)
        {
            if (token.isEmpty() || token == QStringLiteral("*"))
            {
                row.append(QVariant());
                continue;
            }
            QString normalized = token;
            normalized.replace(QLatin1Char('D'), QLatin1Char('E'), Qt::CaseInsensitive);
            bool ok = false;
            const double value = normalized.toDouble(&ok);
            if (!ok || !std::isfinite(value))
            {
                *error = QObject::tr("The %1 table contains an invalid numeric value: %2")
                             .arg(currentKeyword, token);
                return false;
            }
            row.append(value);
        }
        if (row.size() > columns(currentKeyword).size())
        {
            *error = QObject::tr("A row in the %1 table has too many columns.").arg(currentKeyword);
            return false;
        }
        while (row.size() < columns(currentKeyword).size())
        {
            row.append(QVariant());
        }
        rows.append(row);

        if (endsWithSlash && currentKeyword != QStringLiteral("PVTO") &&
            currentKeyword != QStringLiteral("PVTG"))
        {
            QVariantMap table;
            table.insert(QStringLiteral("keyword"), currentKeyword);
            table.insert(QStringLiteral("tableIndex"), ++tableIndex);
            table.insert(QStringLiteral("columns"), columns(currentKeyword));
            table.insert(QStringLiteral("rows"), rows);
            tables->append(table);
            rows.clear();
            currentKeyword.clear();
            terminated = true;
        }
    }

    if (!currentKeyword.isEmpty() || !terminated)
    {
        *error = QObject::tr("The %1 table is missing its terminating slash.").arg(currentKeyword);
        return false;
    }
    if (tables->isEmpty())
    {
        *error = QObject::tr("The file contains no supported ECLIPSE fluid or rock tables.");
        return false;
    }
    return true;
}

QList<IDOSDataObject*> IDOSFluidRockTextProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;
    const QFileInfo fileInfo(filePath);
    const QString suffix = fileInfo.suffix().toUpper();
    if (!fileInfo.isFile() || !fileInfo.isReadable() ||
        (suffix != QStringLiteral("TXT") && suffix != QStringLiteral("DAT") &&
         suffix != QStringLiteral("PRN")))
    {
        setLastError(QObject::tr("The fluid and rock table file is unavailable or has an unsupported extension."));
        return result;
    }

    QVariantList tables;
    QString error;
    if (!parseTableFile(filePath, &tables, &error))
    {
        setLastError(error);
        return result;
    }
    IDOSSimulationCaseObject* importedTables = new IDOSSimulationCaseObject();
    importedTables->setName(fileInfo.completeBaseName());
    importedTables->setSourceFile(fileInfo.absoluteFilePath());
    importedTables->setCurveTables(tables);
    result.append(importedTables);
    return result;
}
