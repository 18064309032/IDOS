#ifndef IDOS_GRID_FILTER_WIDGET_H
#define IDOS_GRID_FILTER_WIDGET_H

#include <QList>
#include <QMap>
#include <QPointer>
#include <QVariantMap>
#include <QVector>
#include <QWidget>

#include "idos_app.h"

class IDOSGridObject;
class IDOSGridPropertyObject;
class IDOSGridRenderProvider;
class IDOSProject;
class IDOSRenderObject;
class IDOSRenderView;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTabWidget;
class QTimer;

class APP_EXPORT IDOSGridFilterWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit IDOSGridFilterWidget(QWidget* parent = nullptr);
    void setProject(IDOSProject* project);
    void setGrid(IDOSGridObject* grid);
    void setView(IDOSRenderView* view);
    void setFilter(const QString& filterId);

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

  private slots:
    void onApply();
    void onReset();
    void onSave();
    void onSaveAs();
    void onNameAccepted();
    void onFilterSelected();
    void onAxisSelect();
    void onAxisClear();
    void onPropertySelected();
    void onPropertyChecked(QListWidgetItem* item);
    void onPropertyClear();
    void onPropertySearch(const QString& text);
    void onEditorChanged();
    void onSummaryRefresh();
    void onRefresh();
    void onInvalidate();
    void onDataRemoving(const QString& id);
    void onRenderObjectRemoving(const QString& id);

  private:
    void buildUi();
    void bindTarget();
    QWidget* buildAxisPanel(int axis);
    QWidget* buildPropertyPage();
    void populateObjects();
    void loadCriteria(const QVariantMap& criteria);
    QVariantMap criteriaFromUi() const;
    void storePropertyCondition();
    void loadPropertyCondition(const QString& id);
    QVariantMap defaultCondition(IDOSGridPropertyObject* property) const;
    bool selectTarget(const QString& filterId);
    bool saveFilter(const QString& name, bool create);
    IDOSRenderObject* renderObject() const;
    IDOSGridRenderProvider* provider() const;
    QMap<QString, IDOSGridPropertyObject*> propertyReferences() const;
    void drawPropertyCharts();
    void updateRangeFromPosition(int x);
    QPointer<IDOSProject> m_project;
    QPointer<IDOSGridObject> m_grid;
    QPointer<IDOSRenderView> m_view;
    QLabel* m_gridName;
    QLabel* m_viewName;
    QString m_filterId;
    QString m_renderId;
    QString m_propertyId;
    bool m_loading;
    QMap<QString, QVariantMap> m_conditionStates;
    QComboBox* m_filterObjects;
    QPushButton* m_save;
    QTabWidget* m_tabs;
    QRadioButton* m_union;
    QRadioButton* m_intersection;
    QRadioButton* m_show;
    QRadioButton* m_hide;
    QRadioButton* m_invert;
    QList<QCheckBox*> m_axisEnabled;
    QList<QSpinBox*> m_axisMinimum;
    QList<QSpinBox*> m_axisMaximum;
    QList<QListWidget*> m_axisLists;
    QList<QSpinBox*> m_axisSteps;
    QLineEdit* m_propertySearch;
    QListWidget* m_propertyList;
    QLabel* m_selectedCount;
    QLabel* m_propertyTitle;
    QLabel* m_propertyRange;
    QComboBox* m_displayProperty;
    QComboBox* m_propertyLogic;
    QCheckBox* m_propertyEnabled;
    QComboBox* m_comparison;
    QDoubleSpinBox* m_minimum;
    QDoubleSpinBox* m_maximum;
    QRadioButton* m_propertyShow;
    QRadioButton* m_propertyHide;
    QRadioButton* m_propertyInvert;
    QCheckBox* m_includeMinimum;
    QCheckBox* m_includeMaximum;
    QCheckBox* m_includeInvalid;
    QDoubleSpinBox* m_epsilon;
    QLabel* m_histogram;
    QLabel* m_rangeBar;
    QLabel* m_summary;
    QTimer* m_summaryTimer;
    double m_dataMinimum;
    double m_dataMaximum;
    QVector<int> m_bins;
    QVector<int> m_selectedBins;
    int m_dragHandle;
};

#endif
