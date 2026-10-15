#ifndef IDOS_FIELD_DIFFERENCE_WIDGET_H
#define IDOS_FIELD_DIFFERENCE_WIDGET_H

#include <QPointer>
#include <QWidget>

class IDOSProject;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;

class IDOSFieldDifferenceWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit IDOSFieldDifferenceWidget(IDOSProject* project, QWidget* parent = nullptr);

  private slots:
    void onRefresh();
    void onGridChanged();
    void onBeforeChanged();
    void onAfterChanged();
    void onCalculate();

  private:
    void populateSteps(QComboBox* property, QComboBox* steps);
    QPointer<IDOSProject> m_project;
    QComboBox* m_grid;
    QComboBox* m_before;
    QComboBox* m_after;
    QComboBox* m_beforeStep;
    QComboBox* m_afterStep;
    QLineEdit* m_name;
    QCheckBox* m_confirm;
    QLabel* m_result;
};

#endif // IDOS_FIELD_DIFFERENCE_WIDGET_H
