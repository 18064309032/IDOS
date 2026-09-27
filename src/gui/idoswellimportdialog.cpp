#include "idoswellimportdialog.h"
#include "idoswell.h"
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QTableWidget>
#include <QVBoxLayout>

IDOSWellImportDialog::IDOSWellImportDialog(const QString& filePath, const QList<IDOSDataObject*>& objects,
                                           const QStringList& existingNames, QWidget* parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("wellImportDialog"));
    setWindowTitle(tr("Import Well Data - Well Headers"));
    setWindowIcon(QIcon(QStringLiteral(":/images/gui-well-import.svg")));
    resize(1000, 560);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* source = new QLabel(tr("File: %1").arg(filePath), this);
    source->setTextFormat(Qt::PlainText);
    source->setWordWrap(true);
    layout->addWidget(source);
    QLabel* note = new QLabel(tr("Existing wells and repeated names are skipped (case-insensitive). KB and Symbol are "
                                 "preserved as source values; units and datum are not converted."),
                              this);
    note->setWordWrap(true);
    layout->addWidget(note);
    QTableWidget* table = new QTableWidget(objects.size(), 8, this);
    table->setObjectName(QStringLiteral("wellImportPreview"));
    table->setHorizontalHeaderLabels({tr("Well Name"), tr("X Coordinate"), tr("Y Coordinate"), tr("Top Depth"),
                                      tr("Bottom Depth"), tr("KB (source value)"), tr("Symbol"), tr("Action")});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setStretchLastSection(true);
    QSet<QString> names;
    for (const QString& name : existingNames)
    {
        names.insert(name.trimmed().toCaseFolded());
    }
    for (int row = 0; row < objects.size(); ++row)
    {
        const IDOSWell* well = qobject_cast<const IDOSWell*>(objects[row]);
        if (well == nullptr)
        {
            continue;
        }
        const IDOSWellHead head = well->wellHead();
        const QString key = well->name().trimmed().toCaseFolded();
        const bool skip = names.contains(key);
        names.insert(key);
        if (!skip)
        {
            m_selectedRows.append(row);
        }
        const QStringList values = {well->name(),
                                    QString::number(head.surfaceX(), 'g', 16),
                                    QString::number(head.surfaceY(), 'g', 16),
                                    QString::number(head.topDepth(), 'g', 16),
                                    QString::number(head.bottomDepth(), 'g', 16),
                                    QString::number(head.kb(), 'g', 16),
                                    QString::number(head.symbol()),
                                    skip ? tr("Skip duplicate") : tr("Create well")};
        for (int column = 0; column < values.size(); ++column)
        {
            table->setItem(row, column, new QTableWidgetItem(values[column]));
        }
    }
    layout->addWidget(table);
    QLabel* summary = new QLabel(tr("%1 rows: %2 wells to create, %3 duplicates to skip.")
                                     .arg(objects.size())
                                     .arg(m_selectedRows.size())
                                     .arg(objects.size() - m_selectedRows.size()),
                                 this);
    layout->addWidget(summary);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Import"));
    buttons->button(QDialogButtonBox::Ok)->setObjectName(QStringLiteral("confirmWellImport"));
    buttons->button(QDialogButtonBox::Ok)->setEnabled(!m_selectedRows.isEmpty());
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    connect(buttons, &QDialogButtonBox::accepted, this, &IDOSWellImportDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &IDOSWellImportDialog::onRejected);
    layout->addWidget(buttons);
}
QList<int> IDOSWellImportDialog::selectedRows() const
{
    return m_selectedRows;
}
void IDOSWellImportDialog::onAccepted()
{
    if (!m_selectedRows.isEmpty())
    {
        accept();
    }
}
void IDOSWellImportDialog::onRejected()
{
    reject();
}
