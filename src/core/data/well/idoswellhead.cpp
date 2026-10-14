#include <QObject>

#include "idoswellhead.h"

IDOSWellHead::IDOSWellHead()
    :
    m_surfaceX(0.0)
    , m_surfaceY(0.0)
    , m_hasSurfaceX(false)
    , m_hasSurfaceY(false)
    , m_surfaceElevation(0.0)
    , m_hasSurfaceElevation(false)
    , m_spatialReference()
    , m_waterDepth(0.0)
    , m_wellheadPressure(0.0)
    , m_wellheadTemperature(0.0)
    , m_topDepth(0.0)
    , m_bottomDepth(0.0)
    , m_kb(0.0)
    , m_symbol(0)
{
}

double IDOSWellHead::surfaceX() const
{
    return m_surfaceX;
}

void IDOSWellHead::setSurfaceX(double x)
{
    m_surfaceX = x;
    m_hasSurfaceX = true;
}

double IDOSWellHead::surfaceY() const
{
    return m_surfaceY;
}

void IDOSWellHead::setSurfaceY(double y)
{
    m_surfaceY = y;
    m_hasSurfaceY = true;
}

bool IDOSWellHead::hasSurfacePosition() const
{
    return m_hasSurfaceX && m_hasSurfaceY;
}

double IDOSWellHead::surfaceElevation() const
{
    return m_surfaceElevation;
}

void IDOSWellHead::setSurfaceElevation(double elevation)
{
    m_surfaceElevation = elevation;
    m_hasSurfaceElevation = true;
}

bool IDOSWellHead::hasSurfaceElevation() const
{
    return m_hasSurfaceElevation;
}

IDOSWellSpatialReference IDOSWellHead::spatialReference() const
{
    return m_spatialReference;
}

void IDOSWellHead::setSpatialReference(const IDOSWellSpatialReference& reference)
{
    m_spatialReference = reference;
}

double IDOSWellHead::waterDepth() const
{
    return m_waterDepth;
}

void IDOSWellHead::setWaterDepth(double depth)
{
    m_waterDepth = depth;
}

double IDOSWellHead::wellheadPressure() const
{
    return m_wellheadPressure;
}

void IDOSWellHead::setWellheadPressure(double pressure)
{
    m_wellheadPressure = pressure;
}

double IDOSWellHead::wellheadTemperature() const
{
    return m_wellheadTemperature;
}

void IDOSWellHead::setWellheadTemperature(double temperature)
{
    m_wellheadTemperature = temperature;
}

double IDOSWellHead::topDepth() const
{
    return m_topDepth;
}

void IDOSWellHead::setTopDepth(double depth)
{
    m_topDepth = depth;
}

double IDOSWellHead::bottomDepth() const
{
    return m_bottomDepth;
}

void IDOSWellHead::setBottomDepth(double depth)
{
    m_bottomDepth = depth;
}

double IDOSWellHead::kb() const
{
    return m_kb;
}

void IDOSWellHead::setKb(double value)
{
    m_kb = value;
}

int IDOSWellHead::symbol() const
{
    return m_symbol;
}

void IDOSWellHead::setSymbol(int value)
{
    m_symbol = value;
}

QVariantMap IDOSWellHead::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("surfaceX"), QVariant::fromValue(m_surfaceX));
    data.insert(QStringLiteral("surfaceY"), QVariant::fromValue(m_surfaceY));
    data.insert(QStringLiteral("hasSurfaceX"), QVariant::fromValue(m_hasSurfaceX));
    data.insert(QStringLiteral("hasSurfaceY"), QVariant::fromValue(m_hasSurfaceY));
    data.insert(QStringLiteral("surfaceElevation"), QVariant::fromValue(m_surfaceElevation));
    data.insert(QStringLiteral("hasSurfaceElevation"), QVariant::fromValue(m_hasSurfaceElevation));
    data.insert(QStringLiteral("spatialReference"), m_spatialReference.projectData());
    data.insert(QStringLiteral("waterDepth"), QVariant::fromValue(m_waterDepth));
    data.insert(QStringLiteral("wellheadPressure"), QVariant::fromValue(m_wellheadPressure));
    data.insert(QStringLiteral("wellheadTemperature"), QVariant::fromValue(m_wellheadTemperature));
    data.insert(QStringLiteral("topDepth"), QVariant::fromValue(m_topDepth));
    data.insert(QStringLiteral("bottomDepth"), QVariant::fromValue(m_bottomDepth));
    data.insert(QStringLiteral("kb"), QVariant::fromValue(m_kb));
    data.insert(QStringLiteral("symbol"), QVariant::fromValue(m_symbol));
    return data;
}

