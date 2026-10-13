#ifndef IDOS_WELL_RENDER_PROVIDER_H
#define IDOS_WELL_RENDER_PROVIDER_H

#include <QColor>
#include <QIcon>
#include <QList>
#include <QPointer>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

#include <vtkSmartPointer.h>

#include "idos_app.h"
#include "idosrenderprovider.h"
#include "idoswellpathpoint.h"

class IDOSWell;
class vtkCellArray;
class vtkPoints;

class APP_EXPORT IDOSWellRenderProvider : public IDOSRenderProvider
{
  public:
    explicit IDOSWellRenderProvider(IDOSWell* dataObject);
    ~IDOSWellRenderProvider() override;
    IDOSDataObject* dataSource() const override;
    void update() override;
    QList<vtkActor*> actors() const override;
    QList<vtkSmartPointer<vtkProp>> legends() const override;
    bool highlighted() const override;
    void setHighlighted(bool highlighted) override;
    bool visible() const override;
    void setVisible(bool visible) override;
    double opacity() const override;
    void setOpacity(double opacity) override;
    QVariantMap displaySettings() const;
    bool setDisplaySettings(const QVariantMap& settings);
    static QIcon symbolIcon(int symbol);
    int effectiveSymbol() const;

  private:
    void applyActorProperties();
    QVector<IDOSWellPathPoint> clippedPoints(double minimum, double maximum) const;
    void addPathActor(const QVector<IDOSWellPathPoint>& points, const QString& part);
    QString statusForSymbol(int symbol) const;
    QColor actorColor(const QString& part) const;
    static void appendLine(vtkPoints* points, vtkCellArray* lines, double ax, double ay, double az,
                           double bx, double by, double bz);
    static void appendCircle(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius,
                             int segments = 24);
    static void appendSquare(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius, bool diamond);
    static void appendPlus(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius);
    static void appendX(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius);
    static void appendStar(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius);
    static void appendSlash(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius);
    static void appendStem(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius);
    static void appendPlug(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius);
    static void appendCornerBox(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius);
    static void appendOilSymbol(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius, bool minor = false);
    static void appendGasSymbol(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius, bool minor = false);
    static void appendCondensateSymbol(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius);
    static void appendWellHeadSymbol(vtkPoints* points, vtkCellArray* lines, const double center[3], double size, int symbol);
    QPointer<IDOSWell> m_dataObject;
    QList<vtkSmartPointer<vtkActor>> m_actors;
    QList<vtkSmartPointer<vtkProp>> m_legends;
    QStringList m_actorParts;
    QVariantMap m_settings;
    bool m_highlighted;
    bool m_visible;
    double m_opacity;
};

#endif
