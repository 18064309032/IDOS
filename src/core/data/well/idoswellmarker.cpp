#include "idoswellmarker.h"

IDOSWellMarker::IDOSWellMarker()
    : m_depth(0.0)
{
}

IDOSWellMarker::IDOSWellMarker(const QString& name, double depth)
    : m_name(name)
    , m_depth(depth)
{
}

QString IDOSWellMarker::name() const { return m_name; }
double IDOSWellMarker::depth() const { return m_depth; }
void IDOSWellMarker::setDepth(double depth) { m_depth = depth; }
