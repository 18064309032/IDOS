#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <tuple>
#include <vector>

#include <QFileInfo>
#include <QObject>
#include <QStringList>
#include <QVector>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ErrorGuard.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/io/eclipse/EInit.hpp>
#include <opm/io/eclipse/EclIOdata.hpp>

#include "idosgridproperty.h"

#include "idosgridpropertyeclipseprovider.h"

QStringList IDOSGridPropertyEclipseProvider::propertyKeywords() const
{
    return QStringList{
        QStringLiteral("PORO"), QStringLiteral("PERMX"), QStringLiteral("PERMY"),
        QStringLiteral("PERMZ"), QStringLiteral("MULTPV"), QStringLiteral("SIGMAV"),
        QStringLiteral("MULTX"), QStringLiteral("MULTY"), QStringLiteral("MULTZ"),
        QStringLiteral("SATNUM"), QStringLiteral("ROCKNUM"), QStringLiteral("PVTNUM"),
        QStringLiteral("EQLNUM"), QStringLiteral("FIPNUM")};
}

QVector<double> IDOSGridPropertyEclipseProvider::values(const Opm::DeckKeyword& keyword) const
{
    QVector<double> result;
    for (std::size_t recordIndex = 0; recordIndex < keyword.size(); ++recordIndex)
    {
        const Opm::DeckRecord& record = keyword.getRecord(recordIndex);
        for (std::size_t itemIndex = 0; itemIndex < record.size(); ++itemIndex)
        {
            const Opm::DeckItem& item = record.getItem(itemIndex);
            for (std::size_t valueIndex = 0; valueIndex < item.data_size(); ++valueIndex)
            {
                if (!item.hasValue(valueIndex))
                {
                    continue;
                }
                if (item.getType() == Opm::get_type<int>())
                {
                    result.append(static_cast<double>(item.get<int>(valueIndex)));
                }
                else if (item.getType() == Opm::get_type<double>())
                {
                    if (item.getActiveDimensions().empty())
                    {
                        result.append(item.get<double>(valueIndex));
                    }
                    else
                    {
                        result.append(item.getSIDouble(valueIndex));
                    }
                }
            }
        }
    }
    return result;
}

void IDOSGridPropertyEclipseProvider::appendProperty(QList<IDOSDataObject*>* result,
                                                     const QString& keyword,
                                                     const QVector<double>& propertyValues) const
{
    if (result == nullptr || keyword.isEmpty() || propertyValues.isEmpty())
    {
        return;
    }

    IDOSGridProperty* property = new IDOSGridProperty();
    property->setName(keyword);
    property->setKeyword(keyword);
    property->setKind(IDOSGridProperty::Kind::Static);
    property->setDimensions(propertyValues.size(), 1, 1);
    if (!property->setValues(propertyValues))
    {
        delete property;
        return;
    }
    result->append(property);
}

QList<IDOSDataObject*> IDOSGridPropertyEclipseProvider::read(const QString& filePath)
{
    setLastError(QString());
    QList<IDOSDataObject*> result;
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.isFile() || !fileInfo.isReadable())
    {
        setLastError(QObject::tr("The ECLIPSE grid property file is unavailable or unreadable."));
        return result;
    }

    try
    {
        if (fileInfo.suffix().compare(QStringLiteral("INIT"), Qt::CaseInsensitive) == 0)
        {
            Opm::EclIO::EInit initFile(filePath.toStdString());
            const int activeCellCount = initFile.activeCells();
            const std::vector<Opm::EclIO::EclFile::EclEntry> arrays = initFile.list_arrays();
            for (const Opm::EclIO::EclFile::EclEntry& entry : arrays)
            {
                const QString keyword = QString::fromStdString(std::get<0>(entry)).trimmed();
                const Opm::EclIO::eclArrType type = std::get<1>(entry);
                const std::int64_t arraySize = std::get<2>(entry);
                if (keyword.isEmpty() || arraySize != activeCellCount ||
                    (type != Opm::EclIO::INTE && type != Opm::EclIO::REAL && type != Opm::EclIO::DOUB))
                {
                    continue;
                }

                QVector<double> propertyValues;
                if (type == Opm::EclIO::INTE)
                {
                    const std::vector<int>& values = initFile.getInitData<int>(keyword.toStdString());
                    propertyValues.reserve(static_cast<int>(values.size()));
                    for (int value : values)
                    {
                        propertyValues.append(static_cast<double>(value));
                    }
                }
                else if (type == Opm::EclIO::REAL)
                {
                    const std::vector<float>& values = initFile.getInitData<float>(keyword.toStdString());
                    propertyValues.reserve(static_cast<int>(values.size()));
                    for (float value : values)
                    {
                        propertyValues.append(static_cast<double>(value));
                    }
                }
                else
                {
                    const std::vector<double>& values = initFile.getInitData<double>(keyword.toStdString());
                    propertyValues.reserve(static_cast<int>(values.size()));
                    for (double value : values)
                    {
                        propertyValues.append(value);
                    }
                }
                if (propertyValues.isEmpty())
                {
                    continue;
                }
                appendProperty(&result, keyword, propertyValues);
            }
        }
        else
        {
            Opm::Parser parser;
            Opm::ParseContext context;
            Opm::ErrorGuard errors;
            const Opm::Deck deck = parser.parseFile(filePath.toStdString(), context, errors);
            if (errors)
            {
                setLastError(QObject::tr("OPM could not parse the grid property file: %1")
                                 .arg(QString::fromStdString(errors.formattedErrors())));
                return result;
            }

            for (const QString& propertyKeyword : propertyKeywords())
            {
                const std::vector<const Opm::DeckKeyword*> matches =
                    deck.getKeywordList(propertyKeyword.toStdString());
                QVector<double> propertyValues;
                for (const Opm::DeckKeyword* match : matches)
                {
                    if (match != nullptr)
                    {
                        propertyValues = values(*match);
                    }
                }
                appendProperty(&result, propertyKeyword, propertyValues);
            }
        }
    }
    catch (const std::exception& error)
    {
        setLastError(QObject::tr("OPM failed to read the grid property file: %1")
                         .arg(QString::fromStdString(error.what())));
        for (IDOSDataObject* object : result)
        {
            delete object;
        }
        result.clear();
        return result;
    }

    if (result.isEmpty())
    {
        setLastError(QObject::tr("The file contains no supported ECLIPSE grid properties."));
    }
    return result;
}
