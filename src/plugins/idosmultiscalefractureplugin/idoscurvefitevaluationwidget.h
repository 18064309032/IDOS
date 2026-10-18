#ifndef IDOS_CURVE_FIT_EVALUATION_WIDGET_H
#define IDOS_CURVE_FIT_EVALUATION_WIDGET_H

#include <QJsonObject>
#include <QPointer>
#include <QWidget>

class IDOSInterface;
class IDOSProject;
class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

class IDOSCurveFitEvaluationWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit IDOSCurveFitEvaluationWidget(IDOSInterface* interface);
    bool isBusy() const;

  private slots:
    void onRefresh();
    void onSourcesChanged();
    void onBrowseSimulated();
    void onBrowseObserved();
    void onEvaluate();
    void onExport();
    void onInvalidateResult();

  private:
    void createSource(int side, QFormLayout* form);
    QString csvText(const QString& value) const;
    void browseFile(int side);
    void displayResult();
    QPointer<IDOSProject> m_project;
    QComboBox* m_sources[2];
    QComboBox* m_series[2];
    QLineEdit* m_files[2];
    QSpinBox* m_timeColumns[2];
    QSpinBox* m_valueColumns[2];
    QPushButton* m_browse[2];
    QComboBox* m_timeMode;
    QDateTimeEdit* m_origin;
    QLineEdit* m_unit;
    QCheckBox* m_header;
    QCheckBox* m_confirm;
    QTableWidget* m_metrics;
    QTableWidget* m_samples;
    QLabel* m_status;
    QPushButton* m_export;
    QJsonObject m_result;
    bool m_busy;
};

#endif // IDOS_CURVE_FIT_EVALUATION_WIDGET_H
