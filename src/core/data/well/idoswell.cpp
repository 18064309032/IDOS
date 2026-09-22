#include "idoswell.h"
#include "idoswellpath.h"
#include "idoswelllogset.h"
#include "idoswellmarkerset.h"

IDOSWell::IDOSWell(QObject* parent)
    : IDOSDataObject(parent)
    , m_type(Type::Producer)
    , m_i(0)
    , m_j(0)
    , m_k(0)
    , m_referenceDepth(0.0)
    , m_hasWellHead(false)
    , m_hasPath(false)
    , m_hasLogs(false)
    , m_hasMarkers(false)
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
    m_hasWellHead = true;
    emit wellHeadChanged(m_wellHead);
    emit dataChanged();
}

const IDOSWellPath& IDOSWell::path() const { return m_path; }
void IDOSWell::setPath(const IDOSWellPath& path)
{
    m_path = path;
    m_hasPath = true;
    emit pathChanged(m_path);
    emit dataChanged();
}

const IDOSWellLogSet& IDOSWell::logs() const { return m_logs; }
void IDOSWell::setLogs(const IDOSWellLogSet& logs)
{
    m_logs = logs;
    m_hasLogs = true;
    emit logsChanged(m_logs);
    emit dataChanged();
}

const IDOSWellMarkerSet& IDOSWell::markers() const { return m_markers; }
void IDOSWell::setMarkers(const IDOSWellMarkerSet& markers)
{
    m_markers = markers;
    m_hasMarkers = true;
    emit markersChanged(m_markers);
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

bool IDOSWell::hasWellHead() const { return m_hasWellHead; }
bool IDOSWell::hasPath() const { return m_hasPath; }
bool IDOSWell::hasLogs() const { return m_hasLogs; }
bool IDOSWell::hasMarkers() const { return m_hasMarkers; }

void IDOSWell::mergeFrom(const IDOSDataObject* other)
{
    if (other == nullptr || other->typeId() != typeId()) return;   // 类型不符 no-op
    const IDOSWell* w = static_cast<const IDOSWell*>(other);

    // header / path：整字段覆盖
    if (w->hasWellHead()) setWellHead(w->wellHead());
    if (w->hasPath())     setPath(w->path());

    // logs：按 channel 名覆盖（同名替换、新名追加）
    if (w->hasLogs())
    {
        IDOSWellLogSet merged = m_logs;   // 值类型拷贝
        const QList<IDOSWellLogChannel> chans = w->logs().channels();
        for (const IDOSWellLogChannel& ch : chans)
            merged.addChannel(ch);
        setLogs(merged);
    }

    // tops：按 horizon 名覆盖（同名覆盖深度、新名追加）
    if (w->hasMarkers())
    {
        IDOSWellMarkerSet merged = m_markers;
        const QList<IDOSWellMarker> ms = w->markers().markers();
        for (const IDOSWellMarker& m : ms)
            merged.addMarker(m);
        setMarkers(merged);
    }
}
