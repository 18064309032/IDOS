#include <climits>
#include <limits>

#include <QDataStream>
#include <QDate>
#include <QDateTime>
#include <QObject>
#include <QTime>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include <vtk_hdf5.h>

#include "idosdataobject.h"

#include "idosdatametadata.h"

IDOSDataMetadata::IDOSDataMetadata() = default;
IDOSDataMetadata::~IDOSDataMetadata() = default;

bool IDOSDataMetadata::writeProjectData(qint64 group, const IDOSDataObject* object, QString& error) const
{
    const QVariantMap data = object != nullptr ? object->projectData() : QVariantMap();
    if (object == nullptr || object->typeId() != typeId() || data.isEmpty())
    {
        error = QObject::tr("This object type does not support project storage: %1.").arg(typeId());
        return false;
    }
    return writeValue(group, "Data", data, error);
}

IDOSDataObject* IDOSDataMetadata::readProjectData(qint64 group, QString& error) const
{
    QVariant value;
    if (!readValue(group, "Data", value, error) || value.type() != QVariant::Map)
    {
        return nullptr;
    }
    IDOSDataObject* object = create();
    if (object == nullptr || object->typeId() != typeId() || !object->restoreProjectData(value.toMap(), error) || object->typeId() != typeId())
    {
        delete object;
        return nullptr;
    }
    return object;
}

bool IDOSDataMetadata::writeArray(qint64 group, qint64 fileType, qint64 memoryType,
                                   const void* data, quint64 count)
{
    const hsize_t length = count;
    const hid_t space = H5Screate_simple(1, &length, nullptr);
    const hid_t properties = H5Pcreate(H5P_DATASET_CREATE);
    bool success = space >= 0 && properties >= 0;
    if (success && count >= 1024)
    {
        const hsize_t chunk = count < 65536 ? count : 65536;
        success = H5Pset_chunk(properties, 1, &chunk) >= 0 && H5Pset_deflate(properties, 1) >= 0 &&
            H5Pset_fletcher32(properties) >= 0;
    }
    const hid_t dataset = success ? H5Dcreate2(group, "Value", fileType, space,
        H5P_DEFAULT, properties, H5P_DEFAULT) : -1;
    success = dataset >= 0;
    if (success && count > 0)
    {
        success = H5Dwrite(dataset, memoryType, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) >= 0;
    }
    if (dataset >= 0)
    {
        success = H5Dclose(dataset) >= 0 && success;
    }
    if (properties >= 0)
    {
        H5Pclose(properties);
    }
    if (space >= 0)
    {
        H5Sclose(space);
    }
    return success;
}

qint64 IDOSDataMetadata::arraySize(qint64 group)
{
    hid_t dataset = -1;
    H5E_BEGIN_TRY
    {
        dataset = H5Dopen2(group, "Value", H5P_DEFAULT);
    }
    H5E_END_TRY
    if (dataset < 0)
    {
        return -1;
    }
    const hid_t space = H5Dget_space(dataset);
    hsize_t length = 0;
    const bool valid = space >= 0 && H5Sget_simple_extent_ndims(space) == 1 &&
        H5Sget_simple_extent_dims(space, &length, nullptr) >= 0 && length <= INT_MAX / 8;
    if (space >= 0)
    {
        H5Sclose(space);
    }
    H5Dclose(dataset);
    return valid ? static_cast<qint64>(length) : -1;
}

bool IDOSDataMetadata::readArray(qint64 group, qint64 fileType, qint64 memoryType,
                                  void* data, quint64 count)
{
    const hid_t dataset = H5Dopen2(group, "Value", H5P_DEFAULT);
    if (dataset < 0)
    {
        return false;
    }
    const hid_t type = H5Dget_type(dataset);
    const hid_t properties = H5Dget_create_plist(dataset);
    const hid_t space = H5Dget_space(dataset);
    hsize_t length = 0;
    bool success = type >= 0 && space >= 0 && H5Tequal(type, fileType) > 0 &&
        H5Sget_simple_extent_ndims(space) == 1 && H5Sget_simple_extent_dims(space, &length, nullptr) >= 0 && length == count;
    if (space >= 0)
    {
        H5Sclose(space);
    }
    success = success && properties >= 0 && H5Pget_external_count(properties) == 0 &&
        H5Pget_layout(properties) != H5D_VIRTUAL;
    if (success && count > 0)
    {
        success = H5Dread(dataset, memoryType, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) >= 0;
    }
    if (type >= 0)
    {
        H5Tclose(type);
    }
    if (properties >= 0)
    {
        H5Pclose(properties);
    }
    H5Dclose(dataset);
    return success;
}

