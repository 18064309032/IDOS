#include <QObject>

#include "idosproject.h"
#include "idoswelllogset.h"
#include "idoswellmarkerset.h"
#include "idoswellpath.h"

#include "idoswellobject.h"

IDOSWellObject::IDOSWellObject(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_type(Type::Producer)
    , m_i(0)
    , m_j(0)
    , m_k(0)
    , m_referenceDepth(0.0)
    , m_hasWellHead(false)
    , m_hasPath(false)
    , m_hasLogs(false)
    , m_hasMarkers(false)
    , m_hasCheckshot(false)
    , m_hasCompletions(false)
    , m_open(true)
{
}

IDOSWellObject::~IDOSWellObject()
{
}

IDOSWellObject::Type IDOSWellObject::type() const
{
    return m_type;
}
void IDOSWellObject::setType(Type type)
{
    if (m_type == type)
    {
        return;
    }
    m_type = type;
    emit typeChanged(m_type);
    emit dataChanged();
}

int IDOSWellObject::i() const
{
    return m_i;
}
void IDOSWellObject::setI(int i)
{
    if (m_i == i)
    {
        return;
    }
    m_i = i;
    emit gridLocationChanged(m_i, m_j, m_k);
    emit dataChanged();
}

int IDOSWellObject::j() const
{
    return m_j;
}
void IDOSWellObject::setJ(int j)
{
    if (m_j == j)
    {
        return;
    }
    m_j = j;
    emit gridLocationChanged(m_i, m_j, m_k);
    emit dataChanged();
}

int IDOSWellObject::k() const
{
    return m_k;
}
void IDOSWellObject::setK(int k)
{
    if (m_k == k)
    {
        return;
    }
    m_k = k;
    emit gridLocationChanged(m_i, m_j, m_k);
    emit dataChanged();
}

double IDOSWellObject::referenceDepth() const
{
    return m_referenceDepth;
}
void IDOSWellObject::setReferenceDepth(double depth)
{
    if (qFuzzyCompare(m_referenceDepth, depth))
    {
        return;
    }
    m_referenceDepth = depth;
    emit referenceDepthChanged(m_referenceDepth);
    emit dataChanged();
}

QList<IDOSWellCompletion> IDOSWellObject::completions() const
{
    return m_completions;
}
void IDOSWellObject::setCompletions(const QList<IDOSWellCompletion>& completions)
{
    m_completions = completions;
    m_hasCompletions = true;
    emit completionsChanged(m_completions);
    emit dataChanged();
}
void IDOSWellObject::addCompletion(const IDOSWellCompletion& completion)
{
    m_completions.append(completion);
    m_hasCompletions = true;
    emit completionsChanged(m_completions);
    emit dataChanged();
}

QList<IDOSWellSegment> IDOSWellObject::segments() const
{
    return m_segments;
}
void IDOSWellObject::setSegments(const QList<IDOSWellSegment>& segments)
{
    m_segments = segments;
    emit segmentsChanged(m_segments);
    emit dataChanged();
}
void IDOSWellObject::addSegment(const IDOSWellSegment& segment)
{
    m_segments.append(segment);
    emit segmentsChanged(m_segments);
    emit dataChanged();
}

IDOSWellControl IDOSWellObject::wellControl() const
{
    return m_wellControl;
}
void IDOSWellObject::setWellControl(const IDOSWellControl& control)
{
    m_wellControl = control;
    emit wellControlChanged(m_wellControl);
    emit dataChanged();
}

IDOSWellHead IDOSWellObject::wellHead() const
{
    return m_wellHead;
}
void IDOSWellObject::setWellHead(const IDOSWellHead& head)
{
    m_wellHead = head;
    m_hasWellHead = true;
    emit wellHeadChanged(m_wellHead);
    emit dataChanged();
}

const IDOSWellPath& IDOSWellObject::path() const
{
    return m_path;
}
void IDOSWellObject::setPath(const IDOSWellPath& path)
{
    m_path = path;
    m_hasPath = true;
    emit pathChanged(m_path);
    emit dataChanged();
}

const IDOSWellLogSet& IDOSWellObject::logs() const
{
    return m_logs;
}
void IDOSWellObject::setLogs(const IDOSWellLogSet& logs)
{
    m_logs = logs;
    m_hasLogs = true;
    emit logsChanged(m_logs);
    emit dataChanged();
}

const IDOSWellMarkerSet& IDOSWellObject::markers() const
{
    return m_markers;
}
void IDOSWellObject::setMarkers(const IDOSWellMarkerSet& markers)
{
    m_markers = markers;
    m_hasMarkers = true;
    emit markersChanged(m_markers);
    emit dataChanged();
}

bool IDOSWellObject::isOpen() const
{
    return m_open;
}
void IDOSWellObject::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }
    m_open = open;
    emit openStatusChanged(m_open);
    emit dataChanged();
}

