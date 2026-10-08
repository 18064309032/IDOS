#include <QFormLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QtGlobal>

#include "idoswell.h"
#include "idoswelllogchannel.h"
#include "idoswelllogset.h"
#include "idoswellpath.h"

#include "idospropertywidget.h"

IDOSPropertyWidget::IDOSPropertyWidget(QWidget* parent)
    : QWidget(parent)
    , m_well(nullptr)
    , m_formLayout(new QFormLayout(this))
{
    m_formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    m_formLayout->setContentsMargins(8, 8, 8, 8);
    refresh();
}

void IDOSPropertyWidget::setWell(IDOSWell* well)
{
    if (m_well == well)
    {
        refresh();
        return;
    }

    if (m_well != nullptr)
    {
        disconnect(m_well, nullptr, this, nullptr);
    }

    m_well = well;
    if (m_well != nullptr)
    {
        connect(m_well, &QObject::destroyed,
                this, &IDOSPropertyWidget::onWellDestroyed);
        connect(m_well, &IDOSObject::nameChanged,
                this, &IDOSPropertyWidget::onWellChanged);
        connect(m_well, &IDOSWell::typeChanged,
                this, &IDOSPropertyWidget::onWellChanged);
        connect(m_well, &IDOSWell::gridLocationChanged,
                this, &IDOSPropertyWidget::onWellChanged);
        connect(m_well, &IDOSWell::referenceDepthChanged,
                this, &IDOSPropertyWidget::onWellChanged);
        connect(m_well, &IDOSWell::wellHeadChanged,
                this, &IDOSPropertyWidget::onWellHeadChanged);
        connect(m_well, &IDOSWell::pathChanged,
                this, &IDOSPropertyWidget::onWellPathChanged);
        connect(m_well, &IDOSWell::logsChanged,
                this, &IDOSPropertyWidget::onWellLogsChanged);
    }
    refresh();
}

void IDOSPropertyWidget::setWellPart(const QString& partKey, const QString& itemKey)
{
    if (m_partKey == partKey && m_itemKey == itemKey)
    {
        return;
    }

    m_partKey = partKey;
    m_itemKey = itemKey;
    refresh();
}

void IDOSPropertyWidget::clear()
{
    setWell(nullptr);
    m_partKey.clear();
    m_itemKey.clear();
    refresh();
}

void IDOSPropertyWidget::onWellDestroyed()
{
    m_well = nullptr;
    m_partKey.clear();
    m_itemKey.clear();
    refresh();
}

void IDOSPropertyWidget::onWellHeadChanged(const IDOSWellHead& head)
{
    Q_UNUSED(head)
    refresh();
}

void IDOSPropertyWidget::onWellPathChanged(const IDOSWellPath& path)
{
    Q_UNUSED(path)
    refresh();
}

void IDOSPropertyWidget::onWellLogsChanged(const IDOSWellLogSet& logs)
{
    Q_UNUSED(logs)
    refresh();
}

void IDOSPropertyWidget::onWellChanged()
{
    refresh();
}

void IDOSPropertyWidget::addProperty(const QString& name, const QString& value)
{
    QLabel* valueLabel = new QLabel(value, this);
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueLabel->setWordWrap(true);
    m_formLayout->addRow(name, valueLabel);
}

void IDOSPropertyWidget::clearProperties()
{
    QLayoutItem* item = m_formLayout->takeAt(0);
    while (item != nullptr)
    {
        delete item->widget();
        delete item;
        item = m_formLayout->takeAt(0);
    }
}

void IDOSPropertyWidget::refresh()
{
    clearProperties();
    if (m_well == nullptr)
    {
        addProperty(tr("Selection"), tr("No object selected"));
        return;
    }

    if (m_partKey == QStringLiteral("idos.well.trajectory"))
    {
        showTrajectoryProperties();
        return;
    }
    if (m_partKey == QStringLiteral("idos.well.logs"))
    {
        showLogProperties();
        return;
    }

    showWellProperties();
}

