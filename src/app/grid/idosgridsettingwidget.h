#ifndef IDOS_GRID_SETTING_WIDGET_H
#define IDOS_GRID_SETTING_WIDGET_H

#include <QList>
#include <QPointer>
#include <QString>
#include <QWidget>

#include "idos_app.h"

class IDOSGridObject;
class IDOSGridRenderProvider;
class IDOSProject;
class IDOSRenderObject;
class IDOSRenderView;
class QCheckBox;
class QColor;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QTabWidget;

class APP_EXPORT IDOSGridSettingWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSGridSettingWidget(QWidget* parent = nullptr);
    void setProject(IDOSProject* project);
    void setGrid(IDOSGridObject* grid);
    void setView(IDOSRenderView* view);
    void setPropertyId(const QString& propertyId);

    bool setRenderTarget(const QString& dataId);

  private slots:
    void onApply();
    void onRefresh();
    void onSolidColor();
    void onInvalidColor();
    void onAppearanceApply();
    void onEdgeColor();
    void onColorSelected(const QColor& color);
    void onInvalidate();
    void onRenderObjectRemoving(const QString& id);
    void onDataRemoving(const QString& id);

  private:
    void populateColorRamps();
    void updateRampPreviews(bool reversed);
    IDOSRenderObject* renderObject() const;
    IDOSGridRenderProvider* provider() const;
    void bindTarget();
    QPointer<IDOSProject> m_project;
    QPointer<IDOSGridObject> m_grid;
    QPointer<IDOSRenderView> m_view;
    QString m_renderId;
    QString m_solidColor;
    QString m_invalidColor;
    QString m_edgeColor;
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
    QLabel* m_gridName;
    QLabel* m_viewName;
    QTabWidget* m_tabs;
    QComboBox* m_modeCombo;
    QCheckBox* m_edgeCheck;
    QCheckBox* m_visibleCheck;
    QDoubleSpinBox* m_opacitySpin;
    QDoubleSpinBox* m_edgeWidthSpin;
    QPushButton* m_edgeButton;
    QString m_targetDataId;
    QString m_propertyId;
};

#endif
