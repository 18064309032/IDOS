#ifndef IDOS_SIMULATIONCASE_TREE_GUI_PROVIDER_H
#define IDOS_SIMULATIONCASE_TREE_GUI_PROVIDER_H

#include <QStringList>

#include "idos_app.h"
#include "idostreeguiprovider.h"

class QAction;
class IDOSCaseObject;
class IDOSProject;
class IDOSWell;
class QWidget;

class APP_EXPORT IDOSSimulationCaseTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    static bool newCase(IDOSProject* project, QWidget* parent);
    static bool importCase(IDOSProject* project, QWidget* parent);
    static bool importCase(IDOSProject* project, const QString& filePath, QWidget* parent);
    static bool deleteCase(IDOSProject* project, const QString& caseId, QWidget* parent);
    static bool importGrid(IDOSProject* project, const QString& targetCaseId, QWidget* parent);

    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;

  private slots:
    void onNewCase();
    void onImportCase();
    void onImportGrid();
    void onDeleteCase();

  private:
    static bool caseNameExists(const IDOSProject* project, const QString& name);
    static QStringList collectGridPropertyIds(const IDOSProject* project, const QString& gridId);
    static QString referencedGridId(const IDOSCaseObject* caseObj);

    QAction* addAction(QMenu* menu, IDOSTreeView* view, const QString& text);
    IDOSTreeView* actionView() const;
};

#endif
