#include <algorithm>
#include <cmath>
#include <limits>

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPointF>
#include <QtGlobal>
#include <QVector3D>

#include <vtkActor.h>
#include <vtkBillboardTextActor3D.h>
#include <vtkCellArray.h>
#include <vtkConeSource.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>
#include <vtkSplineFilter.h>
#include <vtkTextProperty.h>

#include "idoswell.h"
#include "idoswellgeometryresolver.h"

#include "idoswellrenderprovider.h"

IDOSWellRenderProvider::IDOSWellRenderProvider(IDOSWell* dataObject)
    :
    m_dataObject(dataObject)
    , m_actors()
    , m_legends()
    , m_actorParts()
    , m_settings()
    , m_highlighted(false)
    , m_visible(dataObject != nullptr && dataObject->isVisible())
    , m_opacity(1.0)
{
    const bool injector = dataObject != nullptr && dataObject->type() == IDOSWell::Type::Injector;
    const QString color = injector ? QStringLiteral("#33e640") : QStringLiteral("#f23326");
    m_settings.insert(QStringLiteral("headVisible"), true);
    m_settings.insert(QStringLiteral("headShape"), QStringLiteral("symbol"));
    m_settings.insert(QStringLiteral("symbol"), -1);
    m_settings.insert(QStringLiteral("symbolWidth"), 2.5);
    m_settings.insert(QStringLiteral("tubes"), true);
    m_settings.insert(QStringLiteral("statusColors"), true);
    m_settings.insert(QStringLiteral("sameCompletionColors"), true);
    const QStringList statuses{QStringLiteral("producer"), QStringLiteral("injector"),
        QStringLiteral("closed"), QStringLiteral("stopped")};
    const QStringList statusColors{QStringLiteral("#eb1f0d"), QStringLiteral("#2e0ff2"),
        QStringLiteral("#61ff2e"), QStringLiteral("#61ff2e")};
    for (int index = 0; index < statuses.size(); ++index)
    {
        m_settings.insert(statuses.at(index) + QStringLiteral("Color"), statusColors.at(index));
        m_settings.insert(statuses.at(index) + QStringLiteral("CompletionColor"), statusColors.at(index));
    }
    m_settings.insert(QStringLiteral("headSize"), 12.0);
    m_settings.insert(QStringLiteral("headColor"), color);
    m_settings.insert(QStringLiteral("trajectoryVisible"), true);
    m_settings.insert(QStringLiteral("trajectoryColor"), color);
    m_settings.insert(QStringLiteral("trajectoryWidth"), 2.5);
    m_settings.insert(QStringLiteral("smooth"), false);
    m_settings.insert(QStringLiteral("clip"), false);
    m_settings.insert(QStringLiteral("minimum"), 0.0);
    m_settings.insert(QStringLiteral("maximum"), 10000.0);
    m_settings.insert(QStringLiteral("completionVisible"), false);
    m_settings.insert(QStringLiteral("completionColor"), QStringLiteral("#ffff00"));
    m_settings.insert(QStringLiteral("completionWidth"), 6.0);
    m_settings.insert(QStringLiteral("labelVisible"), false);
    m_settings.insert(QStringLiteral("labelColor"), QStringLiteral("#ffffff"));
    m_settings.insert(QStringLiteral("labelSize"), 14.0);
}

IDOSWellRenderProvider::~IDOSWellRenderProvider()
{
}

IDOSDataObject* IDOSWellRenderProvider::dataSource() const
{
    return m_dataObject.data();
}

QVariantMap IDOSWellRenderProvider::displaySettings() const
{
    return m_settings;
}

