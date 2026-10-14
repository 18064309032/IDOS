#include <QObject>

#include "idosprojectmetadata.h"

IDOSProjectMetadata::IDOSProjectMetadata()
    :
    m_unitSystem(UnitSystem::Metric)
    , m_coordinateType(CoordinateType::Unspecified)
{
}

QString IDOSProjectMetadata::name() const
{
    return m_name;
}

void IDOSProjectMetadata::setName(const QString& name)
{
    m_name = name;
}

QString IDOSProjectMetadata::fieldBlock() const
{
    return m_fieldBlock;
}

void IDOSProjectMetadata::setFieldBlock(const QString& fieldBlock)
{
    m_fieldBlock = fieldBlock;
}

QString IDOSProjectMetadata::description() const
{
    return m_description;
}

void IDOSProjectMetadata::setDescription(const QString& description)
{
    m_description = description;
}

IDOSProjectMetadata::UnitSystem IDOSProjectMetadata::unitSystem() const
{
    return m_unitSystem;
}

void IDOSProjectMetadata::setUnitSystem(UnitSystem unitSystem)
{
    m_unitSystem = unitSystem;
}

IDOSProjectMetadata::CoordinateType IDOSProjectMetadata::coordinateType() const
{
    return m_coordinateType;
}

void IDOSProjectMetadata::setCoordinateType(CoordinateType coordinateType)
{
    m_coordinateType = coordinateType;
}

QString IDOSProjectMetadata::coordinateReference() const
{
    return m_coordinateReference;
}

void IDOSProjectMetadata::setCoordinateReference(const QString& coordinateReference)
{
    m_coordinateReference = coordinateReference;
}

QString IDOSProjectMetadata::verticalDatum() const
{
    return m_verticalDatum;
}

void IDOSProjectMetadata::setVerticalDatum(const QString& verticalDatum)
{
    m_verticalDatum = verticalDatum;
}

QVariantMap IDOSProjectMetadata::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("name"), QVariant::fromValue(m_name));
    data.insert(QStringLiteral("fieldBlock"), QVariant::fromValue(m_fieldBlock));
    data.insert(QStringLiteral("description"), QVariant::fromValue(m_description));
    data.insert(QStringLiteral("unitSystem"), static_cast<int>(m_unitSystem));
    data.insert(QStringLiteral("coordinateType"), static_cast<int>(m_coordinateType));
    data.insert(QStringLiteral("coordinateReference"), QVariant::fromValue(m_coordinateReference));
    data.insert(QStringLiteral("verticalDatum"), QVariant::fromValue(m_verticalDatum));
    return data;
}

bool IDOSProjectMetadata::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("name")) || data.value(QStringLiteral("name")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("name"));
        return false;
    }
    if (!data.contains(QStringLiteral("fieldBlock")) || data.value(QStringLiteral("fieldBlock")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("fieldBlock"));
        return false;
    }
    if (!data.contains(QStringLiteral("description")) || data.value(QStringLiteral("description")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("description"));
        return false;
    }
    if (!data.contains(QStringLiteral("unitSystem")) || data.value(QStringLiteral("unitSystem")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("unitSystem"));
        return false;
    }
    if (data.value(QStringLiteral("unitSystem")).toInt() < 0 || data.value(QStringLiteral("unitSystem")).toInt() >= 3)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    if (!data.contains(QStringLiteral("coordinateType")) || data.value(QStringLiteral("coordinateType")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("coordinateType"));
        return false;
    }
    if (data.value(QStringLiteral("coordinateType")).toInt() < 0 || data.value(QStringLiteral("coordinateType")).toInt() >= 3)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    if (!data.contains(QStringLiteral("coordinateReference")) || data.value(QStringLiteral("coordinateReference")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("coordinateReference"));
        return false;
    }
    if (!data.contains(QStringLiteral("verticalDatum")) || data.value(QStringLiteral("verticalDatum")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("verticalDatum"));
        return false;
    }
    m_name = data.value(QStringLiteral("name")).value<QString>();
    m_fieldBlock = data.value(QStringLiteral("fieldBlock")).value<QString>();
    m_description = data.value(QStringLiteral("description")).value<QString>();
    m_unitSystem = static_cast<UnitSystem>(data.value(QStringLiteral("unitSystem")).toInt());
    m_coordinateType = static_cast<CoordinateType>(data.value(QStringLiteral("coordinateType")).toInt());
    m_coordinateReference = data.value(QStringLiteral("coordinateReference")).value<QString>();
    m_verticalDatum = data.value(QStringLiteral("verticalDatum")).value<QString>();
    return true;
}
