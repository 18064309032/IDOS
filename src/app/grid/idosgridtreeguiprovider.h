#ifndef IDOS_GRID_TREE_GUI_PROVIDER_H
#define IDOS_GRID_TREE_GUI_PROVIDER_H

#include <QStringList>

#include "idos_app.h"
#include "idostreeguiprovider.h"

class QAction;
class IDOSCaseObject;
class IDOSProject;
class IDOSWell;
class QWidget;

class APP_EXPORT IDOSGridTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    static bool deleteProperty(IDOSProject* project, const QString& propertyId, QWidget* parent);
    static bool importProperty(IDOSProject* project, const QString& targetGridId,
                               const QString& targetCaseId, QWidget* parent);
    static bool deleteGrid(IDOSProject* project, const QString& gridId, QWidget* parent);

    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;
    bool setChecked(IDOSTreeItem* item, bool checked) override;

  private slots:
    void onImportProperty();
    void onDeleteProperty();
    void onDeleteGrid();

  private:
    static QStringList collectGridPropertyIds(const IDOSProject* project, const QString& gridId);

    QAction* addAction(QMenu* menu, IDOSTreeView* view, const QString& text);
    IDOSTreeView* actionView() const;
};

#endif