bool IDOSWellRenderProvider::setDisplaySettings(const QVariantMap& settings)
{
    QVariantMap candidate = m_settings;
    for (QVariantMap::const_iterator entry = settings.constBegin(); entry != settings.constEnd(); ++entry)
    {
        if (!candidate.contains(entry.key()))
        {
            return false;
        }
        candidate.insert(entry.key(), entry.value());
    }
    for (QVariantMap::const_iterator entry = candidate.constBegin(); entry != candidate.constEnd(); ++entry)
    {
        if (entry.key().endsWith(QStringLiteral("Color")) && !QColor(entry.value().toString()).isValid())
        {
            return false;
        }
    }
    const QStringList sizes{QStringLiteral("headSize"), QStringLiteral("trajectoryWidth"),
        QStringLiteral("completionWidth"), QStringLiteral("labelSize"), QStringLiteral("symbolWidth")};
    for (const QString& key : sizes)
    {
        bool valid = false;
        const double value = candidate.value(key).toDouble(&valid);
        const double maximum = key == QStringLiteral("headSize") ? 10000.0 : 100.0;
        if (!valid || !std::isfinite(value) || value < 1.0 || value > maximum)
        {
            return false;
        }
    }
    bool minimumValid = false;
    bool maximumValid = false;
    const double minimum = candidate.value(QStringLiteral("minimum")).toDouble(&minimumValid);
    const double maximum = candidate.value(QStringLiteral("maximum")).toDouble(&maximumValid);
    if (!minimumValid || !maximumValid || !std::isfinite(minimum) || !std::isfinite(maximum) ||
        (candidate.value(QStringLiteral("clip")).toBool() && minimum >= maximum))
    {
        return false;
    }
    const QString shape = candidate.value(QStringLiteral("headShape")).toString();
    if (shape != QStringLiteral("symbol") && shape != QStringLiteral("point") &&
        shape != QStringLiteral("sphere") && shape != QStringLiteral("cone"))
    {
        return false;
    }
    bool symbolValid = false;
    const int symbol = candidate.value(QStringLiteral("symbol")).toInt(&symbolValid);
    if (!symbolValid || symbol < -1 || symbol > 27 || candidate.value(QStringLiteral("symbol")).toDouble() != symbol)
    {
        return false;
    }
    if (candidate != m_settings)
    {
        m_settings = candidate;
        update();
    }
    return true;
}

QVector<IDOSWellPathPoint> IDOSWellRenderProvider::clippedPoints(double minimum, double maximum) const
{
    QVector<IDOSWellPathPoint> result;
    if (m_dataObject.isNull())
    {
        return result;
    }
    const QVector<IDOSWellPathPoint> points = m_dataObject->path().points();
    for (int index = 1; index < points.size(); ++index)
    {
        const IDOSWellPathPoint& first = points.at(index - 1);
        const IDOSWellPathPoint& second = points.at(index);
        if (second.md() <= first.md() || second.md() < minimum || first.md() > maximum)
        {
            continue;
        }
        const double start = qMax(first.md(), minimum);
        const double end = qMin(second.md(), maximum);
        if (start >= end)
        {
            continue;
        }
        const double depths[2] = {start, end};
        for (int endpoint = 0; endpoint < 2; ++endpoint)
        {
            const double depth = depths[endpoint];
            if (!result.isEmpty() && result.last().md() == depth)
            {
                continue;
            }
            const double fraction = (depth - first.md()) / (second.md() - first.md());
            result.append(IDOSWellPathPoint(depth,
                first.x() + fraction * (second.x() - first.x()),
                first.y() + fraction * (second.y() - first.y()),
                first.z() + fraction * (second.z() - first.z()),
                first.tvd() + fraction * (second.tvd() - first.tvd())));
        }
    }
    return result;
}

void IDOSWellRenderProvider::addPathActor(const QVector<IDOSWellPathPoint>& points, const QString& part)
{
    if (points.size() < 2)
    {
        return;
    }
    vtkSmartPointer<vtkPoints> coordinates = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
    for (const IDOSWellPathPoint& point : points)
    {
        ids->InsertNextId(coordinates->InsertNextPoint(point.x(), point.y(), point.z()));
    }
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    lines->InsertNextCell(ids);
    vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
    data->SetPoints(coordinates);
    data->SetLines(lines);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    if (m_settings.value(QStringLiteral("smooth")).toBool() && points.size() > 2)
    {
        vtkSmartPointer<vtkSplineFilter> spline = vtkSmartPointer<vtkSplineFilter>::New();
        spline->SetInputData(data);
        spline->SetSubdivideToSpecified();
        spline->SetNumberOfSubdivisions(qMin(10000, points.size() * 10));
        mapper->SetInputConnection(spline->GetOutputPort());
    }
    else
    {
        mapper->SetInputData(data);
    }
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    m_actors.append(actor);
    m_actorParts.append(part);
}

