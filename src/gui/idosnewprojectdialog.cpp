#include "idosnewprojectdialog.h"

#include <QComboBox>
#include <QIcon>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

IDOSNewProjectDialog::IDOSNewProjectDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("New Project"));
    setWindowIcon(QIcon(QStringLiteral(":/images/app-project-new.svg")));
    setMinimumWidth(540);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);
    QLabel* heading = new QLabel(tr("Create a Reservoir Study Project"), this);
    QFont font = heading->font();
    font.setPointSize(font.pointSize() + 3);
    font.setBold(true);
    heading->setFont(font);
    layout->addWidget(heading);

    QGroupBox* basic = new QGroupBox(tr("Basic Information"), this);
    QFormLayout* basicForm = new QFormLayout(basic);
    basicForm->setSpacing(12);
    m_name = new QLineEdit(basic);
    m_name->setObjectName(QStringLiteral("projectName"));
    m_name->setPlaceholderText(tr("Enter a project name"));
    m_name->setMaxLength(128);
    basicForm->addRow(tr("Project Name *"), m_name);
    m_fieldBlock = new QLineEdit(basic);
    m_fieldBlock->setObjectName(QStringLiteral("fieldBlock"));
    m_fieldBlock->setPlaceholderText(tr("Oil or gas field or block name (optional)"));
    basicForm->addRow(tr("Field / Block"), m_fieldBlock);
    m_description = new QPlainTextEdit(basic);
    m_description->setObjectName(QStringLiteral("projectDescription"));
    m_description->setPlaceholderText(tr("Study purpose, scope or notes (optional)"));
    m_description->setFixedHeight(76);
    basicForm->addRow(tr("Description"), m_description);
    layout->addWidget(basic);

    QGroupBox* spatial = new QGroupBox(tr("Units and Coordinates"), this);
    QFormLayout* spatialForm = new QFormLayout(spatial);
    spatialForm->setSpacing(12);
    m_units = new QComboBox(spatial);
    m_units->setObjectName(QStringLiteral("unitSystem"));
    m_units->addItem(tr("Metric"), int(IDOSProjectMetadata::UnitSystem::Metric));
    m_units->addItem(tr("Field"), int(IDOSProjectMetadata::UnitSystem::Field));
    m_units->addItem(tr("SI"), int(IDOSProjectMetadata::UnitSystem::SI));
    spatialForm->addRow(tr("Project Unit System"), m_units);
    m_coordinates = new QComboBox(spatial);
    m_coordinates->setObjectName(QStringLiteral("coordinateType"));
    m_coordinates->addItem(tr("Unspecified (set later)"), int(IDOSProjectMetadata::CoordinateType::Unspecified));
    m_coordinates->addItem(tr("Local Coordinate System"), int(IDOSProjectMetadata::CoordinateType::Local));
    m_coordinates->addItem(tr("Custom Coordinate Reference System"), int(IDOSProjectMetadata::CoordinateType::Custom));
    spatialForm->addRow(tr("Coordinate Reference System"), m_coordinates);
    m_coordinateReference = new QLineEdit(spatial);
    m_coordinateReference->setObjectName(QStringLiteral("coordinateReference"));
    m_coordinateReference->setPlaceholderText(tr("Enter the full coordinate system name or EPSG code"));
    m_coordinateReference->setEnabled(false);
    spatialForm->addRow(tr("Coordinate System Definition"), m_coordinateReference);
    QLabel* note = new QLabel(tr("Confirm source file units and coordinates separately when importing data."), spatial);
    note->setWordWrap(true);
    spatialForm->addRow(note);
    layout->addWidget(spatial);

    m_advancedToggle = new QToolButton(this);
    m_advancedToggle->setText(tr("Advanced Settings"));
    m_advancedToggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_advancedToggle->setArrowType(Qt::RightArrow);
    m_advancedToggle->setCheckable(true);
    layout->addWidget(m_advancedToggle);
    m_advanced = new QWidget(this);
    QFormLayout* advancedForm = new QFormLayout(m_advanced);
    advancedForm->setContentsMargins(0, 0, 0, 0);
    m_verticalDatum = new QLineEdit(m_advanced);
    m_verticalDatum->setObjectName(QStringLiteral("verticalDatum"));
    m_verticalDatum->setPlaceholderText(tr("For example: mean sea level or local vertical datum (optional)"));
    advancedForm->addRow(tr("Vertical Datum"), m_verticalDatum);
    m_advanced->hide();
    layout->addWidget(m_advanced);
    connect(m_advancedToggle, &QToolButton::toggled, this, &IDOSNewProjectDialog::onAdvancedToggled);

    m_validation = new QLabel(this);
    m_validation->setObjectName(QStringLiteral("validationMessage"));
    m_validation->setWordWrap(true);
    layout->addWidget(m_validation);
    QDialogButtonBox* buttons = new QDialogButtonBox(this);
    m_create = buttons->addButton(tr("Create"), QDialogButtonBox::AcceptRole);
    m_create->setObjectName(QStringLiteral("createProject"));
    m_create->setDefault(true);
    buttons->addButton(tr("Cancel"), QDialogButtonBox::RejectRole);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, &IDOSNewProjectDialog::onAccepted);
    connect(buttons, &QDialogButtonBox::rejected, this, &IDOSNewProjectDialog::onRejected);
    connect(m_name, &QLineEdit::textChanged, this, &IDOSNewProjectDialog::onInputChanged);
    connect(m_coordinateReference, &QLineEdit::textChanged, this, &IDOSNewProjectDialog::onInputChanged);
    connect(m_coordinates, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &IDOSNewProjectDialog::onCoordinateTypeChanged);
    onInputChanged();
    m_name->setFocus();
}

