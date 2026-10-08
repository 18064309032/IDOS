#ifndef IDOS_RENDER_VIEW_H
#define IDOS_RENDER_VIEW_H

#include <QString>
#include <QVector>
#include <QWidget>

#include "idos_render.h"
#include "idosrendertypes.h"

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
    void setOrientation(IDOSOrientation orientation);
    void setHighlightedObjectId(const QString& objectId);

  signals:
    void objectActivated(const QString& objectId);

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

  private:
    void rebuildActors();
    void activateObjectAt(const QPoint& position);

    IDOSRenderViewPrivate* m_privateData;
};

#endif // IDOS_RENDER_VIEW_H