void IDOSWellRenderProvider::update()
{
    m_actors.clear();
    m_actorParts.clear();
    m_legends.clear();
    if (m_dataObject.isNull())
    {
        return;
    }
    const IDOSWellGeometryResolver resolver;
    const IDOSWellGeometryResolution resolution = resolver.resolve(m_dataObject->wellHead(), m_dataObject->path());
    if (resolution.hasStartPoint())
    {
        const QVector3D position = resolution.startPoint();
        const QString shape = m_settings.value(QStringLiteral("headShape")).toString();
        const double size = m_settings.value(QStringLiteral("headSize")).toDouble();
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        if (shape == QStringLiteral("symbol"))
        {
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
            points->SetDataTypeToDouble();
            vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
            const double center[3] = {position.x(), position.y(), position.z()};
            appendWellHeadSymbol(points, lines, center, size, effectiveSymbol());
            vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
            data->SetPoints(points);
            data->SetLines(lines);
            mapper->SetInputData(data);
        }
        else if (shape == QStringLiteral("sphere"))
        {
            vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
            source->SetCenter(position.x(), position.y(), position.z());
            source->SetRadius(size / 2.0);
            source->SetThetaResolution(24);
            source->SetPhiResolution(24);
            mapper->SetInputConnection(source->GetOutputPort());
        }
        else if (shape == QStringLiteral("cone"))
        {
            vtkSmartPointer<vtkConeSource> source = vtkSmartPointer<vtkConeSource>::New();
            source->SetCenter(position.x(), position.y(), position.z());
            source->SetDirection(0.0, 0.0, 1.0);
            source->SetHeight(size);
            source->SetRadius(size / 2.0);
            source->SetResolution(24);
            mapper->SetInputConnection(source->GetOutputPort());
        }
        else
        {
            vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
            const vtkIdType id = points->InsertNextPoint(position.x(), position.y(), position.z());
            vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
            vertices->InsertNextCell(1, &id);
            vtkSmartPointer<vtkPolyData> data = vtkSmartPointer<vtkPolyData>::New();
            data->SetPoints(points);
            data->SetVerts(vertices);
            mapper->SetInputData(data);
        }
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        m_actors.append(actor);
        m_actorParts.append(QStringLiteral("head"));
        if (m_settings.value(QStringLiteral("labelVisible")).toBool())
        {
            vtkSmartPointer<vtkBillboardTextActor3D> label = vtkSmartPointer<vtkBillboardTextActor3D>::New();
            label->SetInput(m_dataObject->name().toUtf8().constData());
            label->SetPosition(position.x(), position.y(), position.z());
            const QColor color(m_settings.value(QStringLiteral("labelColor")).toString());
            label->GetTextProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
            label->GetTextProperty()->SetFontSize(m_settings.value(QStringLiteral("labelSize")).toInt());
            label->GetTextProperty()->SetOpacity(m_opacity);
            m_legends.append(label);
        }
    }
    const double minimum = m_settings.value(QStringLiteral("clip")).toBool()
        ? m_settings.value(QStringLiteral("minimum")).toDouble() : -std::numeric_limits<double>::max();
    const double maximum = m_settings.value(QStringLiteral("clip")).toBool()
        ? m_settings.value(QStringLiteral("maximum")).toDouble() : std::numeric_limits<double>::max();
    addPathActor(clippedPoints(minimum, maximum), QStringLiteral("trajectory"));
    for (const IDOSWellCompletion& completion : m_dataObject->completions())
    {
        if (completion.k() == 0 && completion.top() < completion.bottom())
        {
            addPathActor(clippedPoints(qMax(minimum, completion.top()), qMin(maximum, completion.bottom())),
                completion.isOpen() ? QStringLiteral("completion") : QStringLiteral("closedCompletion"));
        }
    }
    applyActorProperties();
}