IDOSProjectMetadata IDOSNewProjectDialog::projectMetadata() const
{
    IDOSProjectMetadata info;
    info.setName(m_name->text().trimmed());
    info.setFieldBlock(m_fieldBlock->text().trimmed());
    info.setDescription(m_description->toPlainText().trimmed());
    info.setUnitSystem(static_cast<IDOSProjectMetadata::UnitSystem>(m_units->currentData().toInt()));
    info.setCoordinateType(
        static_cast<IDOSProjectMetadata::CoordinateType>(m_coordinates->currentData().toInt()));
    if (info.coordinateType() == IDOSProjectMetadata::CoordinateType::Custom)
    {
        info.setCoordinateReference(m_coordinateReference->text().trimmed());
    }
    info.setVerticalDatum(m_verticalDatum->text().trimmed());
    return info;
}

void IDOSNewProjectDialog::onInputChanged()
{
    const IDOSProjectMetadata info = projectMetadata();
    QString error;
    if (info.name().isEmpty())
    {
        error = tr("Please enter a project name.");
    }
    else if (info.coordinateType() == IDOSProjectMetadata::CoordinateType::Custom &&
             info.coordinateReference().isEmpty())
    {
        error = tr("Please enter the custom coordinate system name or EPSG code.");
    }
    m_validation->setText(error);
    m_create->setEnabled(error.isEmpty());
}

void IDOSNewProjectDialog::onAccepted()
{
    onInputChanged();
    if (m_create->isEnabled())
    {
        QDialog::accept();
    }
}

void IDOSNewProjectDialog::onRejected()
{
    QDialog::reject();
}

void IDOSNewProjectDialog::onAdvancedToggled(bool expanded)
{
    m_advanced->setVisible(expanded);
    m_advancedToggle->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    adjustSize();
}

void IDOSNewProjectDialog::onCoordinateTypeChanged()
{
    m_coordinateReference->setEnabled(m_coordinates->currentData().toInt() ==
                                      int(IDOSProjectMetadata::CoordinateType::Custom));
    onInputChanged();
}