#ifndef IDOS_GRID_FILTER_RENDER_PROVIDER_H
#define IDOS_GRID_FILTER_RENDER_PROVIDER_H

#include <QPointer>

#include "idosgridrenderprovider.h"

class IDOSGridFilter;

class APP_EXPORT IDOSGridFilterRenderProvider : public IDOSGridRenderProvider
{
  public:
    explicit IDOSGridFilterRenderProvider(IDOSGridFilter* filter);
    ~IDOSGridFilterRenderProvider() override;
    IDOSDataObject* dataSource() const override;
    QStringList dependencyIds() const override;
    void setDataReference(IDOSDataObject* data) override;
    void update() override;
    bool removeDataReference(const QString& dataId) override;

  private:
    QPointer<IDOSGridFilter> m_filter;
    QPointer<IDOSGridProperty> m_displayProperty;
    QString m_boundDisplayId;
    bool m_displayInitialized;
};

#endif
