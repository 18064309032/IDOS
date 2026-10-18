#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QVariantMap>

#include "data/case/idossimulationcaseobject.h"

#include "idossimulationparsesummarywidget.h"

IDOSSimulationParseSummaryWidget::IDOSSimulationParseSummaryWidget(IDOSSimulationCaseObject* simulationCase,
                                                                   QWidget* parent)
    :
    QWidget(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationParseSummaryWindow"));
    setWindowTitle(tr("Parse Summary"));

    const QVariantMap parsedSourceData = simulationCase->parsedSourceData();
    const QVariantMap summary = parsedSourceData.value(QStringLiteral("summary")).toMap();
    QStringList lines;
    lines.append(tr("Case: %1").arg(simulationCase->name()));
    lines.append(tr("DATA title: %1").arg(summary.value(QStringLiteral("caseName")).toString()));
    lines.append(tr("DATA file: %1").arg(summary.value(QStringLiteral("filePath")).toString()));
    lines.append(QString());
    lines.append(tr("Sections"));
    const QVariantMap sectionCounts = summary.value(QStringLiteral("sectionCounts")).toMap();
    for (QVariantMap::const_iterator iterator = sectionCounts.constBegin();
         iterator != sectionCounts.constEnd();
         ++iterator)
    {
        lines.append(QStringLiteral("  %1: %2").arg(iterator.key()).arg(iterator.value().toInt()));
    }
    lines.append(QString());
    lines.append(tr("Simulation keywords"));
    const QVariantMap keywordCounts = summary.value(QStringLiteral("keywordCounts")).toMap();
    for (QVariantMap::const_iterator iterator = keywordCounts.constBegin();
         iterator != keywordCounts.constEnd();
         ++iterator)
    {
        lines.append(QStringLiteral("  %1: %2").arg(iterator.key()).arg(iterator.value().toInt()));
    }
    lines.append(QString());
    const QStringList wellNames = summary.value(QStringLiteral("wellNames")).toStringList();
    lines.append(tr("Wells found: %1").arg(wellNames.size()));
    for (const QString& wellName : wellNames)
    {
        lines.append(QStringLiteral("  %1").arg(wellName));
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    QPlainTextEdit* text = new QPlainTextEdit(this);
    text->setObjectName(QStringLiteral("simulationParseSummaryText"));
    text->setReadOnly(true);
    text->setPlainText(lines.join(QLatin1Char('\n')));
    layout->addWidget(text);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QWidget::close);
    connect(buttons, &QDialogButtonBox::accepted, this, &QWidget::close);
    layout->addWidget(buttons);
    resize(720, 560);
}

IDOSSimulationParseSummaryWidget::~IDOSSimulationParseSummaryWidget()
{
}
