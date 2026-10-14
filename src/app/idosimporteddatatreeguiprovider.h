#ifndef IDOS_IMPORTED_DATA_TREE_GUI_PROVIDER_H
#define IDOS_IMPORTED_DATA_TREE_GUI_PROVIDER_H

#include "idos_app.h"
#include "idostreeguiprovider.h"

class QAction;

class APP_EXPORT IDOSImportedDataTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;
    bool isCheckable(const IDOSTreeItem* item) const override;
    bool isChecked(const IDOSTreeItem* item, const IDOSTreeView* view) const override;
    bool setChecked(IDOSTreeItem* item, bool checked, IDOSTreeView* view) override;

  private slots:
    void onImportDataset();
    void onSelectSeismicInline();

  private:
    QAction* addImportAction(QMenu* menu, IDOSTreeView* view,
                             const QString& text, const QString& providerId,
                             const QString& dataTypeId, const QString& fileFilter);
    IDOSTreeView* actionView() const;
};

#endif // IDOS_IMPORTED_DATA_TREE_GUI_PROVIDER_H
