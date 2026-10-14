#include <cmath>

#include <QObject>

#include "idoswellcheckshot.h"

IDOSWellCheckshot::IDOSWellCheckshot()
    :
    m_depths()
    , m_times()
    , m_depthUnit()
    , m_timeUnit()
    , m_timeType()
    , m_depthType()
{
}

bool IDOSWellCheckshot::setSamples(const QVector<double>& depths, const QVector<double>& times)
{
    if (depths.size() < 2 || depths.size() != times.size())
    {
        return false;
    }
    const bool ascendingDepth = depths.at(1) > depths.at(0);
    const bool ascendingTime = times.at(1) > times.at(0);
    for (int index = 0; index < depths.size(); ++index)
    {
        if (!std::isfinite(depths.at(index)) || !std::isfinite(times.at(index)))
        {
            return false;
        }
        if (index > 0 && ((ascendingDepth ? depths.at(index) <= depths.at(index - 1)
                                        : depths.at(index) >= depths.at(index - 1)) ||
                          (ascendingTime ? times.at(index) <= times.at(index - 1)
                                         : times.at(index) >= times.at(index - 1))))
        {
            return false;
        }
    }
    m_depths = depths;
    m_times = times;
    return true;
}

const QVector<double>& IDOSWellCheckshot::depths() const
{
    return m_depths;
}

const QVector<double>& IDOSWellCheckshot::times() const
{
    return m_times;
}

QString IDOSWellCheckshot::depthUnit() const
{
    return m_depthUnit;
}

QString IDOSWellCheckshot::timeUnit() const
{
    return m_timeUnit;
}

QString IDOSWellCheckshot::timeType() const
{
    return m_timeType;
}

void IDOSWellCheckshot::setUnits(const QString& depthUnit, const QString& timeUnit)
{
    m_depthUnit = depthUnit;
    m_timeUnit = timeUnit;
}

void IDOSWellCheckshot::setTimeType(const QString& timeType)
{
    m_timeType = timeType;
}

QString IDOSWellCheckshot::depthType() const
{
    return m_depthType;
}

void IDOSWellCheckshot::setDepthType(const QString& depthType)
{
    m_depthType = depthType;
}

QVariantMap IDOSWellCheckshot::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("depths"), QVariant::fromValue(m_depths));
    data.insert(QStringLiteral("times"), QVariant::fromValue(m_times));
    data.insert(QStringLiteral("depthUnit"), QVariant::fromValue(m_depthUnit));
    data.insert(QStringLiteral("timeUnit"), QVariant::fromValue(m_timeUnit));
    data.insert(QStringLiteral("timeType"), QVariant::fromValue(m_timeType));
    data.insert(QStringLiteral("depthType"), QVariant::fromValue(m_depthType));
    return data;
}

bool IDOSWellCheckshot::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("depths")) || data.value(QStringLiteral("depths")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("depths"));
        return false;
    }
    if (!data.contains(QStringLiteral("times")) || data.value(QStringLiteral("times")).userType() != qMetaTypeId<QVector<double>>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("times"));
        return false;
    }
    if (!data.contains(QStringLiteral("depthUnit")) || data.value(QStringLiteral("depthUnit")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("depthUnit"));
        return false;
    }
    if (!data.contains(QStringLiteral("timeUnit")) || data.value(QStringLiteral("timeUnit")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("timeUnit"));
        return false;
    }
    if (!data.contains(QStringLiteral("timeType")) || data.value(QStringLiteral("timeType")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("timeType"));
        return false;
    }
    if (!data.contains(QStringLiteral("depthType")) || data.value(QStringLiteral("depthType")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("depthType"));
        return false;
    }
    m_depths = data.value(QStringLiteral("depths")).value<QVector<double>>();
    m_times = data.value(QStringLiteral("times")).value<QVector<double>>();
    m_depthUnit = data.value(QStringLiteral("depthUnit")).value<QString>();
    m_timeUnit = data.value(QStringLiteral("timeUnit")).value<QString>();
    m_timeType = data.value(QStringLiteral("timeType")).value<QString>();
    m_depthType = data.value(QStringLiteral("depthType")).value<QString>();
    return true;
}