bool IDOSWellObject::hasWellHead() const
{
    return m_hasWellHead;
}
bool IDOSWellObject::hasPath() const
{
    return m_hasPath;
}
bool IDOSWellObject::hasLogs() const
{
    return m_hasLogs;
}
bool IDOSWellObject::hasMarkers() const
{
    return m_hasMarkers;
}

const IDOSWellCheckshot& IDOSWellObject::checkshot() const
{
    return m_checkshot;
}

void IDOSWellObject::setCheckshot(const IDOSWellCheckshot& checkshot)
{
    m_checkshot = checkshot;
    m_hasCheckshot = true;
    emit dataChanged();
}

bool IDOSWellObject::hasCheckshot() const
{
    return m_hasCheckshot;
}

bool IDOSWellObject::hasCompletions() const
{
    return m_hasCompletions;
}

void IDOSWellObject::mergeFrom(const IDOSDataObject* other)
{
    if (other == nullptr || other->typeId() != typeId())
    {
        return; // 类型不符 no-op
    }
    const IDOSWellObject* w = static_cast<const IDOSWellObject*>(other);
    if (w->hasCheckshot())
    {
        setCheckshot(w->checkshot());
    }
    if (w->hasCompletions())
    {
        QList<IDOSWellCompletion> merged = m_completions;
        for (const IDOSWellCompletion& incoming : w->completions())
        {
            bool replaced = false;
            for (int index = 0; index < merged.size(); ++index)
            {
                if (merged.at(index).k() == incoming.k() && merged.at(index).top() == incoming.top() &&
                    merged.at(index).bottom() == incoming.bottom() && merged.at(index).eventDate() == incoming.eventDate())
                {
                    merged[index] = incoming;
                    replaced = true;
                    break;
                }
            }
            if (!replaced)
            {
                merged.append(incoming);
            }
        }
        setCompletions(merged);
    }

    // header / path：整字段覆盖
    if (w->hasWellHead())
    {
        setWellHead(w->wellHead());
    }
    if (w->hasPath())
    {
        setPath(w->path());
    }

    // logs：按 channel 名覆盖（同名替换、新名追加）
    if (w->hasLogs())
    {
        IDOSWellLogSet merged = m_logs; // 值类型拷贝
        const QList<IDOSWellLogChannel> chans = w->logs().channels();
        for (const IDOSWellLogChannel& ch : chans)
        {
            merged.addChannel(ch);
        }
        setLogs(merged);
    }

    // tops：按 horizon 名覆盖（同名覆盖深度、新名追加）
    if (w->hasMarkers())
    {
        IDOSWellMarkerSet merged = m_markers;
        const QList<IDOSWellMarker> ms = w->markers().markers();
        for (const IDOSWellMarker& m : ms)
        {
            merged.addMarker(m);
        }
        setMarkers(merged);
    }
}

QString IDOSWellObject::typeId() const
{
    return QStringLiteral("idos.well");
}

void IDOSWellObject::copyInputData(const IDOSWellObject* source)
{
    if (source == nullptr || source == this)
    {
        return;
    }
    m_wellHead = source->m_wellHead;
    m_path = source->m_path;
    m_logs = source->m_logs;
    m_markers = source->m_markers;
    m_checkshot = source->m_checkshot;
    m_completions = source->m_completions;
    m_hasWellHead = source->m_hasWellHead;
    m_hasPath = source->m_hasPath;
    m_hasLogs = source->m_hasLogs;
    m_hasMarkers = source->m_hasMarkers;
    m_hasCheckshot = source->m_hasCheckshot;
    m_hasCompletions = source->m_hasCompletions;
    emit wellHeadChanged(m_wellHead);
    emit pathChanged(m_path);
    emit logsChanged(m_logs);
    emit markersChanged(m_markers);
    emit completionsChanged(m_completions);
    emit dataChanged();
}