QList<vtkActor*> IDOSWellRenderProvider::actors() const
{
    QList<vtkActor*> result;
    for (const vtkSmartPointer<vtkActor>& actor : m_actors)
    {
        result.append(actor.GetPointer());
    }
    return result;
}

QList<vtkSmartPointer<vtkProp>> IDOSWellRenderProvider::legends() const
{
    return m_visible ? m_legends : QList<vtkSmartPointer<vtkProp>>();
}

void IDOSWellRenderProvider::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    applyActorProperties();
}

bool IDOSWellRenderProvider::highlighted() const
{
    return m_highlighted;
}

bool IDOSWellRenderProvider::visible() const
{
    return m_visible;
}

void IDOSWellRenderProvider::setVisible(bool visible)
{
    m_visible = visible;
    applyActorProperties();
}

double IDOSWellRenderProvider::opacity() const
{
    return m_opacity;
}

void IDOSWellRenderProvider::setOpacity(double opacity)
{
    m_opacity = qBound(0.0, opacity, 1.0);
    applyActorProperties();
}

void IDOSWellRenderProvider::applyActorProperties()
{
    for (int index = 0; index < m_actors.size(); ++index)
    {
        vtkActor* actor = m_actors.at(index).GetPointer();
        const QString actorPart = m_actorParts.at(index);
        const QString part = actorPart == QStringLiteral("closedCompletion") ? QStringLiteral("completion") : actorPart;
        const QColor color = actorColor(actorPart);
        actor->GetProperty()->SetColor(m_highlighted ? 1.0 : color.redF(),
            m_highlighted ? 0.95 : color.greenF(), m_highlighted ? 0.15 : color.blueF());
        actor->GetProperty()->SetPointSize(m_settings.value(QStringLiteral("headSize")).toDouble()
            + (m_highlighted ? 6.0 : 0.0));
        if (part != QStringLiteral("head"))
        {
            actor->GetProperty()->SetLineWidth(m_settings.value(part + QStringLiteral("Width")).toDouble()
                + (m_highlighted ? 2.5 : 0.0));
        }
        else
        {
            actor->GetProperty()->SetLineWidth(m_settings.value(QStringLiteral("symbolWidth")).toDouble()
                + (m_highlighted ? 2.5 : 0.0));
        }
        actor->GetProperty()->SetRenderLinesAsTubes(m_settings.value(QStringLiteral("tubes")).toBool());
        actor->GetProperty()->SetAmbient(0.85);
        actor->GetProperty()->SetDiffuse(0.35);
        actor->GetProperty()->SetSpecular(0.0);
        actor->GetProperty()->SetOpacity(m_opacity);
        actor->SetVisibility(m_visible && m_settings.value(part + QStringLiteral("Visible")).toBool());
    }
    for (const vtkSmartPointer<vtkProp>& legend : m_legends)
    {
        legend->SetVisibility(m_visible);
        vtkBillboardTextActor3D* label = vtkBillboardTextActor3D::SafeDownCast(legend);
        if (label != nullptr)
        {
            label->GetTextProperty()->SetOpacity(m_opacity);
        }
    }
}

void IDOSWellRenderProvider::appendLine(vtkPoints* points, vtkCellArray* lines,
    double ax, double ay, double az,
    double bx, double by, double bz)
{
    if (!points || !lines)
    {
        return;
    }
    vtkIdType ids[2] = {
        points->InsertNextPoint(ax, ay, az),
        points->InsertNextPoint(bx, by, bz)
    };
    lines->InsertNextCell(2, ids);
}

