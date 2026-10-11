#ifndef IDOS_GRID_RENDER_GUI_PROVIDER_H
#define IDOS_GRID_RENDER_GUI_PROVIDER_H

#include <QPointer>
#include <QStringList>

#include "idos_app.h"
#include "idosrenderguiprovider.h"

class IDOSRenderServer;

class APP_EXPORT IDOSGridRenderGuiProvider : public IDOSRenderGuiProvider
{
    Q_OBJECT

  public:
    explicit IDOSGridRenderGuiProvider(IDOSRenderServer* server);
    QString name() const override;
    bool supports(const IDOSRenderObject* object) const override;
    void populateMenu(QMenu* menu, IDOSRenderView* view) override;

  private slots:
    void onDelete();
    void onDisplayMode();
    void onObjectsChanged(IDOSRenderView* view, const QStringList& ids, bool dataChanged);
    void onDataRemoving(const QString& id);

  private:
    IDOSRenderView* actionView() const;
    QPointer<IDOSRenderServer> m_server;
};
#endif