bool IDOSWellObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSWellObject::clone() const
{
    IDOSWellObject* copy = new IDOSWellObject();
    copy->setName(name());
    copy->m_type = m_type;
    copy->m_i = m_i;
    copy->m_j = m_j;
    copy->m_k = m_k;
    copy->m_referenceDepth = m_referenceDepth;
    copy->m_completions = m_completions;
    copy->m_segments = m_segments;
    copy->m_wellControl = m_wellControl;
    copy->m_wellHead = m_wellHead;
    copy->m_path = m_path;
    copy->m_logs = m_logs;
    copy->m_markers = m_markers;
    copy->m_checkshot = m_checkshot;
    copy->m_hasWellHead = m_hasWellHead;
    copy->m_hasPath = m_hasPath;
    copy->m_hasLogs = m_hasLogs;
    copy->m_hasMarkers = m_hasMarkers;
    copy->m_hasCheckshot = m_hasCheckshot;
    copy->m_hasCompletions = m_hasCompletions;
    copy->m_open = m_open;
    return copy;
}

QVariantMap IDOSWellObject::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("type"), static_cast<int>(m_type));
    data.insert(QStringLiteral("i"), QVariant::fromValue(m_i));
    data.insert(QStringLiteral("j"), QVariant::fromValue(m_j));
    data.insert(QStringLiteral("k"), QVariant::fromValue(m_k));
    data.insert(QStringLiteral("referenceDepth"), QVariant::fromValue(m_referenceDepth));
    QVariantList completions;
    for (const IDOSWellCompletion& value : m_completions)
    {
        completions.append(value.projectData());
    }
    data.insert(QStringLiteral("completions"), completions);
    QVariantList segments;
    for (const IDOSWellSegment& value : m_segments)
    {
        segments.append(value.projectData());
    }
    data.insert(QStringLiteral("segments"), segments);
    data.insert(QStringLiteral("wellControl"), m_wellControl.projectData());
    data.insert(QStringLiteral("wellHead"), m_wellHead.projectData());
    data.insert(QStringLiteral("path"), m_path.projectData());
    data.insert(QStringLiteral("logs"), m_logs.projectData());
    data.insert(QStringLiteral("markers"), m_markers.projectData());
    data.insert(QStringLiteral("checkshot"), m_checkshot.projectData());
    data.insert(QStringLiteral("hasWellHead"), QVariant::fromValue(m_hasWellHead));
    data.insert(QStringLiteral("hasPath"), QVariant::fromValue(m_hasPath));
    data.insert(QStringLiteral("hasLogs"), QVariant::fromValue(m_hasLogs));
    data.insert(QStringLiteral("hasMarkers"), QVariant::fromValue(m_hasMarkers));
    data.insert(QStringLiteral("hasCheckshot"), QVariant::fromValue(m_hasCheckshot));
    data.insert(QStringLiteral("hasCompletions"), QVariant::fromValue(m_hasCompletions));
    data.insert(QStringLiteral("open"), QVariant::fromValue(m_open));
    return data;
}