void IDOSWellRenderProvider::appendCircle(vtkPoints* points, vtkCellArray* lines,
    const double center[3], double radius, int segments)
{
    if (!points || !lines || radius <= 0.0)
    {
        return;
    }
    const int count = std::max(8, segments);
    vtkIdType firstId = -1;
    vtkIdType previousId = -1;
    for (int index = 0; index < count; ++index)
    {
        const double angle = 2.0 * std::acos(-1.0) * static_cast<double>(index) / static_cast<double>(count);
        const vtkIdType pointId = points->InsertNextPoint(
            center[0] + std::cos(angle) * radius,
            center[1] + std::sin(angle) * radius,
            center[2]);
        if (index == 0)
        {
            firstId = pointId;
        }
        if (previousId >= 0)
        {
            vtkIdType ids[2] = { previousId, pointId };
            lines->InsertNextCell(2, ids);
        }
        previousId = pointId;
    }
    if (firstId >= 0 && previousId >= 0)
    {
        vtkIdType ids[2] = { previousId, firstId };
        lines->InsertNextCell(2, ids);
    }
}

void IDOSWellRenderProvider::appendSquare(vtkPoints* points, vtkCellArray* lines,
    const double center[3], double radius, bool diamond)
{
    const double a = diamond ? std::acos(-1.0) / 4.0 : 0.0;
    double p[4][3] = {};
    for (int index = 0; index < 4; ++index)
    {
        const double angle = a + std::acos(-1.0) * 0.5 * static_cast<double>(index);
        p[index][0] = center[0] + std::cos(angle) * radius;
        p[index][1] = center[1] + std::sin(angle) * radius;
        p[index][2] = center[2];
    }
    for (int index = 0; index < 4; ++index)
    {
        const int next = (index + 1) % 4;
        appendLine(points, lines, p[index][0], p[index][1], p[index][2], p[next][0], p[next][1], p[next][2]);
    }
}

void IDOSWellRenderProvider::appendPlus(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius)
{
    appendLine(points, lines, center[0] - radius, center[1], center[2], center[0] + radius, center[1], center[2]);
    appendLine(points, lines, center[0], center[1] - radius, center[2], center[0], center[1] + radius, center[2]);
}

void IDOSWellRenderProvider::appendX(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius)
{
    const double r = radius * 0.72;
    appendLine(points, lines, center[0] - r, center[1] - r, center[2], center[0] + r, center[1] + r, center[2]);
    appendLine(points, lines, center[0] - r, center[1] + r, center[2], center[0] + r, center[1] - r, center[2]);
}

void IDOSWellRenderProvider::appendStar(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius)
{
    appendPlus(points, lines, center, radius);
    appendX(points, lines, center, radius);
}

void IDOSWellRenderProvider::appendSlash(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius)
{
    const double r = radius * 0.85;
    appendLine(points, lines, center[0] - r, center[1] - r, center[2], center[0] + r, center[1] + r, center[2]);
}

void IDOSWellRenderProvider::appendStem(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius)
{
    appendLine(points, lines, center[0], center[1] - radius, center[2], center[0], center[1] + radius, center[2]);
}

void IDOSWellRenderProvider::appendPlug(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius)
{
    appendStem(points, lines, center, radius);
    appendLine(points, lines, center[0] - radius * 0.45, center[1] - radius * 0.25, center[2],
        center[0] + radius * 0.45, center[1] - radius * 0.25, center[2]);
    appendLine(points, lines, center[0] - radius * 0.45, center[1] + radius * 0.25, center[2],
        center[0] + radius * 0.45, center[1] + radius * 0.25, center[2]);
}

void IDOSWellRenderProvider::appendCornerBox(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius)
{
    const double r = radius;
    const double s = radius * 0.42;
    appendLine(points, lines, center[0] - r, center[1] - r, center[2], center[0] - r + s, center[1] - r, center[2]);
    appendLine(points, lines, center[0] - r, center[1] - r, center[2], center[0] - r, center[1] - r + s, center[2]);
    appendLine(points, lines, center[0] + r, center[1] - r, center[2], center[0] + r - s, center[1] - r, center[2]);
    appendLine(points, lines, center[0] + r, center[1] - r, center[2], center[0] + r, center[1] - r + s, center[2]);
    appendLine(points, lines, center[0] - r, center[1] + r, center[2], center[0] - r + s, center[1] + r, center[2]);
    appendLine(points, lines, center[0] - r, center[1] + r, center[2], center[0] - r, center[1] + r - s, center[2]);
    appendLine(points, lines, center[0] + r, center[1] + r, center[2], center[0] + r - s, center[1] + r, center[2]);
    appendLine(points, lines, center[0] + r, center[1] + r, center[2], center[0] + r, center[1] + r - s, center[2]);
}

