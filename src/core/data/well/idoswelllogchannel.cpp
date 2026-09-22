#include "idoswelllogchannel.h"

IDOSWellLogChannel::IDOSWellLogChannel()
    : m_visible(true)
{
}

IDOSWellLogChannel::IDOSWellLogChannel(const QString& name)
    : m_name(name)
    , m_visible(true)
{
}

QString IDOSWellLogChannel::name() const { return m_name; }
void IDOSWellLogChannel::setName(const QString& name) { m_name = name; }

QString IDOSWellLogChannel::unit() const { return m_unit; }
void IDOSWellLogChannel::setUnit(const QString& unit) { m_unit = unit; }

QVector<double> IDOSWellLogChannel::depths() const { return m_depths; }
QVector<double> IDOSWellLogChannel::values() const { return m_values; }

void IDOSWellLogChannel::setData(const QVector<double>& depths, const QVector<double>& values)
{
    m_depths = depths;
    m_values = values;
}

int IDOSWellLogChannel::sampleCount() const { return m_values.size(); }

bool IDOSWellLogChannel::isVisible() const { return m_visible; }
void IDOSWellLogChannel::setVisible(bool visible) { m_visible = visible; }
