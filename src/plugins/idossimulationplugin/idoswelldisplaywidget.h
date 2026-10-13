#ifndef IDOS_WELL_DISPLAY_WIDGET_H
#define IDOS_WELL_DISPLAY_WIDGET_H

#include <QMap>
#include <QPointer>
#include <QString>
#include <QWidget>

class IDOSProject;
class IDOSRenderObject;
class IDOSRenderView;
class IDOSWell;
class IDOSWellRenderProvider;
class QCheckBox;
class QColor;
class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QPushButton;
class QTabWidget;

class IDOSWellDisplayWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit IDOSWellDisplayWidget(QWidget* parent = nullptr);
    void setProject(IDOSProject* project);
    void setWell(IDOSWell* well);
    void setView(IDOSRenderView* view);
    void setTrajectory(bool trajectory);
  private slots:
    void onApply();
    void onRefresh();
    void onColor();
    void onColorSelected(const QColor& color);
    void onInvalidate();
    void onRenderObjectRemoving(const QString& id);
    void onDataRemoving(const QString& id);
  private:
    void addCheck(QFormLayout* form, const QString& key, const QString& text);
    void addNumber(QFormLayout* form, const QString& key, const QString& text, double minimum, double maximum);
    void addColor(QFormLayout* form, const QString& key, const QString& text);
    IDOSRenderObject* renderObject() const;
    IDOSWellRenderProvider* provider() const;
    void bindTarget();
    QPointer<IDOSProject> m_project;
    QPointer<IDOSWell> m_well;
    QPointer<IDOSRenderView> m_view;
    QString m_renderId;
    bool m_loading;
    QMap<QString, QCheckBox*> m_checks;
    QMap<QString, QDoubleSpinBox*> m_numbers;
    QMap<QString, QPushButton*> m_colors;
    QComboBox* m_shape;
    QComboBox* m_symbol;
    QTabWidget* m_tabs;
    QLabel* m_message;
    QLabel* m_wellName;
    QLabel* m_viewName;
    bool m_trajectory;
};

#endif
