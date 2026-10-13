#ifndef IDOS_GRID_RENDER_PROVIDER_H
#define IDOS_GRID_RENDER_PROVIDER_H

#include <QColor>
#include <QList>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <vtkSmartPointer.h>

#include "idos_app.h"
#include "idosrenderprovider.h"
#include "idosrendertypes.h"

class IDOSGrid;
class IDOSGridProperty;

class APP_EXPORT IDOSGridRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSGridRenderProvider(IDOSGrid* dataObject);
    ~IDOSGridRenderProvider() override;
    IDOSDataObject* dataSource() const override;
    void update() override;
    bool dependsOn(const QStringList& dataIds) const override;
    bool removeDataReference(const QString& dataId) override;
    QList<vtkActor*> actors() const override;
    QList<vtkSmartPointer<vtkProp>> legends() const override;
    bool highlighted() const override;
    void setHighlighted(bool highlighted) override;
    bool visible() const override;
    void setVisible(bool visible) override;
    double opacity() const override;
    void setOpacity(double opacity) override;
    void setProperty(IDOSGridProperty* property);
    IDOSGridProperty* property() const;
    QVariantMap coloringSettings() const;
    bool setColoringSettings(const QVariantMap& settings);
    static QStringList colorRampKeys();
    static QList<QColor> colorRampColors(const QString& key, bool reversed);
    IDOSDisplayMode displayMode() const override;
    bool setDisplayMode(IDOSDisplayMode mode) override;

  private:
    static QList<QColor> colorRampStops(const QString& key);
    void applyActorProperties();
    QPointer<IDOSGrid> m_dataObject;
    QPointer<IDOSGridProperty> m_property;
    IDOSDisplayMode m_displayMode;
    QList<vtkSmartPointer<vtkActor>> m_actors;
    QList<vtkSmartPointer<vtkProp>> m_legends;
    bool m_highlighted;
    bool m_visible;
    double m_opacity;
    QVariantMap m_coloringSettings;
};

#endif
