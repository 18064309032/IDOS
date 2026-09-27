#include "idoswellpathimportdialog.h"
#include "idosproject.h"
#include "idoswell.h"
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QTableWidget>
#include <QVBoxLayout>

IDOSWellPathImportDialog::IDOSWellPathImportDialog(const QStringList& files, const QList<IDOSWell*>& wells,
                                                   const QStringList& errors, const IDOSProject* project,
                                                   QWidget* parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("wellPathImportDialog"));
    setWindowTitle(tr("Import Well Data - Trajectories"));
    setWindowIcon(QIcon(QStringLiteral(":/images/gui-well-trajectory.svg")));
    resize(1100, 560);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* note = new QLabel(tr("Match existing wells by WELL NAME (case-insensitive). Missing, ambiguous or repeated "
                                 "wells and existing trajectories are skipped. Source coordinates are preserved; no "
                                 "unit conversion or wellhead segment is added."),
                              this);
    note->setWordWrap(true);
    layout->addWidget(note);
    QTableWidget* table = new QTableWidget(files.size(), 6, this);
    table->setObjectName(QStringLiteral("wellPathPreview"));
    table->setHorizontalHeaderLabels(
        {tr("File"), tr("Well Name"), tr("Points"), tr("Start MD"), tr("End MD"), tr("Action / Reason")});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
    QSet<QString> seen;
    int ready = 0;
    for (int row = 0; row < files.size(); ++row)
    {
        const IDOSWell* source = wells.value(row, nullptr);
        QString reason = errors.value(row);
        QString targetId;
        QStringList values = {QFileInfo(files[row]).fileName(), QString(), QString(), QString(), QString(), QString()};
        if (source != nullptr && reason.isEmpty())
        {
            const QString name = source->name().trimmed().toCaseFolded();
            const QVector<IDOSWellPathPoint> points = source->path().points();
            values[1] = source->name();
            values[2] = QString::number(points.size());
            if (!points.isEmpty())
            {
                values[3] = QString::number(points.first().md(), 'g', 16);
                values[4] = QString::number(points.last().md(), 'g', 16);
            }
            QList<IDOSWell*> matches;
            if (project != nullptr)
            {
                for (IDOSDataObject* object : project->objects())
                {
                    IDOSWell* well = qobject_cast<IDOSWell*>(object);
                    if (well != nullptr && well->name().trimmed().toCaseFolded() == name)
                    {
                        matches.append(well);
                    }
                }
            }
            if (points.isEmpty())
            {
                reason = tr("Skip: empty trajectory");
            }
            else if (matches.isEmpty())
            {
                reason = tr("Skip: well not found");
            }
            else if (matches.size() != 1)
            {
                reason = tr("Skip: multiple wells match");
            }
            else if (matches.first()->hasPath() || !matches.first()->path().isEmpty())
            {
                reason = tr("Skip: trajectory already exists");
            }
            else if (seen.contains(name))
            {
                reason = tr("Skip: repeated well in selection");
            }
            else
            {
                seen.insert(name);
                targetId = matches.first()->objectId();
                reason = tr("Import trajectory");
                ++ready;
            }
        }
        else if (reason.isEmpty())
        {
            reason = tr("Skip: no valid trajectory");
        }
        m_targetIds.append(targetId);
        values[5] = reason;
        for (int column = 0; column < values.size(); ++column)
        {
            QTableWidgetItem* item = new QTableWidgetItem(values[column]);
            item->setToolTip(column == 0 ? files[row] : values[column]);
            table->setItem(row, column, item);
        }
    }
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    table->setColumnWidth(0, 220);
    layout->addWidget(table);
    layout->addWidget(new QLabel(tr("%1 files: %2 trajectories to import, %3 files to skip.")
                                     .arg(files.size())
                                     .arg(ready)
                                     .arg(files.size() - ready),
                                 this));
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Import"));
    buttons->button(QDialogButtonBox::Ok)->setObjectName(QStringLiteral("confirmPathImport"));
    buttons->button(QDialogButtonBox::Ok)->setEnabled(ready > 0);
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    connect(buttons, &QDialogButtonBox::accepted, this, &IDOSWellPathImportDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &IDOSWellPathImportDialog::onRejected);
    layout->addWidget(buttons);
}
QStringList IDOSWellPathImportDialog::targetIds() const
{
    return m_targetIds;
}
void IDOSWellPathImportDialog::onAccepted()
{
    for (const QString& id : m_targetIds)
    {
        if (!id.isEmpty())
        {
            accept();
            return;
        }
    }
}
void IDOSWellPathImportDialog::onRejected()
{
    reject();
}
