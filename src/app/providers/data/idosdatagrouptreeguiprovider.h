#ifndef IDOS_DATA_GROUP_TREE_GUI_PROVIDER_H
#define IDOS_DATA_GROUP_TREE_GUI_PROVIDER_H

#include <QPointer>
#include <QSet>
#include <QString>

#include "idos_app.h"
#include "tree/idostreeguiprovider.h"

class IDOSTreeRegistry;

class APP_EXPORT IDOSDataGroupTreeGuiProvider : public IDOSTreeGuiProvider
{
  public:
    explicit IDOSDataGroupTreeGuiProvider(IDOSTreeRegistry* registry);
    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;
    bool isCheckable(const IDOSTreeItem* item) const override;
    Qt::CheckState checkState(const IDOSTreeItem* item, const IDOSTreeView* view) const override;
    bool isChecked(const IDOSTreeItem* item, const IDOSTreeView* view) const override;
    bool setChecked(IDOSTreeItem* item, bool checked, IDOSTreeView* view) override;

  private:
    void collectDataIds(const IDOSTreeItem* item, QSet<QString>* ids) const;
    QPointer<IDOSTreeRegistry> m_registry;
};

#endif // IDOS_DATA_GROUP_TREE_GUI_PROVIDER_H
