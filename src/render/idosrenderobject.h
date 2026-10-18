#ifndef IDOS_RENDER_OBJECT_H
#define IDOS_RENDER_OBJECT_H

#include <QString>

#include "idos_render.h"

class IDOSRenderProvider;

class RENDER_EXPORT IDOSRenderObject
{
  public:
    explicit IDOSRenderObject(IDOSRenderProvider* provider);
    virtual ~IDOSRenderObject();
    IDOSRenderObject(const IDOSRenderObject&) = delete;
    IDOSRenderObject& operator=(const IDOSRenderObject&) = delete;
    QString id() const;
    QString name() const;
    void setName(const QString& name);
    IDOSRenderProvider* provider() const;
    void update();
    bool isVisible() const;
    void setVisible(bool visible);
    bool isSelected() const;
    void setSelected(bool highlighted);
    double opacity() const;
    void setOpacity(double opacity);

  private:
    QString m_id;
    QString m_name;
    IDOSRenderProvider* m_provider;
};

#endif // IDOS_RENDER_OBJECT_H
