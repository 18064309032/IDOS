#ifndef IDOS_WELL_TREE_GUI_PROVIDER_H
#define IDOS_WELL_TREE_GUI_PROVIDER_H

#include <QStringList>

#include "idos_app.h"
#include "idostreeguiprovider.h"

class QAction;
class IDOSCaseObject;
class IDOSProject;
class IDOSRenderView;
class IDOSWell;
class QWidget;

class APP_EXPORT IDOSWellTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    static bool newWell(IDOSProject* project, QWidget* parent);
    static int importWellData(IDOSProject* project, QWidget* parent);
    static int importWellData(IDOSProject* project, const QString& targetWellId, QWidget* parent);
    static bool deleteWell(IDOSProject* project, const QString& wellId, QWidget* parent);

    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;
    bool isChecked(const IDOSTreeItem* item, const IDOSTreeView* view) const override;
    bool setChecked(IDOSTreeItem* item, bool checked, IDOSTreeView* view) override;

  private slots:
    void onNewWell();
    void onImportWell();
    void onDeleteWell();

  private:

    QAction* addAction(QMenu* menu, IDOSTreeView* view, const QString& text);
    IDOSTreeView* actionView() const;
};

#endif
