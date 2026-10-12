#ifndef IDOS_TOOL_PROVIDER_H
#define IDOS_TOOL_PROVIDER_H

#include <QList>
#include <QObject>
#include <QPointer>

#include "idos_render.h"

class QAction;
class IDOSRenderView;
class IDOSTool;

class RENDER_EXPORT IDOSToolProvider : public QObject
{
    Q_OBJECT
public:
    explicit IDOSToolProvider(IDOSRenderView* view);
    ~IDOSToolProvider() override;
    IDOSRenderView* view() const;
    virtual QList<QAction*> actions() const = 0;
    virtual QList<IDOSTool*> tools() const = 0;
    virtual void updateState() = 0;
private:
    QPointer<IDOSRenderView> m_view;
};

#endif // IDOS_TOOL_PROVIDER_H
