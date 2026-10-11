#ifndef IDOS_POINT_SET_TREE_GUI_PROVIDER_H
#define IDOS_POINT_SET_TREE_GUI_PROVIDER_H

#include <QList>
#include <QPointer>

#include "idostreeguiprovider.h"

class QAction;

class IDOSPointSetTreeGuiProvider : public IDOSTreeGuiProvider
{
    Q_OBJECT

  public:
    ~IDOSPointSetTreeGuiProvider() override;
    QString name() const override;
    bool supports(const IDOSTreeItem* item) const override;
    void populateMenu(QMenu* menu, IDOSTreeView* view) override;

  private slots:
    void onCreatePointSet();
    void onDeletePointSet();

  private:
    QList<QPointer<QAction>> m_actions;
    IDOSTreeView* actionView() const;
};
#endif

