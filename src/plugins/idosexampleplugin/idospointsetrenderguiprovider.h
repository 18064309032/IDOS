#ifndef IDOS_POINT_SET_RENDER_GUI_PROVIDER_H
#define IDOS_POINT_SET_RENDER_GUI_PROVIDER_H

#include <QList>
#include <QPointer>

#include "idosrenderguiprovider.h"

class QAction;

class IDOSPointSetRenderGuiProvider : public IDOSRenderGuiProvider
{
    Q_OBJECT

  public:
    ~IDOSPointSetRenderGuiProvider() override;
    QString name() const override;
    bool supports(const IDOSRenderObject* object) const override;
    void populateMenu(QMenu* menu, IDOSRenderView* view) override;

  private slots:
    void onIncreasePointSize();
    void onDeletePointSet();

  private:
    QList<QPointer<QAction>> m_actions;
    IDOSRenderView* actionView() const;
};
#endif

