#ifndef IDOS_RENDER_SCENE_H
#define IDOS_RENDER_SCENE_H

#include <QList>
#include <QMap>
#include <QString>

#include "idos_render.h"

class IDOSRenderObject;

class RENDER_EXPORT IDOSRenderScene
{
  public:
    IDOSRenderScene();
    ~IDOSRenderScene();

    void addObject(IDOSRenderObject* object);
    void removeObject(const QString& objectId);
    IDOSRenderObject* object(const QString& objectId) const;
    void setObject(IDOSRenderObject* object);
    void setObjectVisible(const QString& objectId, bool visible);
    void clear();

    const QList<IDOSRenderObject*>& objects() const;

  private:
    QList<IDOSRenderObject*> m_objects;
    QMap<QString, IDOSRenderObject*> m_objectById;
};

#endif // IDOS_RENDER_SCENE_H