void IDOSWellRenderProvider::appendOilSymbol(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius, bool minor)
{
    appendCircle(points, lines, center, minor ? radius * 0.72 : radius, minor ? 18 : 24);
    appendCircle(points, lines, center, radius * 0.22, 10);
}

void IDOSWellRenderProvider::appendGasSymbol(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius, bool minor)
{
    appendStar(points, lines, center, minor ? radius * 0.72 : radius);
}

void IDOSWellRenderProvider::appendCondensateSymbol(vtkPoints* points, vtkCellArray* lines, const double center[3], double radius)
{
    appendCircle(points, lines, center, radius, 24);
    appendPlus(points, lines, center, radius * 0.62);
}

void IDOSWellRenderProvider::appendWellHeadSymbol(vtkPoints* points, vtkCellArray* lines,
    const double wellHead[3], double size, int symbol)
{
    double center[3] = { wellHead[0], wellHead[1], wellHead[2] };
    const double r = size * 0.5;

    switch (symbol)
    {
    case 1:
        appendCircle(points, lines, center, r, 24);
        appendStem(points, lines, center, r);
        break;
    case 2:
        appendSquare(points, lines, center, r, true);
        break;
    case 3:
        appendOilSymbol(points, lines, center, r);
        break;
    case 4:
        appendOilSymbol(points, lines, center, r, true);
        break;
    case 23:
        appendOilSymbol(points, lines, center, r * 0.78, true);
        appendCornerBox(points, lines, center, r);
        break;
    case 27:
        appendOilSymbol(points, lines, center, r);
        appendX(points, lines, center, r);
        appendPlug(points, lines, center, r * 0.75);
        break;
    case 5:
        appendGasSymbol(points, lines, center, r);
        break;
    case 6:
        appendGasSymbol(points, lines, center, r, true);
        break;
    case 7:
        appendCondensateSymbol(points, lines, center, r);
        break;
    case 8:
        appendSquare(points, lines, center, r, false);
        appendPlus(points, lines, center, r * 0.65);
        break;
    case 15:
        appendCircle(points, lines, center, r, 24);
        appendSlash(points, lines, center, r);
        break;
    case 16:
        appendGasSymbol(points, lines, center, r);
        appendSlash(points, lines, center, r);
        break;
    case 17:
        appendCircle(points, lines, center, r * 0.75, 18);
        appendLine(points, lines, center[0] - r, center[1], center[2], center[0] + r, center[1], center[2]);
        break;
    case 18:
        appendCircle(points, lines, center, r);
        appendPlus(points, lines, center, r);
        break;
    case 9:
        appendOilSymbol(points, lines, center, r);
        appendGasSymbol(points, lines, center, r * 0.65);
        appendX(points, lines, center, r);
        break;
    case 10:
        appendOilSymbol(points, lines, center, r);
        appendGasSymbol(points, lines, center, r * 0.48, true);
        appendX(points, lines, center, r);
        break;
    case 11:
        appendOilSymbol(points, lines, center, r);
        appendCondensateSymbol(points, lines, center, r * 0.62);
        appendX(points, lines, center, r);
        break;
    case 12:
        appendGasSymbol(points, lines, center, r);
        appendOilSymbol(points, lines, center, r * 0.52, true);
        appendX(points, lines, center, r);
        break;
    case 13:
        appendGasSymbol(points, lines, center, r);
        appendCondensateSymbol(points, lines, center, r * 0.55);
        appendX(points, lines, center, r);
        break;
    case 14:
        appendOilSymbol(points, lines, center, r * 0.72, true);
        appendGasSymbol(points, lines, center, r * 0.65, true);
        appendX(points, lines, center, r);
        break;
    case 19:
        appendCircle(points, lines, center, r, 24);
        appendPlus(points, lines, center, r * 0.7);
        appendX(points, lines, center, r);
        break;
    case 20:
        appendCircle(points, lines, center, r, 24);
        appendLine(points, lines, center[0] - r, center[1], center[2], center[0] + r, center[1], center[2]);
        appendSlash(points, lines, center, r);
        break;
    case 21:
        appendPlug(points, lines, center, r);
        appendX(points, lines, center, r);
        break;
    case 22:
        appendCircle(points, lines, center, r, 24);
        appendSlash(points, lines, center, r);
        break;
    case 24:
        appendCornerBox(points, lines, center, r);
        break;
    case 25:
        appendSquare(points, lines, center, r, false);
        appendCircle(points, lines, center, r * 0.45, 16);
        appendPlus(points, lines, center, r * 0.45);
        break;
    case 26:
        appendSquare(points, lines, center, r, true);
        appendX(points, lines, center, r);
        appendPlug(points, lines, center, r * 0.75);
        break;
    case 0:
    default:
        appendPlus(points, lines, center, r);
        appendStem(points, lines, center, r);
        break;
    }
}

