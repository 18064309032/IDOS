#ifndef IDOS_GRID_RENDER_PROVIDER_H
#define IDOS_GRID_RENDER_PROVIDER_H

#include <QColor>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QtCore/qnamespace.h>

#include <vtkSmartPointer.h>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif

#include "idos_app.h"
#include "idosrendertypes.h"
#include "providers/idosrenderprovider.h"

class IDOSGrid;
class IDOSGridProperty;
class IDOSRenderObject;
class IDOSRenderView;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSGridRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSGridRenderProvider(IDOSGrid* dataObject);
    ~IDOSGridRenderProvider() override;
    static bool supportsData(const IDOSDataObject* data);
    static QString renderDataId(const IDOSDataObject* data);
    static QStringList requiredDataIds(const IDOSDataObject* data, Qt::CheckState state);
    static void setDataChecked(IDOSDataObject* data, Qt::CheckState state, IDOSRenderView* view,
                               const QList<IDOSDataObject*>& dataObjects);
    static Qt::CheckState dataCheckState(IDOSDataObject* data, IDOSRenderView* view);
    IDOSDataObject* dataSource() const override;
    void update() override;
    QVariantMap presentationState() const override;
    bool restorePresentationState(const QVariantMap& state) override;
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
    static IDOSRenderObject* renderObject(const QString& dataId, IDOSRenderView* view);
    static QList<QColor> colorRampStops(const QString& key);
    void applyActorProperties();
    QPointer<IDOSGrid> m_dataObject;
    QPointer<IDOSGridProperty> m_property;
    QString m_restoredPropertyId;
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
