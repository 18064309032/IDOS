#include "idoscasedialog.h"
#include "idoscaseobject.h"
#include "idosproject.h"
#include "idoswell.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

IDOSCaseDialog::IDOSCaseDialog(const QString& initialName, const QString& sourceFile, const QStringList& wellNames,
                               const IDOSProject* project, QWidget* parent)
    : QDialog(parent)
    , m_name(new QLineEdit(initialName, this))
    , m_validation(new QLabel(this))
    , m_confirm(nullptr)
{
    setObjectName(QStringLiteral("caseDialog"));
    const bool importing = !sourceFile.isEmpty();
    setWindowTitle(importing ? tr("Import Simulation Case") : tr("New Simulation Case"));
    setWindowIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
    resize(importing ? 720 : 450, importing ? 460 : 180);
    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* form = new QFormLayout();
    m_name->setObjectName(QStringLiteral("caseName"));
    form->addRow(tr("Case name:"), m_name);
    form->addRow(tr("Case type:"), new QLabel(tr("Simulation Case"), this));
    layout->addLayout(form);
    if (project != nullptr)
    {
        for (IDOSDataObject* object : project->objects())
        {
            if (qobject_cast<IDOSCaseObject*>(object) != nullptr)
            {
                m_existingNames.append(object->name().trimmed().toCaseFolded());
            }
        }
    }
    if (importing)
    {
        QLabel* source = new QLabel(tr("Source: %1").arg(sourceFile), this);
        source->setTextFormat(Qt::PlainText);
        source->setWordWrap(true);
        layout->addWidget(source);
        QLabel* note = new QLabel(
            tr("Import the case name, source file and well references. Grid geometry, simulation parameters and "
               "results are not loaded in this step. Unmatched wells remain unresolved; no wells are created."),
            this);
        note->setWordWrap(true);
        layout->addWidget(note);
        QTableWidget* table = new QTableWidget(wellNames.size(), 2, this);
        table->setObjectName(QStringLiteral("caseWellPreview"));
        table->setHorizontalHeaderLabels({tr("Well Name"), tr("Reference Status")});
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        for (int row = 0; row < wellNames.size(); ++row)
        {
            int matches = 0;
            if (project != nullptr)
            {
                for (IDOSDataObject* object : project->objects())
                {
                    if (qobject_cast<IDOSWell*>(object) != nullptr &&
                        object->name().trimmed().compare(wellNames[row].trimmed(), Qt::CaseInsensitive) == 0)
                    {
                        ++matches;
                    }
                }
            }
            table->setItem(row, 0, new QTableWidgetItem(wellNames[row]));
            table->setItem(row, 1,
                           new QTableWidgetItem(matches == 1   ? tr("Link existing well")
                                                : matches == 0 ? tr("Unresolved: well not found")
                                                               : tr("Unresolved: multiple matches")));
        }
        layout->addWidget(table);
        if (wellNames.isEmpty())
        {
            layout->addWidget(new QLabel(tr("No WELSPECS well definitions were found."), this));
        }
    }
    layout->addWidget(m_validation);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_confirm = buttons->button(QDialogButtonBox::Ok);
    m_confirm->setObjectName(QStringLiteral("confirmCase"));
    m_confirm->setText(importing ? tr("Import") : tr("Create"));
    buttons->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    connect(m_name, &QLineEdit::textChanged, this, &IDOSCaseDialog::onNameChanged);
    connect(buttons, &QDialogButtonBox::accepted, this, &IDOSCaseDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &IDOSCaseDialog::onRejected);
    layout->addWidget(buttons);
    onNameChanged();
}
QString IDOSCaseDialog::caseName() const
{
    return m_name->text().trimmed();
}
void IDOSCaseDialog::onNameChanged()
{
    const QString name = caseName();
    const bool duplicate = m_existingNames.contains(name.toCaseFolded());
    m_validation->setText(name.isEmpty() ? tr("Please enter a case name.")
                          : duplicate    ? tr("A case with this name already exists.")
                                         : QString());
    m_confirm->setEnabled(!name.isEmpty() && !duplicate);
}
void IDOSCaseDialog::onAccepted()
{
    onNameChanged();
    if (m_confirm->isEnabled())
    {
        accept();
    }
}
void IDOSCaseDialog::onRejected()
{
    reject();
}
