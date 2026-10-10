#ifndef IDOS_RENDER_OBJECT_PROVIDER_H
#define IDOS_RENDER_OBJECT_PROVIDER_H

#include <QList>
#include <QString>
#include <QStringList>

#include "idos_app.h"
#include "idosrendermetadata.h"
#include "idosrenderobjectvtkconverter.h"
#include "idosrenderprovider.h"
#include "idosrendertypes.h"

class IDOSDataObject;
class IDOSRenderObject;
class IDOSRenderProvider;

class APP_EXPORT IDOSDataRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSDataRenderProvider(IDOSDataObject* dataObject);
    ~IDOSDataRenderProvider() override;

    IDOSDataObject* dataObject() const;

  private:
    IDOSDataObject* m_dataObject;
};

class APP_EXPORT IDOSRenderProviderContext
{
  public:
    virtual ~IDOSRenderProviderContext();

    virtual IDOSDataObject* dataObject(const QString& objectId) const = 0;
    virtual QList<IDOSDataObject*> dataObjects() const = 0;
    virtual IDOSRenderObject* renderObject(const QString& objectId) const = 0;
    virtual QList<IDOSRenderObject*> renderObjects() const = 0;
    virtual void addRenderObject(IDOSRenderObject* object) = 0;
    virtual void removeRenderObject(const QString& objectId) = 0;
};

enum class IDOSRenderObjectChange
{
    ProjectReplaced,
    Added,
    DataChanged,
    VisibilityChanged,
    Removed
};

class APP_EXPORT IDOSRenderObjectProvider : public IDOSRenderObjectVtkConverter,
                                           public IDOSRenderMetadata
{
  public:
    IDOSRenderObjectProvider();
    virtual ~IDOSRenderObjectProvider();

    virtual QString dataTypeId() const override = 0;
    bool canConvert(const IDOSRenderObject* object) const override;
    QList<vtkActor*> toVtk(const IDOSRenderObject* object,
                           bool highlighted) const override;
    QList<vtkSmartPointer<vtkProp>> toVtkLegends(const IDOSRenderObject* object) const override;
    virtual bool synchronize(IDOSRenderObjectChange change,
                             const QStringList& objectIds,
                             IDOSRenderProviderContext* context,
                             bool* resetCamera);
    virtual bool canSetDisplayMode(const IDOSRenderObject* object) const;
    virtual bool setDisplayMode(IDOSRenderObject* object, IDOSDisplayMode mode) const;
    virtual IDOSDisplayMode displayMode(const IDOSRenderObject* object) const;

  protected:
    IDOSRenderProvider* createRenderProvider(IDOSDataObject* dataObject,
                                             IDOSRenderObject* renderObject) const;
};

#endif // IDOS_RENDER_OBJECT_PROVIDER_H

