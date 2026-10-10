#include "idoswellrenderobject.h"

IDOSWellRenderObject::IDOSWellRenderObject(IDOSRenderProvider* provider)
    :
    IDOSRenderObject(provider)
    , m_wellHeadPosition()
    , m_hasWellHead(false)
    , m_injector(false)
{
}

IDOSWellRenderObject::~IDOSWellRenderObject()
{
}

bool IDOSWellRenderObject::hasWellHead() const
{
    return m_hasWellHead;
}

QVector3D IDOSWellRenderObject::wellHeadPosition() const
{
    return m_wellHeadPosition;
}

void IDOSWellRenderObject::setWellHeadPosition(const QVector3D& position)
{
    m_wellHeadPosition = position;
    m_hasWellHead = true;
}

bool IDOSWellRenderObject::isInjector() const
{
    return m_injector;
}

void IDOSWellRenderObject::setInjector(bool injector)
{
    m_injector = injector;
}

const QVector<QVector3D>& IDOSWellRenderObject::points() const
{
    return m_points;
}

void IDOSWellRenderObject::setPoints(const QVector<QVector3D>& points)
{
    m_points = points;
}

void IDOSWellRenderObject::appendPoint(const QVector3D& point)
{
    m_points.append(point);
}

int IDOSWellRenderObject::pointCount() const
{
    return m_points.size();
}

bool IDOSWellRenderObject::isEmpty() const
{
    return !m_hasWellHead && m_points.isEmpty();
}

void IDOSWellRenderObject::clear()
{
    m_wellHeadPosition = QVector3D();
    m_hasWellHead = false;
    m_points.clear();
}
