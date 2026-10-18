#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "data/fracture/idosnaturalfracturesetobject.h"
#include "idosinterface.h"
#include "idosproject.h"
#include "idosrenderview.h"
#include "render/idosrenderserver.h"

#include "idosfracturecheckwidget.h"

IDOSFractureCheckWidget::IDOSFractureCheckWidget(IDOSInterface* interface)
    :
    QWidget(interface != nullptr ? interface->mainWindow() : nullptr, Qt::Window)
    , m_project(interface != nullptr ? interface->project() : nullptr)
    , m_targetView(interface != nullptr && interface->renderServer() != nullptr
        ? interface->renderServer()->currentView() : nullptr)
    , m_table(nullptr)
{
    IDOSProject* project = m_project.data();
    IDOSRenderView* targetView = m_targetView.data();
    QList<IDOSNaturalFractureSetObject*> sets;
    if (project != nullptr)
    {
        IDOSNaturalFractureSetObject* selected = qobject_cast<IDOSNaturalFractureSetObject*>(project->currentObject());
        if (selected != nullptr)
        {
            sets.append(selected);
        }
        else
        {
            for (IDOSDataObject* object : project->objects())
            {
                IDOSNaturalFractureSetObject* fractures = qobject_cast<IDOSNaturalFractureSetObject*>(object);
                if (fractures != nullptr)
                {
                    sets.append(fractures);
                }
            }
        }
    }
    if (interface != nullptr)
    {
        connect(interface, &IDOSInterface::projectChanged, this, &QWidget::close);
        connect(interface, &QObject::destroyed, this, &QWidget::close);
    }

    QWidget* dialog = this;
    dialog->setObjectName(QStringLiteral("idosFractureGeometryReport"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Fracture Geometry Check"));
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    QLabel* summary = new QLabel(dialog);
    layout->addWidget(summary);
    QLabel* help = new QLabel(tr("Double-click a row to select its fracture set and locate its geometry in the original view."), dialog);
    help->setWordWrap(true);
    layout->addWidget(help);
    QTableWidget* table = new QTableWidget(0, 4, dialog);
    m_table = table;
    table->setObjectName(QStringLiteral("idosFractureGeometryIssues"));
    table->setHorizontalHeaderLabels({tr("Data"), tr("Fracture ID"), tr("Severity"), tr("Result")});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    table->setColumnWidth(0, 220);
    table->setColumnWidth(1, 100);
    table->setColumnWidth(2, 80);
    if (project != nullptr)
    {
        connect(project, &IDOSProject::objectRemoved, dialog, &QWidget::close);
        connect(project, &IDOSProject::objectsRemoved, dialog, &QWidget::close);
        connect(project, &QObject::destroyed, dialog, &QWidget::close);
    }
    if (targetView != nullptr)
    {
        connect(targetView, &QObject::destroyed, dialog, &QWidget::close);
    }
    int fractureCount = 0;
    int issueCount = 0;
    int skippedCount = 0;
    for (IDOSNaturalFractureSetObject* fractures : sets)
    {
        connect(fractures, &IDOSDataObject::dataChanged, dialog, &QWidget::close);
        connect(fractures, &QObject::destroyed, dialog, &QWidget::close);
        const QVariantList reports = fractures->geometryReport();
        fractureCount += reports.size();
        if (reports.isEmpty())
        {
            const int row = table->rowCount();
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(fractures->name()));
            table->item(row, 0)->setData(Qt::UserRole, fractures->objectId());
            table->setItem(row, 1, new QTableWidgetItem(QStringLiteral("-")));
            table->setItem(row, 2, new QTableWidgetItem(tr("Warning")));
            table->setItem(row, 3, new QTableWidgetItem(tr("The fracture set is empty.")));
            ++issueCount;
        }
        for (const QVariant& reportValue : reports)
        {
            const QVariantMap report = reportValue.toMap();
            const QVariantList issues = report.value(QStringLiteral("issues")).toList();
            if (!report.value(QStringLiteral("valid")).toBool())
            {
                ++skippedCount;
            }
            issueCount += issues.size();
            const int rows = qMax(1, issues.size());
            for (int index = 0; index < rows; ++index)
            {
                const QVariantMap issue = issues.isEmpty() ? QVariantMap() : issues.at(index).toMap();
                const int row = table->rowCount();
                table->insertRow(row);
                QTableWidgetItem* dataItem = new QTableWidgetItem(fractures->name());
                dataItem->setData(Qt::UserRole, fractures->objectId());
                dataItem->setData(Qt::UserRole + 1, report.value(QStringLiteral("center")));
                table->setItem(row, 0, dataItem);
                table->setItem(row, 1, new QTableWidgetItem(report.value(QStringLiteral("fractureId")).toString()));
                table->setItem(row, 2, new QTableWidgetItem(issues.isEmpty() ? tr("OK")
                    : (issue.value(QStringLiteral("error")).toBool() ? tr("Error") : tr("Warning"))));
                table->setItem(row, 3, new QTableWidgetItem(issues.isEmpty() ? tr("No issues found.")
                    : issue.value(QStringLiteral("message")).toString()));
            }
        }
    }
    summary->setText(tr("%1 sets, %2 fractures, %3 issues, %4 polygons excluded from rendering.")
                         .arg(sets.size()).arg(fractureCount).arg(issueCount).arg(skippedCount));
    layout->addWidget(table);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Close, dialog);
    buttons->button(QDialogButtonBox::Close)->setText(tr("Close"));
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QWidget::close);
    connect(table, &QTableWidget::cellDoubleClicked, this, &IDOSFractureCheckWidget::onIssueActivated);
    layout->addWidget(buttons);
    dialog->resize(950, 500);
}

IDOSProject* IDOSFractureCheckWidget::project() const
{
    return m_project.data();
}

IDOSRenderView* IDOSFractureCheckWidget::targetView() const
{
    return m_targetView.data();
}

void IDOSFractureCheckWidget::onIssueActivated(int row, int column)
{
    Q_UNUSED(column)
    if (row < 0 || row >= m_table->rowCount())
    {
        return;
    }
    const QTableWidgetItem* item = m_table->item(row, 0);
    if (item != nullptr)
    {
        emit issueActivated(item->data(Qt::UserRole).toString(),
            item->data(Qt::UserRole + 1).toList());
    }
}
