#ifndef IDOS_WELL_RENDER_GUI_PROVIDER_H
#define IDOS_WELL_RENDER_GUI_PROVIDER_H

#include <QPointer>

#include "idos_app.h"
#include "idosrenderguiprovider.h"

class IDOSRenderServer;

class APP_EXPORT IDOSWellRenderGuiProvider : public IDOSRenderGuiProvider
{
    Q_OBJECT

  public:
    QString name() const override;
    bool supports(const IDOSRenderObject* object) const override;
    void populateMenu(QMenu* menu, IDOSRenderView* view) override;

  private slots:
    void onDelete();

  private:
    IDOSRenderView* actionView() const;
};
#endif
