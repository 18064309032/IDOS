#include "idoswellcontrol.h"

IDOSWellControl::IDOSWellControl()
    : m_wellIndex(0.0)
    , m_skinFactor(0.0)
    , m_dFactor(0.0)
    , m_active(false)
{
}

double IDOSWellControl::wellIndex() const { return m_wellIndex; }
void IDOSWellControl::setWellIndex(double wi) { m_wellIndex = wi; }

double IDOSWellControl::skinFactor() const { return m_skinFactor; }
void IDOSWellControl::setSkinFactor(double skin) { m_skinFactor = skin; }

double IDOSWellControl::dFactor() const { return m_dFactor; }
void IDOSWellControl::setDFactor(double d) { m_dFactor = d; }

bool IDOSWellControl::isActive() const { return m_active; }
void IDOSWellControl::setActive(bool active) { m_active = active; }
