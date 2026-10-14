#ifndef IDOS_SIMULATIONCASE_TREE_GUI_PROVIDER_H
#define IDOS_SIMULATIONCASE_TREE_GUI_PROVIDER_H

#include "idos_app.h"
#include "idostreeguiprovider.h"

class QAction;
class IDOSCaseObject;
class IDOSProject;

class APP_EXPORT IDOSSimulationCaseTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;

  private slots:
    void onNewCase();
    void onImportCase();
    void onImportGrid();
    void onGenerateInput();
    void onViewPropertyCurves();
    void onRenameCase();
    void onDeleteCase();

  private:
    bool caseNameExists(const IDOSTreeView* view, const QString& name) const;
    int gridPropertyCount(const IDOSTreeView* view, const QString& gridId) const;
    static QString referencedGridId(const IDOSCaseObject* caseObj);

    bool newCase(IDOSTreeView* view);
    bool importCase(IDOSTreeView* view);
    bool deleteCase(IDOSTreeView* view, const QString& caseId);
    bool importGrid(IDOSTreeView* view, const QString& targetCaseId);
    QAction* addAction(QMenu* menu, IDOSTreeView* view, const QString& text);
    IDOSTreeView* actionView() const;

  signals:
    void generateInputRequested(IDOSProject* project, const QString& caseId);
    void propertyCurvesRequested(IDOSProject* project, const QString& caseId);
};

#endif
