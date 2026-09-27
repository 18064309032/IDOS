#include "idosprojectmetadata.h"

IDOSProjectMetadata::IDOSProjectMetadata()
    : m_unitSystem(UnitSystem::Metric)
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