bool IDOSWellHead::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("surfaceX")) || data.value(QStringLiteral("surfaceX")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("surfaceX"));
        return false;
    }
    if (!data.contains(QStringLiteral("surfaceY")) || data.value(QStringLiteral("surfaceY")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("surfaceY"));
        return false;
    }
    if (!data.contains(QStringLiteral("hasSurfaceX")) || data.value(QStringLiteral("hasSurfaceX")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("hasSurfaceX"));
        return false;
    }
    if (!data.contains(QStringLiteral("hasSurfaceY")) || data.value(QStringLiteral("hasSurfaceY")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("hasSurfaceY"));
        return false;
    }
    if (!data.contains(QStringLiteral("surfaceElevation")) || data.value(QStringLiteral("surfaceElevation")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("surfaceElevation"));
        return false;
    }
    if (!data.contains(QStringLiteral("hasSurfaceElevation")) || data.value(QStringLiteral("hasSurfaceElevation")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("hasSurfaceElevation"));
        return false;
    }
    if (!data.contains(QStringLiteral("spatialReference")) || data.value(QStringLiteral("spatialReference")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("spatialReference"));
        return false;
    }
    if (!data.contains(QStringLiteral("waterDepth")) || data.value(QStringLiteral("waterDepth")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("waterDepth"));
        return false;
    }
    if (!data.contains(QStringLiteral("wellheadPressure")) || data.value(QStringLiteral("wellheadPressure")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("wellheadPressure"));
        return false;
    }
    if (!data.contains(QStringLiteral("wellheadTemperature")) || data.value(QStringLiteral("wellheadTemperature")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("wellheadTemperature"));
        return false;
    }
    if (!data.contains(QStringLiteral("topDepth")) || data.value(QStringLiteral("topDepth")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("topDepth"));
        return false;
    }
    if (!data.contains(QStringLiteral("bottomDepth")) || data.value(QStringLiteral("bottomDepth")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("bottomDepth"));
        return false;
    }
    if (!data.contains(QStringLiteral("kb")) || data.value(QStringLiteral("kb")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("kb"));
        return false;
    }
    if (!data.contains(QStringLiteral("symbol")) || data.value(QStringLiteral("symbol")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("symbol"));
        return false;
    }
    m_surfaceX = data.value(QStringLiteral("surfaceX")).value<double>();
    m_surfaceY = data.value(QStringLiteral("surfaceY")).value<double>();
    m_hasSurfaceX = data.value(QStringLiteral("hasSurfaceX")).value<bool>();
    m_hasSurfaceY = data.value(QStringLiteral("hasSurfaceY")).value<bool>();
    m_surfaceElevation = data.value(QStringLiteral("surfaceElevation")).value<double>();
    m_hasSurfaceElevation = data.value(QStringLiteral("hasSurfaceElevation")).value<bool>();
    if (!m_spatialReference.restoreProjectData(data.value(QStringLiteral("spatialReference")).toMap(), error))
    {
        return false;
    }
    m_waterDepth = data.value(QStringLiteral("waterDepth")).value<double>();
    m_wellheadPressure = data.value(QStringLiteral("wellheadPressure")).value<double>();
    m_wellheadTemperature = data.value(QStringLiteral("wellheadTemperature")).value<double>();
    m_topDepth = data.value(QStringLiteral("topDepth")).value<double>();
    m_bottomDepth = data.value(QStringLiteral("bottomDepth")).value<double>();
    m_kb = data.value(QStringLiteral("kb")).value<double>();
    m_symbol = data.value(QStringLiteral("symbol")).value<int>();
    return true;
}