int IDOSWellRenderProvider::effectiveSymbol() const
{
    const int symbol = m_settings.value(QStringLiteral("symbol")).toInt();
    if (symbol >= 0)
    {
        return symbol;
    }
    if (m_dataObject.isNull())
    {
        return 0;
    }
    if (m_dataObject->hasWellHead())
    {
        const int sourceSymbol = m_dataObject->wellHead().symbol();
        return sourceSymbol >= 0 && sourceSymbol <= 27 ? sourceSymbol : 0;
    }
    if (!m_dataObject->isOpen())
    {
        return 22;
    }
    return m_dataObject->type() == IDOSWell::Type::Injector ? 15 : 3;
}

QString IDOSWellRenderProvider::statusForSymbol(int symbol) const
{
    switch (symbol)
    {
        case 15:
        case 16:
            return QStringLiteral("injector");
        case 20:
        case 21:
        case 26:
        case 27:
            return QStringLiteral("stopped");
        case 2:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 19:
        case 22:
            return QStringLiteral("closed");
        default:
            return QStringLiteral("producer");
    }
}

QColor IDOSWellRenderProvider::actorColor(const QString& part) const
{
    const bool completion = part == QStringLiteral("completion") || part == QStringLiteral("closedCompletion");
    if (!m_settings.value(QStringLiteral("statusColors")).toBool())
    {
        return QColor(m_settings.value((completion ? QStringLiteral("completion") : part) + QStringLiteral("Color")).toString());
    }
    QString status = statusForSymbol(effectiveSymbol());
    if (part == QStringLiteral("closedCompletion") && status != QStringLiteral("stopped"))
    {
        status = QStringLiteral("closed");
    }
    const QString suffix = completion && !m_settings.value(QStringLiteral("sameCompletionColors")).toBool()
        ? QStringLiteral("CompletionColor") : QStringLiteral("Color");
    return QColor(m_settings.value(status + suffix).toString());
}

QIcon IDOSWellRenderProvider::symbolIcon(int symbol)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    const double center[3] = {0.0, 0.0, 0.0};
    appendWellHeadSymbol(points, lines, center, 14.0, symbol < 0 ? 0 : symbol);
    QPixmap pixmap(20, 20);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(QStringLiteral("#303030")), 1.2));
    vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();
    lines->InitTraversal();
    while (lines->GetNextCell(ids))
    {
        for (vtkIdType index = 1; index < ids->GetNumberOfIds(); ++index)
        {
            double first[3];
            double second[3];
            points->GetPoint(ids->GetId(index - 1), first);
            points->GetPoint(ids->GetId(index), second);
            painter.drawLine(QPointF(10.0 + first[0], 10.0 - first[1]), QPointF(10.0 + second[0], 10.0 - second[1]));
        }
    }
    painter.end();
    return QIcon(pixmap);
}
