#include "idoswell.h"

IDOSWell::IDOSWell(QObject* parent)
    : IDOSDataObject(parent)
    , m_type(Type::Producer)
    , m_i(0)
    , m_j(0)
    , m_k(0)
    , m_referenceDepth(0.0)
    , m_open(true)
{
}

IDOSWell::~IDOSWell() = default;

IDOSWell::Type IDOSWell::type() const { return m_type; }
void IDOSWell::setType(Type type)
{
    if (m_type == type) return;
    m_type = type;
    emit typeChanged(m_type);
    emit dataChanged();
}

int IDOSWell::i() const { return m_i; }
void IDOSWell::setI(int i)
{
    if (m_i == i) return;
    m_i = i;
    emit gridLocationChanged(m_i, m_j, m_k);
    emit dataChanged();
}

int IDOSWell::j() const { return m_j; }
void IDOSWell::setJ(int j)
{
    if (m_j == j) return;
    m_j = j;
    emit gridLocationChanged(m_i, m_j, m_k);
    emit dataChanged();
}

int IDOSWell::k() const { return m_k; }
void IDOSWell::setK(int k)
{
    if (m_k == k) return;
    m_k = k;
    emit gridLocationChanged(m_i, m_j, m_k);
    emit dataChanged();
}

double IDOSWell::referenceDepth() const { return m_referenceDepth; }
void IDOSWell::setReferenceDepth(double depth)
{
    if (qFuzzyCompare(m_referenceDepth, depth)) return;
    m_referenceDepth = depth;
    emit referenceDepthChanged(m_referenceDepth);
    emit dataChanged();
}

QList<IDOSWellCompletion> IDOSWell::completions() const { return m_completions; }
void IDOSWell::setCompletions(const QList<IDOSWellCompletion>& completions)
{
    m_completions = completions;
    emit completionsChanged(m_completions);
    emit dataChanged();
}
void IDOSWell::addCompletion(const IDOSWellCompletion& completion)
{
    m_completions.append(completion);
    emit completionsChanged(m_completions);
    emit dataChanged();
}

QList<IDOSWellSegment> IDOSWell::segments() const { return m_segments; }
void IDOSWell::setSegments(const QList<IDOSWellSegment>& segments)
{
    m_segments = segments;
    emit segmentsChanged(m_segments);
    emit dataChanged();
}
void IDOSWell::addSegment(const IDOSWellSegment& segment)
{
    m_segments.append(segment);
    emit segmentsChanged(m_segments);
    emit dataChanged();
}

IDOSWellControl IDOSWell::wellControl() const { return m_wellControl; }
void IDOSWell::setWellControl(const IDOSWellControl& control)
{
    m_wellControl = control;
    emit wellControlChanged(m_wellControl);
    emit dataChanged();
}

IDOSWellHead IDOSWell::wellHead() const { return m_wellHead; }
void IDOSWell::setWellHead(const IDOSWellHead& head)
{
    m_wellHead = head;
    emit wellHeadChanged(m_wellHead);
    emit dataChanged();
}

bool IDOSWell::isOpen() const { return m_open; }
void IDOSWell::setOpen(bool open)
{
    if (m_open == open) return;
    m_open = open;
    emit openStatusChanged(m_open);
    emit dataChanged();
}
