#ifndef IDOS_RENDER_OBJECT_H
#define IDOS_RENDER_OBJECT_H

#include <QString>

#include "idos_render.h"

class RENDER_EXPORT IDOSRenderObject
{
  public:
    IDOSRenderObject();
    virtual ~IDOSRenderObject();

    QString id() const;
    void setId(const QString& id);

    QString name() const;
    void setName(const QString& name);

    bool visible() const;
    void setVisible(bool visible);

  private:
    QString m_id;
    QString m_name;
    bool m_visible;
};

#endif // IDOS_RENDER_OBJECT_H
