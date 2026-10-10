#ifndef IDOS_RENDER_OBJECT_H
#define IDOS_RENDER_OBJECT_H

#include <QString>

#include "idos_render.h"

class IDOSRenderProvider;

class RENDER_EXPORT IDOSRenderObject
{
  public:
    explicit IDOSRenderObject(IDOSRenderProvider* provider = nullptr);
    virtual ~IDOSRenderObject();

    QString id() const;
    void setId(const QString& id);
    QString name() const;
    void setName(const QString& name);
    IDOSRenderProvider* renderProvider() const;
    void setRenderProvider(IDOSRenderProvider* provider);

  private:
    QString m_id;
    QString m_name;
    IDOSRenderProvider* m_provider;
};

#endif // IDOS_RENDER_OBJECT_H
