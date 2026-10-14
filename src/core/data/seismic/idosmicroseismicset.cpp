#include <QObject>

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

bool IDOSMicroseismicSet::canClone() const
{
    return true;
}

IDOSDataObject* IDOSMicroseismicSet::clone() const
{
    IDOSMicroseismicSet* copy = new IDOSMicroseismicSet();
    copy->setName(name());
    copy->setVisible(isVisible());
    copy->m_sourceFile = m_sourceFile;
    copy->m_events = m_events;
    return copy;
}

QVariantMap IDOSMicroseismicSet::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("events"), QVariant::fromValue(m_events));
    return data;
}

bool IDOSMicroseismicSet::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceFile"));
        return false;
    }
    if (!data.contains(QStringLiteral("events")) || data.value(QStringLiteral("events")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("events"));
        return false;
    }
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_events = data.value(QStringLiteral("events")).value<QVariantList>();
    return true;
}
