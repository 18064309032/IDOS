#ifndef IDOS_NEW_PROJECT_DIALOG_H
#define IDOS_NEW_PROJECT_DIALOG_H

#include <QDialog>

#include "idos_gui.h"
#include "idosprojectmetadata.h"

class QLineEdit;
class QPlainTextEdit;
class QComboBox;
class QLabel;
class QPushButton;
class QToolButton;
class QDoubleSpinBox;

class GUI_EXPORT IDOSNewProjectDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit IDOSNewProjectDialog(QWidget* parent = nullptr);
    IDOSProjectMetadata projectMetadata() const;
    void setProjectMetadata(const IDOSProjectMetadata& metadata);

  public slots:
    void onAccepted();

  private slots:
    void onRejected();
    void onInputChanged();
    void onAdvancedToggled(bool expanded);
    void onCoordinateTypeChanged();
    void onConvertCoordinates();

  private:
    QToolButton* m_advancedToggle;
    QWidget* m_advanced;
    QLineEdit* m_name;
    QLineEdit* m_fieldBlock;
    QPlainTextEdit* m_description;
    QComboBox* m_units;
    QComboBox* m_coordinates;
    QLineEdit* m_coordinateReference;
    QLineEdit* m_verticalDatum;
    QComboBox* m_lengthUnit;
    QComboBox* m_zDirection;
    QDoubleSpinBox* m_originX;
    QDoubleSpinBox* m_originY;
    QDoubleSpinBox* m_originZ;
    QDoubleSpinBox* m_rotation;
    QDoubleSpinBox* m_scale;
    QLineEdit* m_sourceCrs;
    QLineEdit* m_inputCoordinates;
    QLabel* m_convertedCoordinates;
    QLabel* m_validation;
    QPushButton* m_create;
};

#endif
