#include "idosmicroseismicset.h"

IDOSMicroseismicSet::IDOSMicroseismicSet(QObject* parent)
    :
    IDOSDataObject(parent)
{
}

QString IDOSMicroseismicSet::typeId() const
{
    return QStringLiteral("idos.microseismic.set");
}

QString IDOSMicroseismicSet::sourceFile() const
{
    return m_sourceFile;
}

void IDOSMicroseismicSet::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QVariantList IDOSMicroseismicSet::events() const
{
    return m_events;
}

void IDOSMicroseismicSet::setEvents(const QVariantList& events)
{
    if (m_events == events)
    {
        return;
    }
    m_events = events;
    emit dataChanged();
}
