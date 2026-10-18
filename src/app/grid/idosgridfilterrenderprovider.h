#ifndef IDOS_GRID_FILTER_RENDER_PROVIDER_H
#define IDOS_GRID_FILTER_RENDER_PROVIDER_H

#include <QPointer>

#include "idosgridrenderprovider.h"

class IDOSGridFilterObject;

class APP_EXPORT IDOSGridFilterRenderProvider : public IDOSGridRenderProvider
{
  public:
    IDOSGridFilterRenderProvider(IDOSGridFilterObject* filter, IDOSGridObject* grid);
    ~IDOSGridFilterRenderProvider() override;
    IDOSGridFilterObject* filter() const;
    void setProperties(const QList<IDOSGridPropertyObject*>& properties) override;
    void update() override;

  private:
    QPointer<IDOSGridFilterObject> m_filter;
    QPointer<IDOSGridPropertyObject> m_displayProperty;
    QString m_boundDisplayId;
    bool m_displayInitialized;
};

#endif
