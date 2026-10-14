#include <QObject>

#include "idoswelllogchannel.h"

IDOSWellLogChannel::IDOSWellLogChannel()
    :
    m_visible(true)
{
}

IDOSWellLogChannel::IDOSWellLogChannel(const QString& name)
    :
    m_name(name)
    , m_visible(true)
{
}

QString IDOSWellLogChannel::name() const
{
    return m_name;
}
void IDOSWellLogChannel::setName(const QString& name)
{
    m_name = name;
}

QString IDOSWellLogChannel::unit() const
{
    return m_unit;
}
void IDOSWellLogChannel::setUnit(const QString& unit)
{
    m_unit = unit;
}

QVector<double> IDOSWellLogChannel::depths() const
{
    return m_depths;
}
QVector<double> IDOSWellLogChannel::values() const
{
    return m_values;
}

void IDOSWellLogChannel::setData(const QVector<double>& depths, const QVector<double>& values)
{
    m_depths = depths;
    m_values = values;
}

int IDOSWellLogChannel::sampleCount() const
{
    return m_values.size();
}

bool IDOSWellLogChannel::isVisible() const
{
    return m_visible;
}
void IDOSWellLogChannel::setVisible(bool visible)
{
    m_visible = visible;
}

QVariantMap IDOSWellLogChannel::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("name"), QVariant::fromValue(m_name));
    data.insert(QStringLiteral("unit"), QVariant::fromValue(m_unit));
    data.insert(QStringLiteral("depths"), QVariant::fromValue(m_depths));
    data.insert(QStringLiteral("values"), QVariant::fromValue(m_values));
    data.insert(QStringLiteral("visible"), QVariant::fromValue(m_visible));
    return data;
}

bool IDOSWellLogChannel::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("name")) || data.value(QStringLiteral("name")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("name"));
        return false;
    }
    if (!data.contains(QStringLiteral("unit")) || data.value(QStringLiteral("unit")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("unit"));
        return false;
    }
    if (!data.contains(QStringLiteral("depths")) || data.value(QStringLiteral("depths")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("depths"));
        return false;
    }
    if (!data.contains(QStringLiteral("values")) || data.value(QStringLiteral("values")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("values"));
        return false;
    }
    if (!data.contains(QStringLiteral("visible")) || data.value(QStringLiteral("visible")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("visible"));
        return false;
    }
    m_name = data.value(QStringLiteral("name")).value<QString>();
    m_unit = data.value(QStringLiteral("unit")).value<QString>();
    m_depths = data.value(QStringLiteral("depths")).value<QVector<double>>();
    m_values = data.value(QStringLiteral("values")).value<QVector<double>>();
    m_visible = data.value(QStringLiteral("visible")).value<bool>();
    return true;
}
