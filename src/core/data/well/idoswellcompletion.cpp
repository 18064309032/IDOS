#include "idoswellcompletion.h"

IDOSWellCompletion::IDOSWellCompletion()
    :
    m_k(0)
    , m_open(true)
    , m_bottom(0.0)
    , m_top(0.0)
    , m_eventDate()
{
}

IDOSWellCompletion::IDOSWellCompletion(int k, bool open, double bottom, double top)
    :
    m_k(k)
    , m_open(open)
    , m_bottom(bottom)
    , m_top(top)
    , m_eventDate()
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
