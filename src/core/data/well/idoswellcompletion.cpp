#include <QObject>

#include "idoswellcompletion.h"

IDOSWellCompletion::IDOSWellCompletion()
    :
    m_k(0)
    , m_open(true)
    , m_bottom(0.0)
    , m_top(0.0)
    , m_eventDate()
    , m_attributes()
{
}

IDOSWellCompletion::IDOSWellCompletion(int k, bool open, double bottom, double top)
    :
    m_k(k)
    , m_open(open)
    , m_bottom(bottom)
    , m_top(top)
    , m_eventDate()
    , m_attributes()
{
}

int IDOSWellCompletion::k() const
{
    return m_k;
}

QString IDOSWellCompletion::eventDate() const
{
    return m_eventDate;
}

void IDOSWellCompletion::setEventDate(const QString& date)
{
    m_eventDate = date;
}

QVariantMap IDOSWellCompletion::attributes() const
{
    return m_attributes;
}

void IDOSWellCompletion::setAttributes(const QVariantMap& attributes)
{
    m_attributes = attributes;
}

void IDOSWellCompletion::setK(int k)
{
    m_k = k;
}

bool IDOSWellCompletion::isOpen() const
{
    return m_open;
}

void IDOSWellCompletion::setOpen(bool open)
{
    m_open = open;
}

double IDOSWellCompletion::bottom() const
{
    return m_bottom;
}

void IDOSWellCompletion::setBottom(double bottom)
{
    m_bottom = bottom;
}

double IDOSWellCompletion::top() const
{
    return m_top;
}

void IDOSWellCompletion::setTop(double top)
{
    m_top = top;
}

QVariantMap IDOSWellCompletion::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("k"), QVariant::fromValue(m_k));
    data.insert(QStringLiteral("open"), QVariant::fromValue(m_open));
    data.insert(QStringLiteral("bottom"), QVariant::fromValue(m_bottom));
    data.insert(QStringLiteral("top"), QVariant::fromValue(m_top));
    data.insert(QStringLiteral("eventDate"), QVariant::fromValue(m_eventDate));
    data.insert(QStringLiteral("attributes"), QVariant::fromValue(m_attributes));
    return data;
}

bool IDOSWellCompletion::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("k")) || data.value(QStringLiteral("k")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("k"));
        return false;
    }
    if (!data.contains(QStringLiteral("open")) || data.value(QStringLiteral("open")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("open"));
        return false;
    }
    if (!data.contains(QStringLiteral("bottom")) || data.value(QStringLiteral("bottom")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("bottom"));
        return false;
    }
    if (!data.contains(QStringLiteral("top")) || data.value(QStringLiteral("top")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("top"));
        return false;
    }
    if (!data.contains(QStringLiteral("eventDate")) || data.value(QStringLiteral("eventDate")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("eventDate"));
        return false;
    }
    if (!data.contains(QStringLiteral("attributes")) || data.value(QStringLiteral("attributes")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("attributes"));
        return false;
    }
    m_k = data.value(QStringLiteral("k")).value<int>();
    m_open = data.value(QStringLiteral("open")).value<bool>();
    m_bottom = data.value(QStringLiteral("bottom")).value<double>();
    m_top = data.value(QStringLiteral("top")).value<double>();
    m_eventDate = data.value(QStringLiteral("eventDate")).value<QString>();
    m_attributes = data.value(QStringLiteral("attributes")).value<QVariantMap>();
    return true;
}
