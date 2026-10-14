#ifndef IDOS_GRID_RENDER_PROVIDER_H
#define IDOS_GRID_RENDER_PROVIDER_H

#include <QColor>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <vtkSmartPointer.h>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif

#include "idos_app.h"
#include "idosrenderprovider.h"
#include "idosrendertypes.h"

class IDOSGrid;
class IDOSGridProperty;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSGridRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSGridRenderProvider(IDOSGrid* dataObject);
    ~IDOSGridRenderProvider() override;
    IDOSDataObject* dataSource() const override;
    void update() override;
    QStringList dependencyIds() const override;
    void setDataReference(IDOSDataObject* data) override;
    QVariantMap filterCriteria() const;
    bool setFilterCriteria(const QVariantMap& criteria);
    QString previewFilterId() const;
    void setPreviewFilterId(const QString& id);
    QString filterError() const;
    int filteredCellCount() const;
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
    QColor edgeColor() const;
    bool setEdgeColor(const QColor& color);
    double edgeWidth() const;
    bool setEdgeWidth(double width);

  protected:
    void setGridSource(IDOSGrid* grid);
    void setPropertySource(IDOSGridProperty* property);

  private:
    static QList<QColor> colorRampStops(const QString& key);
    void applyActorProperties();
    QPointer<IDOSGrid> m_dataObject;
    QPointer<IDOSGridProperty> m_property;
    IDOSDisplayMode m_displayMode;
    QColor m_edgeColor;
    double m_edgeWidth;
    QList<vtkSmartPointer<vtkActor>> m_actors;
    QList<vtkSmartPointer<vtkProp>> m_legends;
    bool m_highlighted;
    bool m_visible;
    double m_opacity;
    QVariantMap m_coloringSettings;
    QVariantMap m_filterCriteria;
    QString m_previewFilterId;
    QMap<QString, QPointer<IDOSGridProperty>> m_filterProperties;
    QString m_filterError;
    int m_filteredCellCount;
};

#endif
