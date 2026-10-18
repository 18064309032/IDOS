#ifndef IDOS_TOOL_H
#define IDOS_TOOL_H

#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <vtkWeakPointer.h>

#include "idos_render.h"

class IDOSRenderView;
class vtkObject;

class RENDER_EXPORT IDOSTool : public QObject
{
    Q_OBJECT
public:
    explicit IDOSTool(IDOSRenderView* view);
    ~IDOSTool() override;
    IDOSRenderView* view() const;
    bool isActive() const;
    bool isBusy() const;
    bool setActive(bool active);
    virtual QString exclusiveGroup() const;
    virtual void cancel();
signals:
    void callbackFinished(bool consumed);
protected:
    bool eventFilter(QObject* watched, QEvent* event) override final;
    virtual bool onEventFilter(QObject* watched, QEvent* event);
    virtual void onActivated();
    virtual void onDeactivated();
    bool observeVtkEvent(vtkObject* source, unsigned long eventId, float priority = 0.0f);
    virtual void onVtkEvent(vtkObject* source, unsigned long eventId, void* callData);
private:
    void activate();
    void deactivate();
    void removeVtkObservers();
    void onVtkEventReceived(vtkObject* source, unsigned long eventId, void* callData);
    QPointer<IDOSRenderView> m_view;
    bool m_active;
    int m_callbackDepth;
    QList<vtkWeakPointer<vtkObject>> m_observerSources;
    QList<unsigned long> m_observerEvents;
    QList<unsigned long> m_observerTags;
};

#endif // IDOS_TOOL_H
