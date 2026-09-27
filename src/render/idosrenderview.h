#ifndef IDOS_RENDER_VIEW_H
#define IDOS_RENDER_VIEW_H

#include "idos_render.h"

#include <QString>
#include <QVector>
#include <QWidget>

class IDOSRenderObject;
class IDOSRenderScene;
class IDOSRenderViewPrivate;

class RENDER_EXPORT IDOSRenderView : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSRenderView(QWidget* parent = nullptr);
    ~IDOSRenderView() override;

    IDOSRenderScene* scene() const;
    void setScene(IDOSRenderScene* scene);

    void addObject(IDOSRenderObject* object);
    void setObject(IDOSRenderObject* object);
    QString currentObjectId() const;
    bool setCellScalars(const QString& name, const QVector<double>& values);
    void clear();
    void refresh();
    void resetCamera();

  private:
    void rebuildActors();

    IDOSRenderViewPrivate* m_privateData;
};

#endif // IDOS_RENDER_VIEW_H

