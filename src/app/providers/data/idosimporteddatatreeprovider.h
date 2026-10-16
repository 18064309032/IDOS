#ifndef IDOS_IMPORTED_DATA_TREE_PROVIDER_H
#define IDOS_IMPORTED_DATA_TREE_PROVIDER_H

#include <QObject>

#include "idos_app.h"
#include "providers/idostreeitemprovider.h"

class QAction;
class QMenu;

class APP_EXPORT IDOSImportedDataTreeProvider : public QObject, public IDOSTreeItemProvider
{
    Q_OBJECT

  public:
    IDOSImportedDataTreeProvider(IDOSDataObject* data);
    QMenu* createContextMenu() override;
    void refresh() override;

  private:
    QAction* addImportAction(QMenu* menu, const QString& text,
                             const QString& providerId, const QString& dataTypeId,
                             const QString& fileFilter);
};

#endif // IDOS_IMPORTED_DATA_TREE_PROVIDER_H