void IDOSPropertyWidget::showWellProperties()
{
    addProperty(tr("Object Type"), tr("Well"));
    addProperty(tr("Name"), m_well->name());
    addProperty(tr("Well Type"), wellTypeText());
    addProperty(tr("Grid Location"),
                QStringLiteral("(%1, %2, %3)").arg(m_well->i()).arg(m_well->j()).arg(m_well->k()));
    addProperty(tr("Reference Depth"), QString::number(m_well->referenceDepth(), 'f', 2));
    addProperty(tr("Trajectory"), m_well->hasPath()
                ? tr("%1 points").arg(m_well->path().pointCount())
                : tr("Not available"));
    addProperty(tr("Well Log Curves"), m_well->hasLogs()
                ? tr("%1 channels").arg(m_well->logs().channelCount())
                : tr("Not available"));
    if (!m_well->hasWellHead())
    {
        return;
    }

    const IDOSWellHead head = m_well->wellHead();
    addProperty(tr("Surface X"), QString::number(head.surfaceX(), 'f', 2));
    addProperty(tr("Surface Y"), QString::number(head.surfaceY(), 'f', 2));
    addProperty(tr("Surface Elevation"), QString::number(head.surfaceElevation(), 'f', 2));
    addProperty(tr("KB"), QString::number(head.kb(), 'f', 2));
}

void IDOSPropertyWidget::showTrajectoryProperties()
{
    addProperty(tr("Object Type"), tr("Well Trajectory"));
    addProperty(tr("Well"), m_well->name());
    addProperty(tr("Point Count"), QString::number(m_well->path().pointCount()));
}

void IDOSPropertyWidget::showLogProperties()
{
    if (m_itemKey.isEmpty())
    {
        addProperty(tr("Object Type"), tr("Well Log Curves"));
        addProperty(tr("Well"), m_well->name());
        addProperty(tr("Channel Count"), QString::number(m_well->logs().channelCount()));
        addProperty(tr("Depth Range"), depthRangeText());
        return;
    }

    const IDOSWellLogChannel* channel = m_well->logs().channel(m_itemKey);
    if (channel == nullptr)
    {
        addProperty(tr("Selection"), tr("The selected curve is no longer available."));
        return;
    }

    addProperty(tr("Object Type"), tr("Well Log Curve"));
    addProperty(tr("Well"), m_well->name());
    addProperty(tr("Name"), channel->name());
    addProperty(tr("Unit"), channel->unit());
    addProperty(tr("Sample Count"), QString::number(channel->sampleCount()));
    addProperty(tr("Depth Range"), channelDepthRangeText(*channel));
}

QString IDOSPropertyWidget::wellTypeText() const
{
    if (m_well->type() == IDOSWell::Type::Injector)
    {
        return tr("Injector");
    }

    return tr("Producer");
}

QString IDOSPropertyWidget::depthRangeText() const
{
    if (!m_well->hasLogs())
    {
        return tr("Not available");
    }

    const QList<IDOSWellLogChannel> channels = m_well->logs().channels();
    bool hasDepth = false;
    double minimumDepth = 0.0;
    double maximumDepth = 0.0;
    for (const IDOSWellLogChannel& channel : channels)
    {
        const QVector<double> depths = channel.depths();
        for (double depth : depths)
        {
            if (qIsNaN(depth))
            {
                continue;
            }
            if (!hasDepth)
            {
                minimumDepth = depth;
                maximumDepth = depth;
                hasDepth = true;
                continue;
            }
            minimumDepth = qMin(minimumDepth, depth);
            maximumDepth = qMax(maximumDepth, depth);
        }
    }

    if (!hasDepth)
    {
        return tr("Not available");
    }
    return QStringLiteral("%1 - %2")
        .arg(QString::number(minimumDepth, 'f', 2), QString::number(maximumDepth, 'f', 2));
}

QString IDOSPropertyWidget::channelDepthRangeText(const IDOSWellLogChannel& channel) const
{
    const QVector<double> depths = channel.depths();
    bool hasDepth = false;
    double minimumDepth = 0.0;
    double maximumDepth = 0.0;
    for (double depth : depths)
    {
        if (qIsNaN(depth))
        {
            continue;
        }
        if (!hasDepth)
        {
            minimumDepth = depth;
            maximumDepth = depth;
            hasDepth = true;
            continue;
        }
        minimumDepth = qMin(minimumDepth, depth);
        maximumDepth = qMax(maximumDepth, depth);
    }

    if (!hasDepth)
    {
        return tr("Not available");
    }
    return QStringLiteral("%1 - %2")
        .arg(QString::number(minimumDepth, 'f', 2), QString::number(maximumDepth, 'f', 2));
}
