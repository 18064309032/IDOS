#include <QObject>

#include "idosmicroseismicsetobject.h"

IDOSMicroseismicSetObject::IDOSMicroseismicSetObject(QObject* parent)
    :
    IDOSDataObject(parent)
{
}

QString IDOSMicroseismicSetObject::typeId() const
{
    return QStringLiteral("idos.microseismic.set");
}

QString IDOSMicroseismicSetObject::sourceFile() const
{
    return m_sourceFile;
}

void IDOSMicroseismicSetObject::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QVariantList IDOSMicroseismicSetObject::events() const
{
    return m_events;
}

void IDOSMicroseismicSetObject::setEvents(const QVariantList& events)
{
    if (m_events == events)
    {
        return;
    }
    m_events = events;
    emit dataChanged();
}

bool IDOSMicroseismicSetObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSMicroseismicSetObject::clone() const
{
    IDOSMicroseismicSetObject* copy = new IDOSMicroseismicSetObject();
    copy->setName(name());
    copy->m_sourceFile = m_sourceFile;
    copy->m_events = m_events;
    return copy;
}

QVariantMap IDOSMicroseismicSetObject::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("events"), QVariant::fromValue(m_events));
    return data;
}

bool IDOSMicroseismicSetObject::restoreProjectData(const QVariantMap& data, QString& error)
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
