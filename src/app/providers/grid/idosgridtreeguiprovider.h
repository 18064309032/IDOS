#ifndef IDOS_GRID_TREE_GUI_PROVIDER_H
#define IDOS_GRID_TREE_GUI_PROVIDER_H

#include <QMap>
#include <QPointer>

#include "idos_app.h"
#include "tree/idostreeguiprovider.h"

class QAction;
class IDOSGridFilterWidget;
class IDOSGridSettingWidget;
class IDOSProject;
class IDOSRenderObject;
class IDOSRenderView;

class APP_EXPORT IDOSGridTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    explicit IDOSGridTreeGuiProvider(QObject* parent = nullptr);
    ~IDOSGridTreeGuiProvider() override;
    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;
    bool isChecked(const IDOSTreeItem* item, const IDOSTreeView* view) const override;
    bool setChecked(IDOSTreeItem* item, bool checked, IDOSTreeView* view) override;

  private slots:
    void onDeleteFilter();
    void onRenameFilter();
    void onFilterRenameAccepted();
    void onImportProperty();
    void onImportDynamicProperties();
    void onDeleteProperty();
    void onDeleteGrid();
    void onSettingsTriggered();
    void onDisplayTriggered();
    void onDisplayActionDestroyed(QObject* object);

  private:
    int gridPropertyCount(const IDOSTreeView* view, const QString& gridId) const;
    bool deleteProperty(IDOSTreeView* view, const QString& propertyId);
    bool importProperty(IDOSTreeView* view, const QString& targetGridId, const QString& targetCaseId);
    bool importDynamicProperties(IDOSTreeView* view, const QString& targetGridId);
    bool deleteGrid(IDOSTreeView* view, const QString& gridId);
    IDOSRenderObject* gridRenderObject(const QString& gridId, IDOSRenderView* view) const;
    QAction* addAction(QMenu* menu, IDOSTreeView* view, const QString& text);
    IDOSTreeView* actionView() const;
    void populateDisplayMenu(QMenu* menu, IDOSTreeView* view);
    QAction* addDisplayAction(QMenu* menu, IDOSTreeView* view, const QString& text,
                              const QString& gridId, const QString& propertyId);
    IDOSRenderObject* displayRenderObject(QAction* action) const;
    QPointer<IDOSGridSettingWidget> m_widget;
    QPointer<IDOSGridFilterWidget> m_filterWidget;
    QMap<QAction*, QPointer<IDOSProject>> m_displayProjects;
    QMap<QAction*, QPointer<IDOSRenderView>> m_displayViews;
    QMap<QAction*, QPointer<IDOSTreeView>> m_displayTrees;
};

#endif
