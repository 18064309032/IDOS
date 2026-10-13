#ifndef IDOS_COLORING_WIDGET_H
#define IDOS_COLORING_WIDGET_H

#include <QPointer>
#include <QString>
#include <QWidget>

class IDOSGrid;
class IDOSGridRenderProvider;
class IDOSProject;
class IDOSRenderObject;
class IDOSRenderView;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;

class IDOSColoringWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSColoringWidget(IDOSProject* project, IDOSGrid* grid, IDOSRenderView* view,
                               const QString& propertyId, QWidget* parent = nullptr);

  private slots:
    void onApply();
    void onRefresh();
    void onSolidColor();
    void onInvalidColor();
    void onInvalidate();
    void onRenderObjectRemoving(const QString& id);
    void onDataRemoving(const QString& id);

  private:
    void populateColorRamps();
    void updateRampPreviews(bool reversed);
    IDOSRenderObject* renderObject() const;
    IDOSGridRenderProvider* provider() const;
    QPointer<IDOSProject> m_project;
    QPointer<IDOSGrid> m_grid;
    QPointer<IDOSRenderView> m_view;
    QString m_renderId;
    QString m_solidColor;
    QString m_invalidColor;
    bool m_loading;
    QComboBox* m_propertyCombo;
    QComboBox* m_rampCombo;
    QCheckBox* m_automatic;
    QCheckBox* m_reversed;
    QCheckBox* m_legend;
    QDoubleSpinBox* m_minimum;
    QDoubleSpinBox* m_maximum;
    QPushButton* m_solidButton;
    QPushButton* m_invalidButton;
    QLabel* m_message;
};

#endif
