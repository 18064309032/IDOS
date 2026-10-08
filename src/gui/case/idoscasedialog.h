#ifndef IDOS_CASE_DIALOG_H
#define IDOS_CASE_DIALOG_H

#include <QDialog>
#include <QStringList>

#include "idos_gui.h"

class QLineEdit;
class QLabel;
class QPushButton;
class IDOSProject;
class GUI_EXPORT IDOSCaseDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit IDOSCaseDialog(const QString& initialName, const QString& sourceFile, const QStringList& wellNames,
                            const IDOSProject* project, QWidget* parent = nullptr);
    QString caseName() const;
  private slots:
    void onNameChanged();
    void onAccepted();
    void onRejected();

  private:
    QLineEdit* m_name;
    QLabel* m_validation;
    QPushButton* m_confirm;
    QStringList m_existingNames;
};
#endif
