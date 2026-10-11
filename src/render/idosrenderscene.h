#ifndef IDOS_RENDER_SCENE_H
#define IDOS_RENDER_SCENE_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>

#include "idos_render.h"

class IDOSRenderObject;

class RENDER_EXPORT IDOSRenderScene : public QObject
{
    Q_OBJECT
  public:
    explicit IDOSRenderScene(QObject* parent = nullptr);
    ~IDOSRenderScene() override;
    // Ownership transfers only on success. Removal destroys the object after its lifecycle signal.
    bool add(IDOSRenderObject* object);
    bool remove(const QString& id);
    void clear();
    IDOSRenderObject* object(const QString& id) const;
    QList<IDOSRenderObject*> objects() const;
    QList<QString> ids() const;
    bool contains(const QString& id) const;
    void notifyChanged(const QString& id);
    void beginUpdate();
    void endUpdate();
  signals:
    void objectAdded(const QString& id);
    void objectAboutToBeRemoved(const QString& id);
    void objectRemoved(const QString& id);
    void objectChanged(const QString& id);
    void changed();

  private:
    void notify();
    QMap<QString, IDOSRenderObject*> m_objects;
    int m_updateDepth;
    bool m_pending;
};

#endif // IDOS_RENDER_SCENE_H
