#include <cmath>

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
