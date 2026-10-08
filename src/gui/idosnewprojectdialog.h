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

class GUI_EXPORT IDOSNewProjectDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit IDOSNewProjectDialog(QWidget* parent = nullptr);
    IDOSProjectMetadata projectMetadata() const;

  public slots:
    void onAccepted();

  private slots:
    void onRejected();
    void onInputChanged();
    void onAdvancedToggled(bool expanded);
    void onCoordinateTypeChanged();

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
    QLabel* m_validation;
    QPushButton* m_create;
};

#endif
