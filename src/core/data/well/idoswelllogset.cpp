#include "idoswelllogset.h"

IDOSWellLogSet::IDOSWellLogSet() = default;

QList<IDOSWellLogChannel> IDOSWellLogSet::channels() const { return m_channels; }

const IDOSWellLogChannel* IDOSWellLogSet::channel(const QString& name) const
{
    for (const IDOSWellLogChannel& c : m_channels)
    {
        if (c.name() == name) return &c;
    }
    return nullptr;
}

IDOSWellLogChannel* IDOSWellLogSet::channel(const QString& name)
{
    for (IDOSWellLogChannel& c : m_channels)
    {
        if (c.name() == name) return &c;
    }
    return nullptr;
}

void IDOSWellLogSet::addChannel(const IDOSWellLogChannel& channel)
{
    for (int i = 0; i < m_channels.size(); ++i)
    {
        if (m_channels[i].name() == channel.name())
        {
            m_channels[i] = channel;   // 同名替换
            return;
        }
    }
    m_channels.append(channel);
}

int IDOSWellLogSet::channelCount() const { return m_channels.size(); }

bool IDOSWellLogSet::isEmpty() const { return m_channels.isEmpty(); }

void IDOSWellLogSet::clear() { m_channels.clear(); }