bool IDOSWellObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("type")) || data.value(QStringLiteral("type")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("type"));
        return false;
    }
    if (data.value(QStringLiteral("type")).toInt() < 0 || data.value(QStringLiteral("type")).toInt() >= 2)
    {
        error = QObject::tr("A stored enumeration value is invalid.");
        return false;
    }
    if (!data.contains(QStringLiteral("i")) || data.value(QStringLiteral("i")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("i"));
        return false;
    }
    if (!data.contains(QStringLiteral("j")) || data.value(QStringLiteral("j")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("j"));
        return false;
    }
    if (!data.contains(QStringLiteral("k")) || data.value(QStringLiteral("k")).userType() != qMetaTypeId<int>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("k"));
        return false;
    }
    if (!data.contains(QStringLiteral("referenceDepth")) || data.value(QStringLiteral("referenceDepth")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("referenceDepth"));
        return false;
    }
    if (!data.contains(QStringLiteral("completions")) || data.value(QStringLiteral("completions")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("completions"));
        return false;
    }
    if (!data.contains(QStringLiteral("segments")) || data.value(QStringLiteral("segments")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("segments"));
        return false;
    }
    if (!data.contains(QStringLiteral("wellControl")) || data.value(QStringLiteral("wellControl")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("wellControl"));
        return false;
    }
    if (!data.contains(QStringLiteral("wellHead")) || data.value(QStringLiteral("wellHead")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("wellHead"));
        return false;
    }
    if (!data.contains(QStringLiteral("path")) || data.value(QStringLiteral("path")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("path"));
        return false;
    }
    if (!data.contains(QStringLiteral("logs")) || data.value(QStringLiteral("logs")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("logs"));
        return false;
    }
    if (!data.contains(QStringLiteral("markers")) || data.value(QStringLiteral("markers")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("markers"));
        return false;
    }
    if (!data.contains(QStringLiteral("checkshot")) || data.value(QStringLiteral("checkshot")).userType() != qMetaTypeId<QVariantMap>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("checkshot"));
        return false;
    }
    if (!data.contains(QStringLiteral("hasWellHead")) || data.value(QStringLiteral("hasWellHead")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("hasWellHead"));
        return false;
    }
    if (!data.contains(QStringLiteral("hasPath")) || data.value(QStringLiteral("hasPath")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("hasPath"));
        return false;
    }
    if (!data.contains(QStringLiteral("hasLogs")) || data.value(QStringLiteral("hasLogs")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("hasLogs"));
        return false;
    }
    if (!data.contains(QStringLiteral("hasMarkers")) || data.value(QStringLiteral("hasMarkers")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("hasMarkers"));
        return false;
    }
    if (!data.contains(QStringLiteral("hasCheckshot")) || data.value(QStringLiteral("hasCheckshot")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("hasCheckshot"));
        return false;
    }
    if (!data.contains(QStringLiteral("hasCompletions")) || data.value(QStringLiteral("hasCompletions")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("hasCompletions"));
        return false;
    }
    if (!data.contains(QStringLiteral("open")) || data.value(QStringLiteral("open")).userType() != qMetaTypeId<bool>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("open"));
        return false;
    }
    m_type = static_cast<Type>(data.value(QStringLiteral("type")).toInt());
    m_i = data.value(QStringLiteral("i")).value<int>();
    m_j = data.value(QStringLiteral("j")).value<int>();
    m_k = data.value(QStringLiteral("k")).value<int>();
    m_referenceDepth = data.value(QStringLiteral("referenceDepth")).value<double>();
    m_completions.clear();
    for (const QVariant& entry : data.value(QStringLiteral("completions")).toList())
    {
        IDOSWellCompletion value;
        if (!value.restoreProjectData(entry.toMap(), error))
        {
            return false;
        }
        m_completions.append(value);
    }
    m_segments.clear();
    for (const QVariant& entry : data.value(QStringLiteral("segments")).toList())
    {
        IDOSWellSegment value;
        if (!value.restoreProjectData(entry.toMap(), error))
        {
            return false;
        }
        m_segments.append(value);
    }
    if (!m_wellControl.restoreProjectData(data.value(QStringLiteral("wellControl")).toMap(), error))
    {
        return false;
    }
    if (!m_wellHead.restoreProjectData(data.value(QStringLiteral("wellHead")).toMap(), error))
    {
        return false;
    }
    if (!m_path.restoreProjectData(data.value(QStringLiteral("path")).toMap(), error))
    {
        return false;
    }
    if (!m_logs.restoreProjectData(data.value(QStringLiteral("logs")).toMap(), error))
    {
        return false;
    }
    if (!m_markers.restoreProjectData(data.value(QStringLiteral("markers")).toMap(), error))
    {
        return false;
    }
    if (!m_checkshot.restoreProjectData(data.value(QStringLiteral("checkshot")).toMap(), error))
    {
        return false;
    }
    m_hasWellHead = data.value(QStringLiteral("hasWellHead")).value<bool>();
    m_hasPath = data.value(QStringLiteral("hasPath")).value<bool>();
    m_hasLogs = data.value(QStringLiteral("hasLogs")).value<bool>();
    m_hasMarkers = data.value(QStringLiteral("hasMarkers")).value<bool>();
    m_hasCheckshot = data.value(QStringLiteral("hasCheckshot")).value<bool>();
    m_hasCompletions = data.value(QStringLiteral("hasCompletions")).value<bool>();
    m_open = data.value(QStringLiteral("open")).value<bool>();
    return true;
}
bool IDOSWellObject::validateProjectReferences(const IDOSProject* project, QString& error) const
{
    Q_UNUSED(project);
    if (m_checkshot.depths().size() != m_checkshot.times().size())
    {
        error = tr("Stored well checkshot arrays have inconsistent lengths.");
        return false;
    }
    for (const IDOSWellLogChannel& channel : m_logs.channels())
    {
        if (channel.depths().size() != channel.values().size())
        {
            error = tr("Stored well log arrays have inconsistent lengths.");
            return false;
        }
    }
    return true;
}
