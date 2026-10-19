#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QScrollArea>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVector>

#include "idosnewprojectdialog.h"

IDOSNewProjectDialog::IDOSNewProjectDialog(QWidget* parent)
    :
    QDialog(parent)
{
    setWindowTitle(tr("New Project"));
    setWindowIcon(QIcon(QStringLiteral(":/images/app-project-new.svg")));
    setMinimumWidth(540);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(16);
    QLabel* heading = new QLabel(tr("Create a Reservoir Study Project"), this);
    heading->setObjectName(QStringLiteral("projectDialogHeading"));
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
    QScrollArea* advancedScroll = new QScrollArea(this);
    advancedScroll->setWidgetResizable(true);
    const QScreen* screen = QGuiApplication::primaryScreen();
    advancedScroll->setMaximumHeight(screen != nullptr ? qBound(120, screen->availableGeometry().height() - 600, 300) : 200);
    QWidget* advancedContent = new QWidget(advancedScroll);
    m_advanced = advancedScroll;
    QFormLayout* advancedForm = new QFormLayout(advancedContent);
    advancedForm->setContentsMargins(0, 0, 0, 0);
    m_verticalDatum = new QLineEdit(m_advanced);
    m_verticalDatum->setObjectName(QStringLiteral("verticalDatum"));
    m_verticalDatum->setPlaceholderText(tr("For example: mean sea level or local vertical datum (optional)"));
    advancedForm->addRow(tr("Vertical Datum"), m_verticalDatum);
    m_lengthUnit = new QComboBox(m_advanced);
    m_lengthUnit->setObjectName(QStringLiteral("coordinateLengthUnit"));
    m_lengthUnit->addItem(tr("Metres"), 0);
    m_lengthUnit->addItem(tr("International feet"), 1);
    m_lengthUnit->addItem(tr("US survey feet"), 2);
    advancedForm->addRow(tr("Local XY / vertical units"), m_lengthUnit);
    m_zDirection = new QComboBox(m_advanced);
    m_zDirection->setObjectName(QStringLiteral("coordinateZDirection"));
    m_zDirection->addItem(tr("Unspecified (set before conversion)"), 0);
    m_zDirection->addItem(tr("Elevation positive up"), 1);
    m_zDirection->addItem(tr("Depth positive down"), 2);
    advancedForm->addRow(tr("Z direction"), m_zDirection);
    m_originX = new QDoubleSpinBox(m_advanced);
    m_originY = new QDoubleSpinBox(m_advanced);
    m_originZ = new QDoubleSpinBox(m_advanced);
    m_rotation = new QDoubleSpinBox(m_advanced);
    m_scale = new QDoubleSpinBox(m_advanced);
    const QList<QDoubleSpinBox*> offsets{m_originX, m_originY, m_originZ};
    const QStringList offsetNames{QStringLiteral("coordinateOriginX"), QStringLiteral("coordinateOriginY"),
        QStringLiteral("coordinateOriginZ")};
    for (int index = 0; index < offsets.size(); ++index)
    {
        offsets.at(index)->setRange(-1.0e12, 1.0e12);
        offsets.at(index)->setDecimals(6);
        offsets.at(index)->setObjectName(offsetNames.at(index));
    }
    m_rotation->setObjectName(QStringLiteral("coordinateRotation"));
    m_rotation->setRange(-360.0, 360.0);
    m_rotation->setDecimals(6);
    m_scale->setObjectName(QStringLiteral("coordinateScale"));
    m_scale->setRange(0.000000001, 1.0e6);
    m_scale->setDecimals(9);
    m_scale->setValue(1.0);
    advancedForm->addRow(tr("Local origin X (m)"), m_originX);
    advancedForm->addRow(tr("Local origin Y (m)"), m_originY);
    advancedForm->addRow(tr("Vertical reference elevation (m)"), m_originZ);
    advancedForm->addRow(tr("Local rotation, counterclockwise (deg)"), m_rotation);
    advancedForm->addRow(tr("Local horizontal scale"), m_scale);
    QLabel* coordinateNote = new QLabel(tr("Projected XY uses CRS units; geographic XY uses degrees. Local origins use metres in the anchor CRS. Settings do not move existing data."), m_advanced);
    coordinateNote->setWordWrap(true);
    advancedForm->addRow(coordinateNote);
    m_sourceCrs = new QLineEdit(m_advanced);
    m_sourceCrs->setObjectName(QStringLiteral("conversionSourceCrs"));
    m_sourceCrs->setPlaceholderText(QStringLiteral("EPSG:4326"));
    m_inputCoordinates = new QLineEdit(m_advanced);
    m_inputCoordinates->setObjectName(QStringLiteral("conversionInput"));
    m_inputCoordinates->setPlaceholderText(tr("X, Y, Z (source XY units, elevation in metres)"));
    advancedForm->addRow(tr("Conversion source CRS"), m_sourceCrs);
    advancedForm->addRow(tr("Source XYZ"), m_inputCoordinates);
    QPushButton* convert = new QPushButton(tr("Convert to project coordinates"), m_advanced);
    convert->setObjectName(QStringLiteral("convertCoordinates"));
    connect(convert, &QPushButton::clicked, this, &IDOSNewProjectDialog::onConvertCoordinates);
    advancedForm->addRow(convert);
    m_convertedCoordinates = new QLabel(m_advanced);
    m_convertedCoordinates->setObjectName(QStringLiteral("conversionResult"));
    m_convertedCoordinates->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_convertedCoordinates->setWordWrap(true);
    advancedForm->addRow(m_convertedCoordinates);
    QLabel* conversionNote = new QLabel(tr("Preview only. Input Z must already use the project's vertical datum. This does not modify imported objects."), m_advanced);
    conversionNote->setWordWrap(true);
    advancedForm->addRow(conversionNote);
    advancedScroll->setWidget(advancedContent);
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
    onCoordinateTypeChanged();
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
    if (info.coordinateType() != IDOSProjectMetadata::CoordinateType::Unspecified)
    {
        info.setCoordinateReference(m_coordinateReference->text().trimmed());
    }
    info.setVerticalDatum(m_verticalDatum->text().trimmed());
    info.setLengthUnit(static_cast<IDOSProjectMetadata::LengthUnit>(m_lengthUnit->currentData().toInt()));
    info.setZDirection(static_cast<IDOSProjectMetadata::ZDirection>(m_zDirection->currentData().toInt()));
    info.setOrigin(m_originX->value(), m_originY->value(), m_originZ->value());
    info.setRotationDegrees(m_rotation->value());
    info.setHorizontalScale(m_scale->value());
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
    if (error.isEmpty())
    {
        info.validateCoordinates(error);
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
    m_coordinateReference->setEnabled(m_coordinates->currentData().toInt() !=
                                      int(IDOSProjectMetadata::CoordinateType::Unspecified));
    const bool local = m_coordinates->currentData().toInt() == int(IDOSProjectMetadata::CoordinateType::Local);
    m_originX->setEnabled(local);
    m_originY->setEnabled(local);
    m_rotation->setEnabled(local);
    m_scale->setEnabled(local);
    onInputChanged();
}

void IDOSNewProjectDialog::setProjectMetadata(const IDOSProjectMetadata& metadata)
{
    setWindowTitle(tr("Project Settings"));
    findChild<QLabel*>(QStringLiteral("projectDialogHeading"))->setText(tr("Project Settings"));
    m_create->setText(tr("Apply"));
    m_name->setText(metadata.name());
    m_fieldBlock->setText(metadata.fieldBlock());
    m_description->setPlainText(metadata.description());
    m_units->setCurrentIndex(static_cast<int>(metadata.unitSystem()));
    m_coordinates->setCurrentIndex(static_cast<int>(metadata.coordinateType()));
    m_coordinateReference->setText(metadata.coordinateReference());
    m_verticalDatum->setText(metadata.verticalDatum());
    m_lengthUnit->setCurrentIndex(static_cast<int>(metadata.lengthUnit()));
    m_zDirection->setCurrentIndex(static_cast<int>(metadata.zDirection()));
    m_originX->setValue(metadata.originX());
    m_originY->setValue(metadata.originY());
    m_originZ->setValue(metadata.originZ());
    m_rotation->setValue(metadata.rotationDegrees());
    m_scale->setValue(metadata.horizontalScale());
    onInputChanged();
}

void IDOSNewProjectDialog::onConvertCoordinates()
{
    const QStringList fields = m_inputCoordinates->text().trimmed().split(
        QRegularExpression(QStringLiteral("[,\\s]+")), Qt::SkipEmptyParts);
    QVector<double> xyz;
    if (fields.size() == 3)
    {
        for (const QString& field : fields)
        {
            bool valid = false;
            const double value = field.toDouble(&valid);
            if (!valid)
            {
                break;
            }
            xyz.append(value);
        }
    }
    if (xyz.size() != 3)
    {
        m_convertedCoordinates->setText(tr("Enter three numeric coordinates."));
        return;
    }
    const IDOSProjectMetadata target = projectMetadata();
    IDOSProjectMetadata source;
    source.setCoordinateType(IDOSProjectMetadata::CoordinateType::Custom);
    source.setCoordinateReference(m_sourceCrs->text().trimmed());
    source.setZDirection(IDOSProjectMetadata::ZDirection::PositiveUp);
    source.setVerticalDatum(target.verticalDatum());
    QString error;
    if (!target.transformCoordinates(source, xyz, error))
    {
        m_convertedCoordinates->setText(error);
        return;
    }
    m_convertedCoordinates->setText(tr("Result XYZ: %1, %2, %3")
        .arg(xyz.at(0), 0, 'g', 16).arg(xyz.at(1), 0, 'g', 16).arg(xyz.at(2), 0, 'g', 16));
}
