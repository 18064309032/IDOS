#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMap>
#include <QPushButton>
#include <QSaveFile>
#include <QStringList>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QVariantList>
#include <QVariantMap>

#include "data/case/idossimulationcaseobject.h"
#include "idossimulationpreflightwidget.h"
#include "logger/idoslogger.h"

#include "idossimulationcaseinventoryreportwidget.h"

IDOSSimulationCaseInventoryReportWidget::IDOSSimulationCaseInventoryReportWidget(
    IDOSSimulationCaseObject* simulationCase, QWidget* parent)
    :
    QWidget(parent, Qt::Window)
    , m_reportHtml(buildReportHtml(simulationCase))
    , m_reportBrowser(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("simulationCaseInventoryReportWindow"));
    setWindowTitle(
        tr("Case Inventory Report — %1").arg(simulationCase != nullptr ? simulationCase->name() : QString()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_reportBrowser = new QTextBrowser(this);
    m_reportBrowser->setOpenExternalLinks(false);
    m_reportBrowser->setHtml(m_reportHtml);
    layout->addWidget(m_reportBrowser, 1);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(1);
    QPushButton* saveButton = new QPushButton(tr("Save HTML Report"), this);
    buttonLayout->addWidget(saveButton);
    layout->addLayout(buttonLayout);
    connect(saveButton, &QPushButton::clicked, this, &IDOSSimulationCaseInventoryReportWidget::onSaveReportTriggered);
    resize(900, 680);
}

QString IDOSSimulationCaseInventoryReportWidget::buildReportHtml(IDOSSimulationCaseObject* simulationCase) const
{
    QString html = QStringLiteral("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><style>"
                                  "body{font-family:Segoe UI,Arial,sans-serif;margin:28px;color:#202124;}"
                                  "table{border-collapse:collapse;width:100%;margin:12px 0 24px;}"
                                  "th,td{border:1px solid #c9cdd2;padding:7px;text-align:left;}"
                                  "th{background:#eef1f4;}h1{margin-bottom:4px;}h2{margin-top:28px;}"
                                  "</style></head><body>");
    if (simulationCase == nullptr)
    {
        html += QStringLiteral("<p>%1</p>").arg(tr("No simulation case is selected.").toHtmlEscaped());
        html += QStringLiteral("</body></html>");
        return html;
    }

    const QVariantMap parsedData = simulationCase->parsedSourceData();
    const QVariantMap summary = parsedData.value(QStringLiteral("summary")).toMap();
    const QVariantMap sectionCounts = summary.value(QStringLiteral("sectionCounts")).toMap();
    const QVariantMap keywordCounts = summary.value(QStringLiteral("keywordCounts")).toMap();
    const QVariantMap runInfo = simulationCase->runInfo();
    const QString runDirectory = runInfo.value(QStringLiteral("runDirectory")).toString();
    const QStringList inputFiles = runInfo.value(QStringLiteral("files")).toStringList();
    IDOSSimulationPreflightWidget preflightWidget(simulationCase);
    const QString preflightStatus =
        preflightWidget.hasBlockingIssues() ? tr("Blocked") : tr("No blocking issues found");

    html += QStringLiteral("<h1>%1</h1>").arg(tr("Case Inventory Report").toHtmlEscaped());
    html += QStringLiteral("<p>%1</p>").arg(QDateTime::currentDateTime().toString(Qt::ISODate).toHtmlEscaped());
    html += QStringLiteral("<h2>%1</h2><table>").arg(tr("Case Details").toHtmlEscaped());
    html += QStringLiteral("<tr><th>%1</th><td>%2</td></tr>")
                .arg(tr("Case").toHtmlEscaped(), simulationCase->name().toHtmlEscaped());
    html += QStringLiteral("<tr><th>%1</th><td>%2</td></tr>")
                .arg(tr("DATA file").toHtmlEscaped(), simulationCase->sourceFile().toHtmlEscaped());
    html +=
        QStringLiteral("<tr><th>%1</th><td>%2</td></tr>")
            .arg(tr("DATA title").toHtmlEscaped(), summary.value(QStringLiteral("title")).toString().toHtmlEscaped());
    html += QStringLiteral("</table>");

    html += QStringLiteral("<h2>%1</h2><table><tr><th>%2</th><th>%3</th></tr>")
                .arg(tr("Input Sections").toHtmlEscaped(), tr("Section").toHtmlEscaped(), tr("Count").toHtmlEscaped());
    for (QVariantMap::const_iterator iterator = sectionCounts.constBegin(); iterator != sectionCounts.constEnd();
         ++iterator)
    {
        html += QStringLiteral("<tr><td>%1</td><td>%2</td></tr>")
                    .arg(iterator.key().toHtmlEscaped(), iterator.value().toString().toHtmlEscaped());
    }
    html += QStringLiteral("</table>");

    html += QStringLiteral("<h2>%1</h2><table><tr><th>%2</th><th>%3</th></tr>")
                .arg(tr("Keyword Records").toHtmlEscaped(), tr("Keyword").toHtmlEscaped(), tr("Count").toHtmlEscaped());
    for (QVariantMap::const_iterator iterator = keywordCounts.constBegin(); iterator != keywordCounts.constEnd();
         ++iterator)
    {
        html += QStringLiteral("<tr><td>%1</td><td>%2</td></tr>")
                    .arg(iterator.key().toHtmlEscaped(), iterator.value().toString().toHtmlEscaped());
    }
    html += QStringLiteral("</table>");

    const QVariantList keywordIndex = parsedData.value(QStringLiteral("keywordIndex")).toList();
    QStringList sourceFiles;
    for (const QVariant& keywordValue : keywordIndex)
    {
        const QString filePath = keywordValue.toMap().value(QStringLiteral("file")).toString();
        if (!filePath.isEmpty() && !sourceFiles.contains(filePath, Qt::CaseInsensitive))
        {
            sourceFiles.append(filePath);
        }
    }
    sourceFiles.sort(Qt::CaseInsensitive);
    html +=
        QStringLiteral("<h2>%1</h2><table><tr><th>%2</th><th>%3</th></tr>")
            .arg(tr("Indexed Input Files").toHtmlEscaped(), tr("File").toHtmlEscaped(), tr("Status").toHtmlEscaped());
    for (const QString& filePath : sourceFiles)
    {
        const QFileInfo fileInfo(filePath);
        html += QStringLiteral("<tr><td>%1</td><td>%2</td></tr>")
                    .arg(filePath.toHtmlEscaped(),
                         fileInfo.exists() ? tr("Available").toHtmlEscaped() : tr("Missing").toHtmlEscaped());
    }
    html += QStringLiteral("</table>");

    html += QStringLiteral("<h2>%1</h2><table>").arg(tr("Run Preparation").toHtmlEscaped());
    html += QStringLiteral("<tr><th>%1</th><td>%2</td></tr>")
                .arg(tr("Status").toHtmlEscaped(), runInfo.isEmpty()
                                                       ? tr("Not initialized").toHtmlEscaped()
                                                       : tr("Initialized; simulator not started").toHtmlEscaped());
    html += QStringLiteral("<tr><th>%1</th><td>%2</td></tr>")
                .arg(tr("Run Preflight").toHtmlEscaped(), preflightStatus.toHtmlEscaped());
    if (!runInfo.isEmpty())
    {
        html += QStringLiteral("<tr><th>%1</th><td>%2</td></tr>")
                    .arg(tr("Run directory").toHtmlEscaped(), runDirectory.toHtmlEscaped());
        html += QStringLiteral("<tr><th>%1</th><td>%2</td></tr>")
                    .arg(tr("Initialized at (UTC)").toHtmlEscaped(),
                         runInfo.value(QStringLiteral("initializedAt")).toString().toHtmlEscaped());
        html += QStringLiteral("<tr><th>%1</th><td>%2</td></tr>")
                    .arg(tr("Prepared input file count").toHtmlEscaped(), QString::number(inputFiles.size()));
    }
    html += QStringLiteral("</table>");

    QStringList resultFiles;
    if (!runDirectory.isEmpty() && QFileInfo(runDirectory).isDir())
    {
        QDirIterator iterator(runDirectory, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext())
        {
            const QString filePath = iterator.next();
            const QString relativePath = QDir::fromNativeSeparators(QDir(runDirectory).relativeFilePath(filePath));
            if (!inputFiles.contains(relativePath, Qt::CaseInsensitive))
            {
                resultFiles.append(relativePath);
            }
        }
        resultFiles.sort(Qt::CaseInsensitive);
    }
    html += QStringLiteral("<h2>%1</h2><table><tr><th>%2</th><th>%3</th></tr>")
                .arg(tr("Generated Files").toHtmlEscaped(), tr("File").toHtmlEscaped(), tr("Size").toHtmlEscaped());
    for (const QString& relativePath : resultFiles)
    {
        const QFileInfo fileInfo(QDir(runDirectory).absoluteFilePath(relativePath));
        html += QStringLiteral("<tr><td>%1</td><td>%2</td></tr>")
                    .arg(relativePath.toHtmlEscaped(), tr("%1 bytes").arg(fileInfo.size()).toHtmlEscaped());
    }
    if (resultFiles.isEmpty())
    {
        html += QStringLiteral("<tr><td colspan=\"2\">%1</td></tr>")
                    .arg(tr("No generated result files found.").toHtmlEscaped());
    }
    html += QStringLiteral("</table></body></html>");
    return html;
}

void IDOSSimulationCaseInventoryReportWidget::onSaveReportTriggered()
{
    const QString filePath =
        QFileDialog::getSaveFileName(this, tr("Save Case Inventory Report"),
                                     QStringLiteral("case_inventory_report.html"), tr("HTML files (*.html *.htm)"));
    if (filePath.isEmpty())
    {
        return;
    }

    const QByteArray contents = m_reportHtml.toUtf8();
    QSaveFile reportFile(filePath);
    if (!reportFile.open(QIODevice::WriteOnly) || reportFile.write(contents) != contents.size() || !reportFile.commit())
    {
        m_reportBrowser->setHtml(
            QStringLiteral("<p>%1</p>").arg(tr("Unable to save the report file.").toHtmlEscaped()));
        IDOS_MESSAGE(tr("Unable to save the report file."), IDOSLogLevel::Error);
    }
    else
    {
        IDOS_MESSAGE(tr("Case inventory report saved: %1").arg(filePath), IDOSLogLevel::Info);
    }
}
