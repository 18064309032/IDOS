#include <QObject>

#include "idoswelllogset.h"

IDOSWellLogSet::IDOSWellLogSet()
{
}

QList<IDOSWellLogChannel> IDOSWellLogSet::channels() const
{
    return m_channels;
}

const IDOSWellLogChannel* IDOSWellLogSet::channel(const QString& name) const
{
    for (const IDOSWellLogChannel& c : m_channels)
    {
        if (c.name() == name)
        {
            return &c;
        }
    }
    return nullptr;
}

IDOSWellLogChannel* IDOSWellLogSet::channel(const QString& name)
{
    for (IDOSWellLogChannel& c : m_channels)
    {
        if (c.name() == name)
        {
            return &c;
        }
    }
    return nullptr;
}

void IDOSWellLogSet::addChannel(const IDOSWellLogChannel& channel)
{
    for (int i = 0; i < m_channels.size(); ++i)
    {
        if (m_channels[i].name() == channel.name())
        {
            m_channels[i] = channel; // 同名替换
            return;
        }
    }
    m_channels.append(channel);
}

int IDOSWellLogSet::channelCount() const
{
    return m_channels.size();
}

bool IDOSWellLogSet::isEmpty() const
{
    return m_channels.isEmpty();
}

void IDOSWellLogSet::clear()
{
    m_channels.clear();
}

QVariantMap IDOSWellLogSet::projectData() const
{
    QVariantMap data;
    QVariantList channels;
    for (const IDOSWellLogChannel& value : m_channels)
    {
        channels.append(value.projectData());
    }
    data.insert(QStringLiteral("channels"), channels);
    return data;
}

bool IDOSWellLogSet::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("channels")) || data.value(QStringLiteral("channels")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("channels"));
        return false;
    }
    m_channels.clear();
    for (const QVariant& entry : data.value(QStringLiteral("channels")).toList())
    {
        IDOSWellLogChannel value;
        if (!value.restoreProjectData(entry.toMap(), error))
        {
            return false;
        }
        m_channels.append(value);
    }
    return true;
}
