#ifndef IDOS_RENDER_GUI_PROVIDER_H
#define IDOS_RENDER_GUI_PROVIDER_H

#include <QObject>
#include <QString>

#include "idos_render.h"

class IDOSRenderObject;
class IDOSRenderView;
class QMenu;

class RENDER_EXPORT IDOSRenderGuiProvider : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSRenderGuiProvider(QObject* parent = nullptr);
    ~IDOSRenderGuiProvider() override;
    virtual QString name() const = 0;
    virtual bool supports(const IDOSRenderObject* object) const = 0;
    virtual void populateMenu(QMenu* menu, IDOSRenderView* view) = 0;
};

#endif
