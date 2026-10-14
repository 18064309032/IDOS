#include <QObject>

#include "idoswellmarker.h"

IDOSWellMarker::IDOSWellMarker()
    :
    m_depth(0.0)
{
}

IDOSWellMarker::IDOSWellMarker(const QString& name, double depth)
    :
    m_name(name)
    , m_depth(depth)
{
}

QString IDOSWellMarker::name() const
{
    return m_name;
}
double IDOSWellMarker::depth() const
{
    return m_depth;
}
void IDOSWellMarker::setDepth(double depth)
{
    m_depth = depth;
}

QVariantMap IDOSWellMarker::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("name"), QVariant::fromValue(m_name));
    data.insert(QStringLiteral("depth"), QVariant::fromValue(m_depth));
    return data;
}

bool IDOSWellMarker::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("name")) || data.value(QStringLiteral("name")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("name"));
        return false;
    }
    if (!data.contains(QStringLiteral("depth")) || data.value(QStringLiteral("depth")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("depth"));
        return false;
    }
    m_name = data.value(QStringLiteral("name")).value<QString>();
    m_depth = data.value(QStringLiteral("depth")).value<double>();
    return true;
}
