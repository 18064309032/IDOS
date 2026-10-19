#ifndef IDOS_GRID_RENDER_PROVIDER_H
#define IDOS_GRID_RENDER_PROVIDER_H

#include <QColor>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>
#include <QtCore/qnamespace.h>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#endif
#include <vtkSmartPointer.h>
#include <vtkType.h>
#include <vtkUnstructuredGrid.h>

#include "idos_app.h"
#include "idosrenderprovider.h"
#include "idosrendertypes.h"

class IDOSDataObject;
class IDOSGridObject;
class IDOSGridPropertyObject;
class IDOSRenderObject;
class IDOSRenderView;
#ifndef IDOS_ENABLE_RENDER
class vtkActor;
#endif

class APP_EXPORT IDOSGridRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSGridRenderProvider(IDOSGridObject* dataObject);
    ~IDOSGridRenderProvider() override;
    static IDOSRenderObject* renderObject(const QString& dataId, IDOSRenderView* view);
    static bool supportsData(const IDOSDataObject* data);
    static QString renderDataId(const IDOSDataObject* data);
    static QStringList requiredDataIds(const IDOSDataObject* data, Qt::CheckState state);
    static void setDataChecked(IDOSDataObject* data, Qt::CheckState state, IDOSRenderView* view,
                               const QList<IDOSDataObject*>& dataObjects);
    static Qt::CheckState dataCheckState(IDOSDataObject* data, IDOSRenderView* view);
    IDOSGridObject* grid() const;
    void update() override;
    QList<vtkProp*> props() const override;
    QVariantMap saveState() const override;
    bool restoreState(const QVariantMap& state) override;
    void setPropertyInput(IDOSGridPropertyObject* property);
    virtual void setProperties(const QList<IDOSGridPropertyObject*>& properties);
    QVariantMap filterCriteria() const;
    bool setFilterCriteria(const QVariantMap& criteria);
    QString previewFilterId() const;
    void setPreviewFilterId(const QString& id);
    QString filterError() const;
    int filteredCellCount() const;
    bool isSelected() const;
    void setSelected(bool highlighted);
    bool isVisible() const;
    void setVisible(bool visible);
    double opacity() const;
    void setOpacity(double opacity);
    void setProperty(IDOSGridPropertyObject* property);
    IDOSGridPropertyObject* property() const;
    QVariantMap coloringSettings() const;
    bool setColoringSettings(const QVariantMap& settings);
    static QStringList colorRampKeys();
    static QList<QColor> colorRampColors(const QString& key, bool reversed);
    IDOSDisplayMode displayMode() const;
    bool setDisplayMode(IDOSDisplayMode mode);
    QColor edgeColor() const;
    bool setEdgeColor(const QColor& color);
    double edgeWidth() const;
    bool setEdgeWidth(double width);

  protected:
    void setGridSource(IDOSGridObject* grid);
    void setPropertySource(IDOSGridPropertyObject* property);

  private:
    static QList<QColor> colorRampStops(const QString& key);
    void applyActorProperties();
    QPointer<IDOSGridObject> m_dataObject;
    QPointer<IDOSGridPropertyObject> m_property;
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
    QMap<QString, QPointer<IDOSGridPropertyObject>> m_filterProperties;
    QString m_filterError;
    int m_filteredCellCount;
    vtkSmartPointer<vtkUnstructuredGrid> m_geometry;
    QVector<double> m_geometryCoord;
    QVector<double> m_geometryCorners;
    QVector<int> m_geometryActive;
    QVector<quint8> m_geometrySelection;
    QList<int> m_geometryDimensions;
    vtkMTimeType m_geometryTime;
};

#endif