bool IDOSDataMetadata::writeValue(qint64 parent, const QByteArray& name, const QVariant& value, QString& error)
{
    return writePacked(parent, name, value, error);
}

bool IDOSDataMetadata::readValue(qint64 parent, const QByteArray& name, QVariant& value, QString& error)
{
    quint64 nodes = 0;
    return readNode(parent, name, value, 0, nodes, error);
}

bool IDOSDataMetadata::writeNode(qint64 parent, const QByteArray& name, const QVariant& value,
                                  int depth, QString& error)
{
    if (depth > 64)
    {
        error = QObject::tr("Project data nesting exceeds the supported limit.");
        return false;
    }
    const hid_t group = H5Gcreate2(parent, name.constData(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (group < 0)
    {
        error = QObject::tr("Cannot create a project data block.");
        return false;
    }
    int kind = 0;
    bool success = true;
    if (!value.isValid())
    {
        kind = 0;
    }
    else if (value.type() == QVariant::Map)
    {
        kind = 1;
        const QVariantMap map = value.toMap();
        for (QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd() && success; ++it)
        {
            success = writeNode(group, QByteArray("k") + it.key().toUtf8().toHex(), it.value(), depth + 1, error);
        }
    }
    else if (value.type() == QVariant::List || value.type() == QVariant::StringList)
    {
        kind = value.type() == QVariant::StringList ? 3 : 2;
        const QVariantList list = value.toList();
        for (int index = 0; index < list.size() && success; ++index)
        {
            success = writeNode(group, QByteArray::number(index), list.at(index), depth + 1, error);
        }
    }
    else if (value.type() == QVariant::String || value.type() == QVariant::ByteArray ||
             value.type() == QVariant::Date || value.type() == QVariant::Time || value.type() == QVariant::DateTime)
    {
        QByteArray bytes;
        if (value.type() == QVariant::ByteArray)
        {
            kind = 5;
            bytes = value.toByteArray();
        }
        else
        {
            kind = value.type() == QVariant::String ? 4 : value.type() == QVariant::Date ? 15
                : value.type() == QVariant::Time ? 16 : 17;
            const QString text = kind == 15 ? value.toDate().toString(Qt::ISODate)
                : kind == 16 ? value.toTime().toString(Qt::ISODateWithMs)
                : kind == 17 ? value.toDateTime().toString(Qt::ISODateWithMs) : value.toString();
            bytes = text.toUtf8();
        }
        success = writeArray(group, H5T_STD_U8LE, H5T_NATIVE_UCHAR, bytes.constData(), bytes.size());
    }
    else if (value.type() == QVariant::Bool || value.type() == QVariant::Int || value.type() == QVariant::LongLong)
    {
        kind = value.type() == QVariant::Bool ? 6 : value.type() == QVariant::Int ? 7 : 8;
        const qint64 number = value.toLongLong();
        success = writeArray(group, H5T_STD_I64LE, H5T_NATIVE_LLONG, &number, 1);
    }
    else if (value.type() == QVariant::UInt || value.type() == QVariant::ULongLong)
    {
        kind = value.type() == QVariant::UInt ? 18 : 19;
        const quint64 number = value.toULongLong();
        success = writeArray(group, H5T_STD_U64LE, H5T_NATIVE_ULLONG, &number, 1);
    }
    else if (value.type() == QVariant::Double || value.userType() == QMetaType::Float)
    {
        kind = value.type() == QVariant::Double ? 9 : 20;
        const double number = value.toDouble();
        success = writeArray(group, H5T_IEEE_F64LE, H5T_NATIVE_DOUBLE, &number, 1);
    }
    else if (value.userType() == qMetaTypeId<QVector<double>>())
    {
        kind = 10;
        const QVector<double> values = value.value<QVector<double>>();
        success = writeArray(group, H5T_IEEE_F64LE, H5T_NATIVE_DOUBLE, values.constData(), values.size());
    }
    else if (value.userType() == qMetaTypeId<QVector<int>>())
    {
        kind = 11;
        const QVector<int> values = value.value<QVector<int>>();
        success = writeArray(group, H5T_STD_I32LE, H5T_NATIVE_INT, values.constData(), values.size());
    }
    else if (value.userType() == qMetaTypeId<QVector<float>>())
    {
        kind = 12;
        const QVector<float> values = value.value<QVector<float>>();
        success = writeArray(group, H5T_IEEE_F32LE, H5T_NATIVE_FLOAT, values.constData(), values.size());
    }
    else if (value.userType() == qMetaTypeId<QVector<qint64>>())
    {
        kind = 13;
        const QVector<qint64> values = value.value<QVector<qint64>>();
        success = writeArray(group, H5T_STD_I64LE, H5T_NATIVE_LLONG, values.constData(), values.size());
    }
    else
    {
        success = false;
        error = QObject::tr("An unsupported project value type was encountered: %1.").arg(QString::fromLatin1(value.typeName()));
    }
    const hid_t space = H5Screate(H5S_SCALAR);
    const hid_t attribute = space >= 0 ? H5Acreate2(group, "Kind", H5T_STD_I32LE, space, H5P_DEFAULT, H5P_DEFAULT) : -1;
    success = attribute >= 0 && H5Awrite(attribute, H5T_NATIVE_INT, &kind) >= 0 && success;
    if (attribute >= 0)
    {
        H5Aclose(attribute);
    }
    if (space >= 0)
    {
        H5Sclose(space);
    }
    success = H5Gclose(group) >= 0 && success;
    if (!success && error.isEmpty())
    {
        error = QObject::tr("Cannot write a project data block.");
    }
    return success;
}

bool IDOSDataMetadata::readNode(qint64 parent, const QByteArray& name, QVariant& value,
                                 int depth, quint64& nodes, QString& error)
{
    if (depth > 64 || ++nodes > 10000000)
    {
        error = QObject::tr("Project data exceed the supported structural limits.");
        return false;
    }
    hid_t group = -1;
    H5L_info_t linkInfo;
    H5E_BEGIN_TRY
    {
        if (H5Lget_info(parent, name.constData(), &linkInfo, H5P_DEFAULT) >= 0 && linkInfo.type == H5L_TYPE_HARD)
        {
            group = H5Gopen2(parent, name.constData(), H5P_DEFAULT);
        }
    }
    H5E_END_TRY
    if (group < 0)
    {
        error = QObject::tr("A required project data block is missing.");
        return false;
    }
    int kind = -1;
    hid_t attribute = -1;
    H5E_BEGIN_TRY
    {
        attribute = H5Aopen(group, "Kind", H5P_DEFAULT);
    }
    H5E_END_TRY
    bool success = attribute >= 0;
    if (success)
    {
        const hid_t space = H5Aget_space(attribute);
        success = space >= 0 && H5Sget_simple_extent_npoints(space) == 1 &&
            H5Aread(attribute, H5T_NATIVE_INT, &kind) >= 0;
        if (space >= 0)
        {
            H5Sclose(space);
        }
        H5Aclose(attribute);
    }
    if (success && kind == 21)
    {
        success = readPacked(group, value, depth, nodes, error);
    }
    else if (success && (kind == 1 || kind == 2 || kind == 3))
    {
        H5G_info_t info;
        success = H5Gget_info(group, &info) >= 0 && info.nlinks <= 1000000;
        QVariantMap map;
        QVariantList list;
        QStringList strings;
        for (hsize_t index = 0; success && index < info.nlinks; ++index)
        {
            QByteArray child;
            if (kind == 1)
            {
                const ssize_t size = H5Lget_name_by_idx(group, ".", H5_INDEX_NAME, H5_ITER_INC,
                    index, nullptr, 0, H5P_DEFAULT);
                if (size < 1 || size > 65536)
                {
                    success = false;
                    break;
                }
                child.resize(static_cast<int>(size) + 1);
                success = H5Lget_name_by_idx(group, ".", H5_INDEX_NAME, H5_ITER_INC,
                    index, child.data(), child.size(), H5P_DEFAULT) == size;
                child.resize(static_cast<int>(size));
                const QByteArray decoded = QByteArray::fromHex(child.mid(1));
                if (!child.startsWith('k') || QByteArray("k") + decoded.toHex() != child ||
                    QString::fromUtf8(decoded).toUtf8() != decoded)
                {
                    success = false;
                    break;
                }
            }
            else
            {
                child = QByteArray::number(index);
            }
            H5L_info_t link;
            if (!success || H5Lget_info(group, child.constData(), &link, H5P_DEFAULT) < 0 || link.type != H5L_TYPE_HARD)
            {
                success = false;
                break;
            }
            QVariant entry;
            success = readNode(group, child, entry, depth + 1, nodes, error);
            if (kind == 1)
            {
                map.insert(QString::fromUtf8(QByteArray::fromHex(child.mid(1))), entry);
            }
            else if (kind == 3)
            {
                success = success && entry.type() == QVariant::String;
                strings.append(entry.toString());
            }
            else
            {
                list.append(entry);
            }
        }
        value = kind == 1 ? QVariant(map) : kind == 3 ? QVariant(strings) : QVariant(list);
    }
    else if (success && kind == 0)
    {
        value = QVariant();
    }
    else if (success)
    {
        const bool scalar = kind == 6 || kind == 7 || kind == 8 || kind == 9 || kind == 18 || kind == 19 || kind == 20;
        const qint64 count = scalar ? 1 : arraySize(group);
        success = count >= 0;
        if (success && (kind == 4 || kind == 5 || kind == 15 || kind == 16 || kind == 17))
        {
            QByteArray bytes(static_cast<int>(count), '\0');
            success = readArray(group, H5T_STD_U8LE, H5T_NATIVE_UCHAR, bytes.data(), count);
            const QString text = QString::fromUtf8(bytes);
            if (kind != 5 && text.toUtf8() != bytes)
            {
                success = false;
            }
            value = kind == 5 ? QVariant(bytes) : kind == 15 ? QVariant(QDate::fromString(text, Qt::ISODate))
                : kind == 16 ? QVariant(QTime::fromString(text, Qt::ISODateWithMs))
                : kind == 17 ? QVariant(QDateTime::fromString(text, Qt::ISODateWithMs)) : QVariant(text);
        }
        else if (success && (kind == 6 || kind == 7 || kind == 8))
        {
            qint64 number = 0;
            success = count == 1 && readArray(group, H5T_STD_I64LE, H5T_NATIVE_LLONG, &number, 1);
            success = success && (kind != 6 || number == 0 || number == 1) &&
                (kind != 7 || (number >= INT_MIN && number <= INT_MAX));
            value = kind == 6 ? QVariant(number != 0) : kind == 7 ? QVariant(static_cast<int>(number)) : QVariant(number);
        }
        else if (success && (kind == 18 || kind == 19))
        {
            quint64 number = 0;
            success = count == 1 && readArray(group, H5T_STD_U64LE, H5T_NATIVE_ULLONG, &number, 1) &&
                (kind != 18 || number <= UINT_MAX);
            value = kind == 18 ? QVariant(static_cast<uint>(number)) : QVariant(number);
        }
        else if (success && (kind == 9 || kind == 20))
        {
            double number = 0.0;
            success = count == 1 && readArray(group, H5T_IEEE_F64LE, H5T_NATIVE_DOUBLE, &number, 1);
            value = kind == 9 ? QVariant(number) : QVariant::fromValue(static_cast<float>(number));
        }
        else if (success && kind == 10)
        {
            QVector<double> values(static_cast<int>(count));
            success = readArray(group, H5T_IEEE_F64LE, H5T_NATIVE_DOUBLE, values.data(), count);
            value = QVariant::fromValue(values);
        }
        else if (success && kind == 11)
        {
            QVector<int> values(static_cast<int>(count));
            success = readArray(group, H5T_STD_I32LE, H5T_NATIVE_INT, values.data(), count);
            value = QVariant::fromValue(values);
        }
        else if (success && kind == 12)
        {
            QVector<float> values(static_cast<int>(count));
            success = readArray(group, H5T_IEEE_F32LE, H5T_NATIVE_FLOAT, values.data(), count);
            value = QVariant::fromValue(values);
        }
        else if (success && kind == 13)
        {
            QVector<qint64> values(static_cast<int>(count));
            success = readArray(group, H5T_STD_I64LE, H5T_NATIVE_LLONG, values.data(), count);
            value = QVariant::fromValue(values);
        }
        else
        {
            success = false;
        }
    }
    H5Gclose(group);
    if (!success && error.isEmpty())
    {
        error = QObject::tr("A project data block has an invalid type, size or value.");
    }
    return success;
}
bool IDOSDataMetadata::isArray(const QVariant& value)
{
    return value.userType() == qMetaTypeId<QVector<double>>() ||
        value.userType() == qMetaTypeId<QVector<float>>() ||
        value.userType() == qMetaTypeId<QVector<int>>() ||
        value.userType() == qMetaTypeId<QVector<qint64>>();
}

bool IDOSDataMetadata::isScalar(const QVariant& value)
{
    return !value.isValid() || value.type() == QVariant::String || value.type() == QVariant::StringList ||
        value.type() == QVariant::ByteArray || value.type() == QVariant::Bool || value.type() == QVariant::Int ||
        value.type() == QVariant::UInt || value.type() == QVariant::LongLong || value.type() == QVariant::ULongLong ||
        value.type() == QVariant::Double || value.userType() == QMetaType::Float ||
        value.type() == QVariant::Date || value.type() == QVariant::Time || value.type() == QVariant::DateTime;
}

QVariantMap IDOSDataMetadata::tableColumns(const QVariantList& rows)
{
    if (rows.size() < 2 || rows.first().type() != QVariant::Map)
    {
        return QVariantMap();
    }
    const QStringList keys = rows.first().toMap().keys();
    if (keys.isEmpty())
    {
        return QVariantMap();
    }
    for (const QVariant& row : rows)
    {
        if (row.type() != QVariant::Map)
        {
            return QVariantMap();
        }
        const QVariantMap fields = row.toMap();
        if (fields.size() != keys.size())
        {
            return QVariantMap();
        }
        for (const QString& key : keys)
        {
            if (!fields.contains(key))
            {
                return QVariantMap();
            }
        }
    }
    QVariantMap columns;
    for (const QString& key : keys)
    {
        QVariantList values;
        values.reserve(rows.size());
        const int type = rows.first().toMap().value(key).userType();
        bool uniform = true;
        for (const QVariant& row : rows)
        {
            const QVariant value = row.toMap().value(key);
            uniform = uniform && value.userType() == type;
            values.append(value);
        }
        if (uniform && type == QMetaType::Double)
        {
            QVector<double> numbers;
            numbers.reserve(values.size());
            for (const QVariant& value : values)
            {
                numbers.append(value.toDouble());
            }
            columns.insert(key, QVariant::fromValue(numbers));
        }
        else if (uniform && type == QMetaType::Int)
        {
            QVector<int> numbers;
            numbers.reserve(values.size());
            for (const QVariant& value : values)
            {
                numbers.append(value.toInt());
            }
            columns.insert(key, QVariant::fromValue(numbers));
        }
        else if (uniform && type == QMetaType::LongLong)
        {
            QVector<qint64> numbers;
            numbers.reserve(values.size());
            for (const QVariant& value : values)
            {
                numbers.append(value.toLongLong());
            }
            columns.insert(key, QVariant::fromValue(numbers));
        }
        else if (uniform && type == QMetaType::Float)
        {
            QVector<float> numbers;
            numbers.reserve(values.size());
            for (const QVariant& value : values)
            {
                numbers.append(value.value<float>());
            }
            columns.insert(key, QVariant::fromValue(numbers));
        }
        else if (uniform && type == QMetaType::QString)
        {
            QStringList strings;
            strings.reserve(values.size());
            for (const QVariant& value : values)
            {
                strings.append(value.toString());
            }
            columns.insert(key, strings);
        }
        else
        {
            columns.insert(key, values);
        }
    }
    return columns;
}

bool IDOSDataMetadata::encodePacked(QDataStream& stream, const QVariant& value,
                                      QList<QVariant>& arrays, int depth, QString& error)
{
    if (depth > 64)
    {
        error = QObject::tr("Project data nesting exceeds the supported limit.");
        return false;
    }
    if (isArray(value))
    {
        stream << qint32(20) << qint32(arrays.size());
        arrays.append(value);
    }
    else if (value.type() == QVariant::Map)
    {
        const QVariantMap map = value.toMap();
        stream << qint32(1) << qint32(map.size());
        for (QVariantMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
        {
            stream << it.key();
            if (!encodePacked(stream, it.value(), arrays, depth + 1, error))
            {
                return false;
            }
        }
    }
    else if (value.type() == QVariant::List)
    {
        const QVariantList list = value.toList();
        const QVariantMap columns = tableColumns(list);
        if (!columns.isEmpty())
        {
            stream << qint32(21) << qint32(list.size());
            return encodePacked(stream, columns, arrays, depth + 1, error);
        }
        stream << qint32(2) << qint32(list.size());
        for (const QVariant& entry : list)
        {
            if (!encodePacked(stream, entry, arrays, depth + 1, error))
            {
                return false;
            }
        }
    }
    else if (isScalar(value))
    {
        stream << qint32(0) << value;
    }
    else
    {
        error = QObject::tr("An unsupported project value type was encountered: %1.").arg(QString::fromLatin1(value.typeName()));
        return false;
    }
    return stream.status() == QDataStream::Ok;
}

int IDOSDataMetadata::columnSize(const QVariant& column)
{
    if (column.userType() == qMetaTypeId<QVector<double>>())
    {
        return column.value<QVector<double>>().size();
    }
    if (column.userType() == qMetaTypeId<QVector<float>>())
    {
        return column.value<QVector<float>>().size();
    }
    if (column.userType() == qMetaTypeId<QVector<int>>())
    {
        return column.value<QVector<int>>().size();
    }
    if (column.userType() == qMetaTypeId<QVector<qint64>>())
    {
        return column.value<QVector<qint64>>().size();
    }
    if (column.type() == QVariant::StringList)
    {
        return column.toStringList().size();
    }
    return column.type() == QVariant::List ? column.toList().size() : -1;
}

QVariant IDOSDataMetadata::columnValue(const QVariant& column, int index)
{
    if (column.userType() == qMetaTypeId<QVector<double>>())
    {
        return column.value<QVector<double>>().at(index);
    }
    if (column.userType() == qMetaTypeId<QVector<float>>())
    {
        return QVariant::fromValue(column.value<QVector<float>>().at(index));
    }
    if (column.userType() == qMetaTypeId<QVector<int>>())
    {
        return column.value<QVector<int>>().at(index);
    }
    if (column.userType() == qMetaTypeId<QVector<qint64>>())
    {
        return QVariant::fromValue(column.value<QVector<qint64>>().at(index));
    }
    if (column.type() == QVariant::StringList)
    {
        return column.toStringList().at(index);
    }
    return column.toList().at(index);
}

bool IDOSDataMetadata::decodePacked(QDataStream& stream, qint64 group, QVariant& value,
                                      int depth, int& arrayIndex, quint64& nodes, QString& error)
{
    if (depth > 64 || ++nodes > 10000000 || stream.atEnd())
    {
        return false;
    }
    qint32 kind = -1;
    stream >> kind;
    if (kind == 0)
    {
        QDataStream typeStream(stream.device()->peek(4));
        typeStream.setByteOrder(QDataStream::LittleEndian);
        quint32 type = 0;
        typeStream >> type;
        if (typeStream.status() != QDataStream::Ok ||
            (type != QMetaType::UnknownType && type != QMetaType::Bool && type != QMetaType::Int &&
             type != QMetaType::UInt && type != QMetaType::LongLong && type != QMetaType::ULongLong &&
             type != QMetaType::Double && type != QMetaType::Float && type != QMetaType::QString &&
             type != QMetaType::QStringList && type != QMetaType::QByteArray && type != QMetaType::QDate &&
             type != QMetaType::QTime && type != QMetaType::QDateTime))
        {
            return false;
        }
        stream >> value;
        return stream.status() == QDataStream::Ok && isScalar(value);
    }
    if (kind == 20)
    {
        qint32 index = -1;
        stream >> index;
        return index == arrayIndex++ && readNode(group, QByteArray("A") + QByteArray::number(index), value,
            depth + 1, nodes, error) && isArray(value);
    }
    qint32 count = -1;
    stream >> count;
    if (stream.status() != QDataStream::Ok || count < 0 || count > 1000000)
    {
        return false;
    }
    if (kind == 1)
    {
        QVariantMap map;
        for (int index = 0; index < count; ++index)
        {
            QString key;
            QVariant entry;
            stream >> key;
            if (stream.status() != QDataStream::Ok || map.contains(key) ||
                !decodePacked(stream, group, entry, depth + 1, arrayIndex, nodes, error))
            {
                return false;
            }
            map.insert(key, entry);
        }
        value = map;
    }
    else if (kind == 2)
    {
        QVariantList list;
        list.reserve(count);
        for (int index = 0; index < count; ++index)
        {
            QVariant entry;
            if (!decodePacked(stream, group, entry, depth + 1, arrayIndex, nodes, error))
            {
                return false;
            }
            list.append(entry);
        }
        value = list;
    }
    else if (kind == 21)
    {
        QVariant entry;
        if (!decodePacked(stream, group, entry, depth + 1, arrayIndex, nodes, error) || entry.type() != QVariant::Map)
        {
            return false;
        }
        const QVariantMap columns = entry.toMap();
        if (columns.isEmpty())
        {
            return false;
        }
        for (QVariantMap::const_iterator it = columns.constBegin(); it != columns.constEnd(); ++it)
        {
            if (columnSize(it.value()) != count)
            {
                return false;
            }
        }
        QVariantList rows;
        rows.reserve(count);
        for (int index = 0; index < count; ++index)
        {
            QVariantMap row;
            for (QVariantMap::const_iterator it = columns.constBegin(); it != columns.constEnd(); ++it)
            {
                row.insert(it.key(), columnValue(it.value(), index));
            }
            rows.append(row);
        }
        value = rows;
    }
    else
    {
        return false;
    }
    return stream.status() == QDataStream::Ok;
}

bool IDOSDataMetadata::writePacked(qint64 parent, const QByteArray& name, const QVariant& value, QString& error)
{
    QByteArray metadata;
    QDataStream stream(&metadata, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_15);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint32(0x49444f32);
    QList<QVariant> arrays;
    if (!encodePacked(stream, value, arrays, 0, error))
    {
        return false;
    }
    const hid_t group = H5Gcreate2(parent, name.constData(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (group < 0)
    {
        error = QObject::tr("Cannot create a project data block.");
        return false;
    }
    const int kind = 21;
    const hid_t space = H5Screate(H5S_SCALAR);
    const hid_t attribute = space >= 0 ? H5Acreate2(group, "Kind", H5T_STD_I32LE, space, H5P_DEFAULT, H5P_DEFAULT) : -1;
    bool success = attribute >= 0 && H5Awrite(attribute, H5T_NATIVE_INT, &kind) >= 0;
    if (attribute >= 0)
    {
        H5Aclose(attribute);
    }
    if (space >= 0)
    {
        H5Sclose(space);
    }
    success = success && writeNode(group, "Metadata", metadata, 0, error);
    for (int index = 0; success && index < arrays.size(); ++index)
    {
        success = writeNode(group, QByteArray("A") + QByteArray::number(index), arrays.at(index), 0, error);
    }
    success = H5Gclose(group) >= 0 && success;
    return success;
}

bool IDOSDataMetadata::readPacked(qint64 group, QVariant& value, int depth, quint64& nodes, QString& error)
{
    QVariant metadata;
    if (!readNode(group, "Metadata", metadata, depth + 1, nodes, error) || metadata.type() != QVariant::ByteArray)
    {
        return false;
    }
    QDataStream stream(metadata.toByteArray());
    stream.setVersion(QDataStream::Qt_5_15);
    stream.setByteOrder(QDataStream::LittleEndian);
    quint32 magic = 0;
    stream >> magic;
    int arrayIndex = 0;
    H5G_info_t info;
    return magic == 0x49444f32 && decodePacked(stream, group, value, depth + 1, arrayIndex, nodes, error) &&
        stream.status() == QDataStream::Ok && stream.atEnd() && H5Gget_info(group, &info) >= 0 &&
        info.nlinks == static_cast<hsize_t>(arrayIndex) + 1;
}
