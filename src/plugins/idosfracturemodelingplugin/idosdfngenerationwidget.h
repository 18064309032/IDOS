#ifndef IDOS_DFN_GENERATION_WIDGET_H
#define IDOS_DFN_GENERATION_WIDGET_H

#include <QPointer>
#include <QString>
#include <QWidget>

class IDOSInterface;
class IDOSProject;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

class IDOSDfnGenerationWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSDfnGenerationWidget(IDOSInterface* interface);

  private slots:
    void onRefreshInputs();
    void onGridChanged();
    void onGenerate();

  private:
    QDoubleSpinBox* createNumber(const QString& name, double minimum, double maximum, double value);
    QPointer<IDOSProject> m_project;
    QString m_gridId;
    QComboBox* m_grid;
    QComboBox* m_density;
    QLineEdit* m_name;
    QSpinBox* m_count;
    QSpinBox* m_seed;
    QSpinBox* m_firstLayer;
    QSpinBox* m_lastLayer;
    QDoubleSpinBox* m_minimumLength;
    QDoubleSpinBox* m_maximumLength;
    QDoubleSpinBox* m_aspectRatio;
    QDoubleSpinBox* m_strike;
    QDoubleSpinBox* m_dip;
    QDoubleSpinBox* m_strikeSpread;
    QDoubleSpinBox* m_dipSpread;
    QLabel* m_result;
    QPushButton* m_generate;
};

#endif // IDOS_DFN_GENERATION_WIDGET_H
