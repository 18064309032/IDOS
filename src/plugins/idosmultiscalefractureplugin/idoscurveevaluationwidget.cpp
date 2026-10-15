#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextStream>
#include <QVBoxLayout>

#include "data/case/idossimulationsummaryresults.h"
#include "idoscommandmanager.h"
#include "idosproject.h"

#include "idoscurveevaluationwidget.h"

IDOSCurveEvaluationWidget::IDOSCurveEvaluationWidget(IDOSProject* project, QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_project(project)
    , m_timeMode(new QComboBox(this))
    , m_origin(new QDateTimeEdit(QDateTime::fromMSecsSinceEpoch(0, Qt::UTC), this))
    , m_unit(new QLineEdit(this))
    , m_header(new QCheckBox(tr("CSV files have a header row"), this))
    , m_confirm(new QCheckBox(tr("I confirm that both selected value columns represent the same quantity and use the common unit."), this))
    , m_metrics(new QTableWidget(0, 2, this))
    , m_samples(new QTableWidget(0, 4, this))
    , m_status(new QLabel(this))
    , m_export(new QPushButton(tr("Export CSV..."), this))
    , m_busy(false)
{
    setObjectName(QStringLiteral("idosCurveEvaluationWidget"));
    setWindowTitle(tr("Production Curve Fit Evaluation"));
    setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* explanation = new QLabel(tr("Read-only fit evaluation, not automatic history matching. Simulated values are linearly interpolated at observed times within the overlap; no extrapolation or index pairing. Times use UTC and millisecond resolution. Units are not converted."), this);
    explanation->setWordWrap(true);
    layout->addWidget(explanation);
    QHBoxLayout* sources = new QHBoxLayout();
    for (int side = 0; side < 2; ++side)
    {
        QGroupBox* group = new QGroupBox(side == 0 ? tr("Simulated curve") : tr("Observed curve"), this);
        QFormLayout* form = new QFormLayout(group);
        createSource(side, form);
        sources->addWidget(group);
    }
    layout->addLayout(sources);
    m_timeMode->setObjectName(QStringLiteral("curveTimeMode"));
    m_timeMode->addItem(tr("Elapsed days"), QStringLiteral("days"));
    m_timeMode->addItem(tr("ISO calendar dates / timestamps"), QStringLiteral("calendar"));
    m_origin->setObjectName(QStringLiteral("curveOrigin"));
    m_origin->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm:ss 'UTC'"));
    m_unit->setObjectName(QStringLiteral("curveUnit"));
    m_confirm->setObjectName(QStringLiteral("curveConfirmUnits"));
    m_header->setChecked(true);
    QFormLayout* options = new QFormLayout();
    options->addRow(tr("CSV time format:"), m_timeMode);
    options->addRow(tr("Day zero (UTC):"), m_origin);
    options->addRow(tr("Common value unit:"), m_unit);
    layout->addLayout(options);
    layout->addWidget(m_header);
    layout->addWidget(m_confirm);
    m_metrics->setObjectName(QStringLiteral("curveMetrics"));
    m_samples->setObjectName(QStringLiteral("curveSamples"));
    m_metrics->setHorizontalHeaderLabels({tr("Metric"), tr("Value")});
    m_samples->setHorizontalHeaderLabels({tr("UTC time"), tr("Simulated"), tr("Observed"), tr("Error")});
    m_metrics->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_samples->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_metrics->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_samples->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_metrics->setMaximumHeight(190);
    layout->addWidget(m_metrics);
    layout->addWidget(m_samples, 1);
    m_status->setObjectName(QStringLiteral("curveStatus"));
    m_status->setWordWrap(true);
    layout->addWidget(m_status);
    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addStretch();
    QPushButton* evaluate = new QPushButton(tr("Evaluate"), this);
    evaluate->setObjectName(QStringLiteral("curveEvaluate"));
    m_export->setEnabled(false);
    buttons->addWidget(evaluate);
    buttons->addWidget(m_export);
    QPushButton* closeButton = new QPushButton(tr("Close"), this);
    buttons->addWidget(closeButton);
    layout->addLayout(buttons);
    connect(evaluate, &QPushButton::clicked, this, &IDOSCurveEvaluationWidget::onEvaluate);
    connect(m_export, &QPushButton::clicked, this, &IDOSCurveEvaluationWidget::onExport);
    connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(m_browse[0], &QPushButton::clicked, this, &IDOSCurveEvaluationWidget::onBrowseSimulated);
    connect(m_browse[1], &QPushButton::clicked, this, &IDOSCurveEvaluationWidget::onBrowseObserved);
    if (project != nullptr)
    {
        connect(project, &IDOSProject::objectAdded, this, &IDOSCurveEvaluationWidget::onRefresh);
        connect(project, &IDOSProject::objectRemoved, this, &IDOSCurveEvaluationWidget::onRefresh);
        connect(project, &IDOSProject::objectsAdded, this, &IDOSCurveEvaluationWidget::onRefresh);
        connect(project, &IDOSProject::objectsRemoved, this, &IDOSCurveEvaluationWidget::onRefresh);
        connect(project, &IDOSProject::objectDataChanged, this, &IDOSCurveEvaluationWidget::onRefresh);
        connect(project, &IDOSProject::objectsDataChanged, this, &IDOSCurveEvaluationWidget::onRefresh);
        connect(project, &QObject::destroyed, this, &QWidget::close);
    }
    connect(m_timeMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IDOSCurveEvaluationWidget::onInvalidateResult);
    connect(m_origin, &QDateTimeEdit::dateTimeChanged, this, &IDOSCurveEvaluationWidget::onInvalidateResult);
    connect(m_unit, &QLineEdit::textChanged, this, &IDOSCurveEvaluationWidget::onInvalidateResult);
    connect(m_header, &QCheckBox::toggled, this, &IDOSCurveEvaluationWidget::onInvalidateResult);
    connect(m_confirm, &QCheckBox::toggled, this, &IDOSCurveEvaluationWidget::onInvalidateResult);
    onRefresh();
    resize(1000, 840);
}

void IDOSCurveEvaluationWidget::createSource(int side, QFormLayout* form)
{
    const QString prefix = side == 0 ? QStringLiteral("simulated") : QStringLiteral("observed");
    m_sources[side] = new QComboBox(this);
    m_series[side] = new QComboBox(this);
    m_files[side] = new QLineEdit(this);
    m_timeColumns[side] = new QSpinBox(this);
    m_valueColumns[side] = new QSpinBox(this);
    m_browse[side] = new QPushButton(tr("Browse..."), this);
    m_sources[side]->setObjectName(prefix + QStringLiteral("Source"));
    m_series[side]->setObjectName(prefix + QStringLiteral("Series"));
    m_files[side]->setObjectName(prefix + QStringLiteral("File"));
    m_timeColumns[side]->setObjectName(prefix + QStringLiteral("TimeColumn"));
    m_valueColumns[side]->setObjectName(prefix + QStringLiteral("ValueColumn"));
    m_timeColumns[side]->setRange(1, 10000);
    m_valueColumns[side]->setRange(1, 10000);
    m_valueColumns[side]->setValue(2);
    form->addRow(tr("Source:"), m_sources[side]);
    form->addRow(tr("Summary series:"), m_series[side]);
    QHBoxLayout* file = new QHBoxLayout();
    file->addWidget(m_files[side], 1);
    file->addWidget(m_browse[side]);
    form->addRow(tr("CSV file:"), file);
    form->addRow(tr("Time column (1-based):"), m_timeColumns[side]);
    form->addRow(tr("Value column (1-based):"), m_valueColumns[side]);
    connect(m_sources[side], QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IDOSCurveEvaluationWidget::onSourcesChanged);
    connect(m_series[side], QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IDOSCurveEvaluationWidget::onInvalidateResult);
    connect(m_files[side], &QLineEdit::textChanged, this, &IDOSCurveEvaluationWidget::onInvalidateResult);
    connect(m_timeColumns[side], QOverload<int>::of(&QSpinBox::valueChanged), this, &IDOSCurveEvaluationWidget::onInvalidateResult);
    connect(m_valueColumns[side], QOverload<int>::of(&QSpinBox::valueChanged), this, &IDOSCurveEvaluationWidget::onInvalidateResult);
}

void IDOSCurveEvaluationWidget::onRefresh()
{
    for (int side = 0; side < 2; ++side)
    {
        const QString selected = m_sources[side]->currentData().toString();
        const QSignalBlocker blocker(m_sources[side]);
        m_sources[side]->clear();
        m_sources[side]->addItem(tr("CSV file"), QString());
        if (!m_project.isNull())
        {
            for (IDOSDataObject* object : m_project->objects())
            {
                if (qobject_cast<IDOSSimulationSummaryResults*>(object) != nullptr)
                {
                    m_sources[side]->addItem(object->name(), object->objectId());
                }
            }
        }
        const int index = m_sources[side]->findData(selected);
        if (index >= 0)
        {
            m_sources[side]->setCurrentIndex(index);
        }
    }
    onSourcesChanged();
}

void IDOSCurveEvaluationWidget::onSourcesChanged()
{
    for (int side = 0; side < 2; ++side)
    {
        const QVariant previous = m_series[side]->currentData();
        m_series[side]->clear();
        const QString id = m_sources[side]->currentData().toString();
        const IDOSSimulationSummaryResults* source = !m_project.isNull()
            ? qobject_cast<IDOSSimulationSummaryResults*>(m_project->object(id)) : nullptr;
        const bool csv = source == nullptr;
        m_files[side]->setEnabled(csv);
        m_browse[side]->setEnabled(csv);
        m_timeColumns[side]->setEnabled(csv);
        m_valueColumns[side]->setEnabled(csv);
        m_series[side]->setEnabled(!csv);
        if (source != nullptr)
        {
            const QList<QVariantMap> series = source->series();
            for (int index = 0; index < series.size(); ++index)
            {
                m_series[side]->addItem(series.at(index).value(QStringLiteral("key")).toString()
                    + QStringLiteral(" [") + series.at(index).value(QStringLiteral("unit")).toString() + QStringLiteral("]"), index);
            }
            const int selected = m_series[side]->findData(previous);
            if (selected >= 0)
            {
                m_series[side]->setCurrentIndex(selected);
            }
            if (m_unit->text().isEmpty() && !series.isEmpty())
            {
                m_unit->setText(series.first().value(QStringLiteral("unit")).toString());
                if (!source->timeValues().isEmpty())
                {
                    m_origin->setDateTime(QDateTime::fromMSecsSinceEpoch(source->timeValues().first(), Qt::UTC));
                }
            }
        }
    }
    m_result = QJsonObject();
    m_metrics->setRowCount(0);
    m_samples->setRowCount(0);
    m_export->setEnabled(false);
    m_status->clear();
}

void IDOSCurveEvaluationWidget::browseFile(int side)
{
    QPointer<IDOSCurveEvaluationWidget> guard(this);
    m_busy = true;
    const QString file = QFileDialog::getOpenFileName(this, tr("Select Curve CSV"), QString(), tr("CSV Files (*.csv *.CSV);;All Files (*)"));
    if (guard.isNull())
    {
        return;
    }
    m_busy = false;
    if (!file.isEmpty())
    {
        m_files[side]->setText(file);
    }
}

void IDOSCurveEvaluationWidget::onBrowseSimulated()
{
    browseFile(0);
}

void IDOSCurveEvaluationWidget::onBrowseObserved()
{
    browseFile(1);
}

void IDOSCurveEvaluationWidget::onEvaluate()
{
    if (m_project.isNull())
    {
        return;
    }
    QDateTime origin = m_origin->dateTime();
    origin.setTimeSpec(Qt::UTC);
    QJsonObject arguments{{QStringLiteral("unit"), m_unit->text()}, {QStringLiteral("confirmUnits"), m_confirm->isChecked()},
        {QStringLiteral("timeMode"), m_timeMode->currentData().toString()}, {QStringLiteral("origin"), origin.toString(Qt::ISODate)},
        {QStringLiteral("header"), m_header->isChecked()}};
    for (int side = 0; side < 2; ++side)
    {
        const QString prefix = side == 0 ? QStringLiteral("simulated") : QStringLiteral("observed");
        arguments.insert(prefix + QStringLiteral("Id"), m_sources[side]->currentData().toString());
        arguments.insert(prefix + QStringLiteral("Series"), m_series[side]->currentData().toInt());
        arguments.insert(prefix + QStringLiteral("File"), m_files[side]->text());
        arguments.insert(prefix + QStringLiteral("TimeColumn"), m_timeColumns[side]->value());
        arguments.insert(prefix + QStringLiteral("ValueColumn"), m_valueColumns[side]->value());
    }
    const QJsonObject response = m_project->commandManager()->execute(QStringLiteral("analysis.curve.evaluate"), arguments);
    m_result = QJsonObject();
    m_metrics->setRowCount(0);
    m_samples->setRowCount(0);
    m_export->setEnabled(false);
    if (!response.value(QStringLiteral("success")).toBool())
    {
        m_status->setText(response.value(QStringLiteral("error")).toString());
        return;
    }
    m_result = response.value(QStringLiteral("result")).toObject();
    displayResult();
}

void IDOSCurveEvaluationWidget::displayResult()
{
    const QStringList keys = {QStringLiteral("samples"), QStringLiteral("rmse"), QStringLiteral("mae"), QStringLiteral("mape"),
        QStringLiteral("mapeSamples"), QStringLiteral("r2"), QStringLiteral("outsideOverlap")};
    const QStringList labels = {tr("Paired samples"), tr("RMSE"), tr("MAE"), tr("MAPE (%)"), tr("MAPE nonzero samples"), tr("R squared"), tr("Observed points outside overlap")};
    m_metrics->setRowCount(keys.size());
    for (int index = 0; index < keys.size(); ++index)
    {
        const QJsonValue value = m_result.value(keys.at(index));
        m_metrics->setItem(index, 0, new QTableWidgetItem(labels.at(index)));
        m_metrics->setItem(index, 1, new QTableWidgetItem(value.isNull() ? tr("Undefined") : QString::number(value.toDouble(), 'g', 12)));
    }
    const QJsonArray rows = m_result.value(QStringLiteral("rows")).toArray();
    m_samples->setRowCount(qMin(1000, rows.size()));
    for (int index = 0; index < m_samples->rowCount(); ++index)
    {
        const QJsonObject row = rows.at(index).toObject();
        m_samples->setItem(index, 0, new QTableWidgetItem(QDateTime::fromMSecsSinceEpoch(qRound64(row.value(QStringLiteral("time")).toDouble() * 86400000.0), Qt::UTC).toString(Qt::ISODateWithMs)));
        const QStringList columns = {QStringLiteral("simulated"), QStringLiteral("observed"), QStringLiteral("error")};
        for (int column = 0; column < columns.size(); ++column)
        {
            m_samples->setItem(index, column + 1, new QTableWidgetItem(QString::number(row.value(columns.at(column)).toDouble(), 'g', 12)));
        }
    }
    m_status->setText(tr("Showing %1 of %2 paired samples; CSV export includes all. MAPE excludes zero observations; R squared is undefined for constant observations.").arg(m_samples->rowCount()).arg(rows.size()));
    m_export->setEnabled(true);
}

void IDOSCurveEvaluationWidget::onExport()
{
    if (m_result.isEmpty())
    {
        return;
    }
    const QJsonObject snapshot = m_result;
    QPointer<IDOSCurveEvaluationWidget> guard(this);
    m_busy = true;
    const QString path = QFileDialog::getSaveFileName(this, tr("Export Curve Evaluation"), QString(), tr("CSV Files (*.csv)"));
    if (guard.isNull())
    {
        return;
    }
    m_busy = false;
    if (path.isEmpty())
    {
        return;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_status->setText(tr("Cannot write evaluation CSV: %1").arg(file.errorString()));
        return;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << "metric,value\n";
    for (const QString& key : QStringList{QStringLiteral("samples"), QStringLiteral("rmse"), QStringLiteral("mae"), QStringLiteral("mape"), QStringLiteral("mapeSamples"), QStringLiteral("r2"), QStringLiteral("outsideOverlap")})
    {
        const QJsonValue value = snapshot.value(key);
        stream << key << ',' << (value.isNull() ? QStringLiteral("undefined") : QString::number(value.toDouble(), 'g', 17)) << '\n';
    }
    stream << "\ninput_key,input_value\n";
    const QJsonObject inputs = snapshot.value(QStringLiteral("inputs")).toObject();
    for (QJsonObject::const_iterator iterator = inputs.constBegin(); iterator != inputs.constEnd(); ++iterator)
    {
        stream << csvText(iterator.key()) << ',' << csvText(iterator.value().toVariant().toString()) << '\n';
    }
    stream << "\ntime_utc,simulated,observed,error\n";
    for (const QJsonValue& value : snapshot.value(QStringLiteral("rows")).toArray())
    {
        const QJsonObject row = value.toObject();
        stream << QDateTime::fromMSecsSinceEpoch(qRound64(row.value(QStringLiteral("time")).toDouble() * 86400000.0), Qt::UTC).toString(Qt::ISODateWithMs)
            << ',' << QString::number(row.value(QStringLiteral("simulated")).toDouble(), 'g', 17)
            << ',' << QString::number(row.value(QStringLiteral("observed")).toDouble(), 'g', 17)
            << ',' << QString::number(row.value(QStringLiteral("error")).toDouble(), 'g', 17) << '\n';
    }
    stream.flush();
    if (stream.status() != QTextStream::Ok || !file.commit())
    {
        m_status->setText(tr("Cannot write evaluation CSV: %1").arg(file.errorString()));
        return;
    }
    m_status->setText(tr("Evaluation CSV exported to %1.").arg(path));
}

bool IDOSCurveEvaluationWidget::isBusy() const
{
    return m_busy;
}

QString IDOSCurveEvaluationWidget::csvText(const QString& value) const
{
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    return QLatin1Char('"') + escaped + QLatin1Char('"');
}

void IDOSCurveEvaluationWidget::onInvalidateResult()
{
    m_result = QJsonObject();
    m_metrics->setRowCount(0);
    m_samples->setRowCount(0);
    m_export->setEnabled(false);
    m_status->clear();
}
