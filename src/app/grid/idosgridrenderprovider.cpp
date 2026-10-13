#include <cmath>
#include <limits>

#include <QColor>
#include <QList>
#include <QObject>
#include <QtGlobal>
#include <QVector>
#include <QVector3D>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkDataSetMapper.h>
#include <vtkDoubleArray.h>
#include <vtkHexahedron.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkScalarBarActor.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>
#include <vtkUnstructuredGrid.h>
#endif

#include "idosgrid.h"
#include "idosgridfilter.h"
#include "idosgridproperty.h"

#include "idosgridrenderprovider.h"

IDOSGridRenderProvider::IDOSGridRenderProvider(IDOSGrid* dataObject)
    :
    m_dataObject(dataObject)
    , m_property(nullptr)
    , m_displayMode(IDOSDisplayMode::Surface)
    , m_edgeColor(Qt::black)
    , m_edgeWidth(1.0)
    , m_actors()
    , m_legends()
    , m_highlighted(false)
    , m_visible(dataObject != nullptr && dataObject->isVisible())
    , m_opacity(1.0)
    , m_coloringSettings()
    , m_filterCriteria()
    , m_previewFilterId()
    , m_filterProperties()
    , m_filterError()
    , m_filteredCellCount(0)
{
    m_coloringSettings.insert(QStringLiteral("ramp"), QStringLiteral("Default"));
    m_coloringSettings.insert(QStringLiteral("reversed"), false);
    m_coloringSettings.insert(QStringLiteral("automatic"), true);
    m_coloringSettings.insert(QStringLiteral("minimum"), 0.0);
    m_coloringSettings.insert(QStringLiteral("maximum"), 1.0);
    m_coloringSettings.insert(QStringLiteral("solidColor"), QStringLiteral("#59b8ff"));
    m_coloringSettings.insert(QStringLiteral("invalidColor"), QStringLiteral("#808080"));
    m_coloringSettings.insert(QStringLiteral("legend"), true);
}

IDOSGridRenderProvider::~IDOSGridRenderProvider()
{
}

IDOSDataObject* IDOSGridRenderProvider::dataSource() const
{
    return m_dataObject.data();
}

bool IDOSGridRenderProvider::dependsOn(const QStringList& dataIds) const
{
    for (const QString& id : dependencyIds())
    {
        if (dataIds.contains(id))
        {
            return true;
        }
    }
    return false;
}

bool IDOSGridRenderProvider::removeDataReference(const QString& dataId)
{
    bool removed = m_filterProperties.remove(dataId) > 0;
    if (!m_previewFilterId.isEmpty() && m_previewFilterId == dataId)
    {
        setFilterCriteria(QVariantMap());
        removed = true;
    }
    if (m_property != nullptr && m_property->objectId() == dataId)
    {
        m_property = nullptr;
        removed = true;
    }
    return removed;
}

void IDOSGridRenderProvider::update()
{
    m_actors.clear();
    m_legends.clear();
    m_filterError.clear();
    m_filteredCellCount = 0;
    if (m_dataObject.isNull())
    {
        return;
    }
    const QVariantMap axes = m_filterCriteria.value(QStringLiteral("axes")).toMap();
    const QStringList names = {QStringLiteral("i"), QStringLiteral("j"), QStringLiteral("k")};
    const QList<int> dimensions = {m_dataObject->nx(), m_dataObject->ny(), m_dataObject->nz()};
    for (int axis = 0; axis < 3; ++axis)
    {
        const QVariantMap range = axes.value(names.at(axis)).toMap();
        bool invalid = range.value(QStringLiteral("enabled")).toBool() && range.value(QStringLiteral("maximum")).toInt() > dimensions.at(axis);
        for (const QVariant& value : range.value(QStringLiteral("values")).toList())
        {
            invalid = invalid || value.toInt() > dimensions.at(axis);
        }
        if (invalid)
        {
            m_filterError = QObject::tr("A filter index range exceeds the source grid dimensions.");
            return;
        }
    }
    QMap<QString, IDOSGridProperty*> filterProperties;
    for (const QString& id : IDOSGridFilter::propertyIds(m_filterCriteria))
    {
        IDOSGridProperty* property = m_filterProperties.value(id).data();
        const qint64 cellCount = static_cast<qint64>(m_dataObject->nx()) * m_dataObject->ny() * m_dataObject->nz();
        if (property == nullptr || property->gridId() != m_dataObject->objectId() || property->values().size() != cellCount || property->numX() != m_dataObject->nx() ||
            property->numY() != m_dataObject->ny() || property->numZ() != m_dataObject->nz())
        {
            m_filterError = QObject::tr("A filter property is missing or incompatible with the source grid.");
            return;
        }
        filterProperties.insert(id, property);
    }
#ifdef IDOS_ENABLE_RENDER
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkUnstructuredGrid> grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    grid->SetPoints(points);
    vtkSmartPointer<vtkIntArray> originalIds = vtkSmartPointer<vtkIntArray>::New();
    originalIds->SetName("GlobalIndex");
    vtkSmartPointer<vtkIntArray> indicesI = vtkSmartPointer<vtkIntArray>::New();
    indicesI->SetName("I");
    vtkSmartPointer<vtkIntArray> indicesJ = vtkSmartPointer<vtkIntArray>::New();
    indicesJ->SetName("J");
    vtkSmartPointer<vtkIntArray> indicesK = vtkSmartPointer<vtkIntArray>::New();
    indicesK->SetName("K");
    grid->GetCellData()->AddArray(originalIds);
    grid->GetCellData()->AddArray(indicesI);
    grid->GetCellData()->AddArray(indicesJ);
    grid->GetCellData()->AddArray(indicesK);
    QVector<double> values;
    const IDOSGrid* sourceGrid = m_dataObject.data();
    const QVector<quint8> selection = IDOSGridFilter::selection(m_filterCriteria, sourceGrid->nx(), sourceGrid->ny(), sourceGrid->nz(), filterProperties);
    const bool hasScalars = !m_property.isNull() && !m_property->values().isEmpty();
    for (int k = 0; k < sourceGrid->nz(); ++k)
    {
        for (int j = 0; j < sourceGrid->ny(); ++j)
        {
            for (int i = 0; i < sourceGrid->nx(); ++i)
            {
                const int globalIndex = IDOSGridProperty::cellIndex(i, j, k, sourceGrid->nx(), sourceGrid->ny());
                if (!sourceGrid->isActive(i, j, k) || selection.at(globalIndex) == 0)
                {
                    continue;
                }
                vtkIdType pointIds[8];
                for (int corner = 0; corner < 8; ++corner)
                {
                    const QVector3D point = sourceGrid->cornerPosition(i, j, k, corner);
                    pointIds[corner] = points->InsertNextPoint(point.x(), point.y(), point.z());
                }
                grid->InsertNextCell(VTK_HEXAHEDRON, 8, pointIds);
                originalIds->InsertNextValue(globalIndex);
                indicesI->InsertNextValue(i + 1);
                indicesJ->InsertNextValue(j + 1);
                indicesK->InsertNextValue(k + 1);
                ++m_filteredCellCount;
                if (hasScalars)
                {
                    values.append(globalIndex < m_property->values().size() ? m_property->values().at(globalIndex)
                                                                            : std::numeric_limits<double>::quiet_NaN());
                }
            }
        }
    }
    const bool hasCellScalars = hasScalars && !values.isEmpty();
    vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputData(grid);
    if (!hasCellScalars)
    {
        mapper->ScalarVisibilityOff();
    }
    else
    {
        vtkSmartPointer<vtkDoubleArray> scalars = vtkSmartPointer<vtkDoubleArray>::New();
        scalars->SetName(m_property->name().toUtf8().constData());
        double minimumValue = std::numeric_limits<double>::infinity();
        double maximumValue = -std::numeric_limits<double>::infinity();
        for (int index = 0; index < values.size(); ++index)
        {
            const double value = values.at(index);
            scalars->InsertNextValue(std::isfinite(value) ? value : std::numeric_limits<double>::quiet_NaN());
            if (std::isfinite(value))
            {
                minimumValue = qMin(minimumValue, value);
                maximumValue = qMax(maximumValue, value);
            }
        }
        if (!std::isfinite(minimumValue))
        {
            minimumValue = 0.0;
            maximumValue = 1.0;
        }
        if (!m_coloringSettings.value(QStringLiteral("automatic")).toBool())
        {
            minimumValue = m_coloringSettings.value(QStringLiteral("minimum")).toDouble();
            maximumValue = m_coloringSettings.value(QStringLiteral("maximum")).toDouble();
        }
        if (minimumValue == maximumValue)
        {
            const double padding = qMax(1.0, std::abs(minimumValue) * 0.01);
            minimumValue -= padding;
            maximumValue += padding;
        }
        if (m_coloringSettings.value(QStringLiteral("automatic")).toBool())
        {
            m_coloringSettings.insert(QStringLiteral("minimum"), minimumValue);
            m_coloringSettings.insert(QStringLiteral("maximum"), maximumValue);
        }
        grid->GetCellData()->SetScalars(scalars);
        vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
        lookupTable->SetHueRange(0.667, 0.0);
        lookupTable->SetSaturationRange(0.85, 0.85);
        lookupTable->SetValueRange(0.95, 0.95);
        lookupTable->SetRange(minimumValue, maximumValue);
        lookupTable->Build();
        lookupTable->SetNumberOfTableValues(256);
        const QString ramp = m_coloringSettings.value(QStringLiteral("ramp")).toString();
        const bool reversed = m_coloringSettings.value(QStringLiteral("reversed")).toBool();
        const QList<QColor> colors = colorRampColors(ramp, reversed);
        for (int index = 0; index < 256; ++index)
        {
            const QColor color = colors.at(index);
            lookupTable->SetTableValue(index, color.redF(), color.greenF(), color.blueF(), 1.0);
        }
        const QColor invalidColor(m_coloringSettings.value(QStringLiteral("invalidColor")).toString());
        lookupTable->SetNanColor(invalidColor.redF(), invalidColor.greenF(), invalidColor.blueF(), 1.0);
        mapper->SetLookupTable(lookupTable);
        mapper->SetScalarRange(minimumValue, maximumValue);
        mapper->SetScalarModeToUseCellData();
        mapper->SetInterpolateScalarsBeforeMapping(false);
        mapper->ScalarVisibilityOn();
    }

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetOpacity(m_opacity);
    actor->GetProperty()->SetInterpolationToFlat();
    actor->GetProperty()->SetLineWidth(1.0);
    m_actors.append(actor);
    if (hasCellScalars && m_coloringSettings.value(QStringLiteral("legend")).toBool())
    {
        vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
        scalarBar->SetLookupTable(mapper->GetLookupTable());
        scalarBar->SetTitle(m_property->name().toUtf8().constData());
        scalarBar->SetNumberOfLabels(5);
        scalarBar->SetPosition(0.92, 0.32);
        scalarBar->SetWidth(0.05);
        scalarBar->SetHeight(0.36);
        scalarBar->SetUnconstrainedFontSize(true);
        scalarBar->GetTitleTextProperty()->SetColor(1.0, 1.0, 1.0);
        scalarBar->GetTitleTextProperty()->SetFontSize(18);
        scalarBar->GetLabelTextProperty()->SetColor(1.0, 1.0, 1.0);
        scalarBar->GetLabelTextProperty()->SetFontSize(14);
        m_legends.append(scalarBar);
    }
#endif
    applyActorProperties();
}

QList<vtkActor*> IDOSGridRenderProvider::actors() const
{
    QList<vtkActor*> result;
#ifdef IDOS_ENABLE_RENDER
    for (int index = 0; index < m_actors.size(); ++index)
    {
        result.append(m_actors.at(index).GetPointer());
    }
#endif
    return result;
}

QList<vtkSmartPointer<vtkProp>> IDOSGridRenderProvider::legends() const
{
    return m_visible ? m_legends : QList<vtkSmartPointer<vtkProp>>();
}

void IDOSGridRenderProvider::setHighlighted(bool highlighted)
{
    m_highlighted = highlighted;
    applyActorProperties();
}

bool IDOSGridRenderProvider::visible() const
{
    return m_visible;
}

void IDOSGridRenderProvider::setVisible(bool visible)
{
    m_visible = visible;
    applyActorProperties();
}

double IDOSGridRenderProvider::opacity() const
{
    return m_opacity;
}

void IDOSGridRenderProvider::setOpacity(double opacity)
{
    m_opacity = qBound(0.0, opacity, 1.0);
    applyActorProperties();
}

void IDOSGridRenderProvider::setProperty(IDOSGridProperty* property)
{
    m_property = property;
    update();
}

IDOSGridProperty* IDOSGridRenderProvider::property() const
{
    return m_property.data();
}

QVariantMap IDOSGridRenderProvider::coloringSettings() const
{
    return m_coloringSettings;
}

bool IDOSGridRenderProvider::setColoringSettings(const QVariantMap& settings)
{
    const QString ramp = settings.value(QStringLiteral("ramp")).toString();
    bool minimumValid = false;
    bool maximumValid = false;
    const double minimum = settings.value(QStringLiteral("minimum")).toDouble(&minimumValid);
    const double maximum = settings.value(QStringLiteral("maximum")).toDouble(&maximumValid);
    if (!colorRampKeys().contains(ramp) || !minimumValid || !maximumValid ||
        !std::isfinite(minimum) || !std::isfinite(maximum) ||
        (!settings.value(QStringLiteral("automatic")).toBool() && minimum >= maximum) ||
        !QColor(settings.value(QStringLiteral("solidColor")).toString()).isValid() ||
        !QColor(settings.value(QStringLiteral("invalidColor")).toString()).isValid())
    {
        return false;
    }
    m_coloringSettings = settings;
    update();
    return true;
}

IDOSDisplayMode IDOSGridRenderProvider::displayMode() const
{
    return m_displayMode;
}

bool IDOSGridRenderProvider::setDisplayMode(IDOSDisplayMode mode)
{
    if (mode != IDOSDisplayMode::Surface && mode != IDOSDisplayMode::Wireframe &&
        mode != IDOSDisplayMode::SurfaceWithEdges && mode != IDOSDisplayMode::Points)
    {
        return false;
    }
    m_displayMode = mode;
    applyActorProperties();
    return true;
}

QColor IDOSGridRenderProvider::edgeColor() const
{
    return m_edgeColor;
}

bool IDOSGridRenderProvider::setEdgeColor(const QColor& color)
{
    if (!color.isValid())
    {
        return false;
    }
    m_edgeColor = color;
    applyActorProperties();
    return true;
}

double IDOSGridRenderProvider::edgeWidth() const
{
    return m_edgeWidth;
}

bool IDOSGridRenderProvider::setEdgeWidth(double width)
{
    if (!std::isfinite(width) || width < 0.5 || width > 20.0)
    {
        return false;
    }
    m_edgeWidth = width;
    applyActorProperties();
    return true;
}

bool IDOSGridRenderProvider::highlighted() const
{
    return m_highlighted;
}

void IDOSGridRenderProvider::applyActorProperties()
{
#ifdef IDOS_ENABLE_RENDER
    for (int index = 0; index < m_actors.size(); ++index)
    {
        vtkActor* actor = m_actors.at(index).GetPointer();
        switch (m_displayMode)
        {
        case IDOSDisplayMode::Surface:
            actor->GetProperty()->SetRepresentationToSurface();
            actor->GetProperty()->EdgeVisibilityOff();
            break;
        case IDOSDisplayMode::Wireframe:
            actor->GetProperty()->SetRepresentationToWireframe();
            actor->GetProperty()->EdgeVisibilityOn();
            break;
        case IDOSDisplayMode::SurfaceWithEdges:
            actor->GetProperty()->SetRepresentationToSurface();
            actor->GetProperty()->EdgeVisibilityOn();
            break;
        case IDOSDisplayMode::Points:
            actor->GetProperty()->SetRepresentationToPoints();
            actor->GetProperty()->EdgeVisibilityOff();
            break;
        }
        if (actor->GetMapper()->GetScalarVisibility() == 0)
        {
            const QColor color(m_coloringSettings.value(QStringLiteral("solidColor")).toString());
            actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
        }
        actor->GetProperty()->SetEdgeColor(m_highlighted ? 1.0 : m_edgeColor.redF(),
                                           m_highlighted ? 0.95 : m_edgeColor.greenF(),
                                           m_highlighted ? 0.15 : m_edgeColor.blueF());
        actor->GetProperty()->SetLineWidth(m_highlighted ? qMax(3.0, m_edgeWidth + 2.0) : m_edgeWidth);
        if (m_highlighted && m_displayMode != IDOSDisplayMode::Points)
        {
            actor->GetProperty()->EdgeVisibilityOn();
        }
        actor->GetProperty()->SetOpacity(m_opacity);
        actor->SetVisibility(m_visible);
    }
    for (int index = 0; index < m_legends.size(); ++index)
    {
        m_legends.at(index)->SetVisibility(m_visible);
    }
#endif
}

QList<QColor> IDOSGridRenderProvider::colorRampColors(const QString& key, bool reversed)
{
    const QList<QColor> stops = colorRampStops(key);
    QList<QColor> colors;
    for (int index = 0; index < 256; ++index)
    {
        const double fraction = static_cast<double>(reversed ? 255 - index : index) / 255.0;
        QColor color;
        if (key == QStringLiteral("gray"))
        {
            color = QColor::fromRgbF(fraction, fraction, fraction);
        }
        else if (key == QStringLiteral("blueYellow"))
        {
            color = QColor::fromRgbF(0.1 + 0.85 * fraction, 0.2 + 0.7 * fraction, 0.8 - 0.7 * fraction);
        }
        else if (stops.size() >= 2)
        {
            const double position = fraction * (stops.size() - 1);
            const int first = qMin(static_cast<int>(position), stops.size() - 2);
            const double weight = position - first;
            const QColor left = stops.at(first);
            const QColor right = stops.at(first + 1);
            color = QColor::fromRgbF(left.redF() * (1.0 - weight) + right.redF() * weight,
                                    left.greenF() * (1.0 - weight) + right.greenF() * weight,
                                    left.blueF() * (1.0 - weight) + right.blueF() * weight);
        }
        else
        {
            color = QColor::fromHsvF((1.0 - fraction) * 0.667, 0.85, 0.95);
        }
        colors.append(color);
    }
    return colors;
}

QStringList IDOSGridRenderProvider::colorRampKeys()
{
    return {
        QStringLiteral("Default"),
        QStringLiteral("SoftRainbow"),
        QStringLiteral("OceanBreeze"),
        QStringLiteral("WarmAmber"),
        QStringLiteral("FreshMint"),
        QStringLiteral("TwilightPurple"),
        QStringLiteral("EarthTones"),
        QStringLiteral("GlacierBlue"),
        QStringLiteral("WarmColdDiverging"),
        QStringLiteral("LavaMetal"),
        QStringLiteral("DarkForest"),
        QStringLiteral("VintageSunset"),
        QStringLiteral("Aurora"),
        QStringLiteral("MineralSand"),
        QStringLiteral("Porcelain"),
        QStringLiteral("AutumnBerry"),
        QStringLiteral("MonochromeJade"),
        QStringLiteral("GraphiteScale"),
        QStringLiteral("Cyberpunk"),
        QStringLiteral("blueRed"),
        QStringLiteral("blueYellow"),
        QStringLiteral("gray")};
}

QList<QColor> IDOSGridRenderProvider::colorRampStops(const QString& key)
{
    if (key == QStringLiteral("Default"))
    {
        return {
            QColor::fromRgbF(0.0, 0.0, 0.5),
            QColor::fromRgbF(0.0, 0.0, 1.0),
            QColor::fromRgbF(0.0, 1.0, 1.0),
            QColor::fromRgbF(0.0, 0.85, 0.25),
            QColor::fromRgbF(1.0, 1.0, 0.0),
            QColor::fromRgbF(1.0, 0.45, 0.0),
            QColor::fromRgbF(1.0, 0.0, 0.0),
            QColor::fromRgbF(0.5, 0.0, 0.0)};
    }
    if (key == QStringLiteral("SoftRainbow"))
    {
        return {
            QColor::fromRgbF(0.40, 0.55, 0.85),
            QColor::fromRgbF(0.35, 0.70, 0.80),
            QColor::fromRgbF(0.45, 0.75, 0.55),
            QColor::fromRgbF(0.80, 0.80, 0.45),
            QColor::fromRgbF(0.90, 0.65, 0.40),
            QColor::fromRgbF(0.85, 0.45, 0.45)};
    }
    if (key == QStringLiteral("OceanBreeze"))
    {
        return {
            QColor::fromRgbF(0.15, 0.25, 0.45),
            QColor::fromRgbF(0.20, 0.45, 0.65),
            QColor::fromRgbF(0.35, 0.65, 0.75),
            QColor::fromRgbF(0.60, 0.80, 0.80),
            QColor::fromRgbF(0.85, 0.90, 0.85),
            QColor::fromRgbF(0.95, 0.95, 0.90)};
    }
    if (key == QStringLiteral("WarmAmber"))
    {
        return {
            QColor::fromRgbF(0.98, 0.95, 0.85),
            QColor::fromRgbF(0.95, 0.85, 0.65),
            QColor::fromRgbF(0.90, 0.70, 0.45),
            QColor::fromRgbF(0.85, 0.55, 0.30),
            QColor::fromRgbF(0.70, 0.40, 0.20),
            QColor::fromRgbF(0.50, 0.25, 0.10)};
    }
    if (key == QStringLiteral("FreshMint"))
    {
        return {
            QColor::fromRgbF(0.85, 0.95, 0.85),
            QColor::fromRgbF(0.70, 0.90, 0.85),
            QColor::fromRgbF(0.55, 0.85, 0.85),
            QColor::fromRgbF(0.40, 0.75, 0.80),
            QColor::fromRgbF(0.25, 0.60, 0.70),
            QColor::fromRgbF(0.15, 0.40, 0.55)};
    }
    if (key == QStringLiteral("TwilightPurple"))
    {
        return {
            QColor::fromRgbF(0.90, 0.85, 0.95),
            QColor::fromRgbF(0.80, 0.70, 0.90),
            QColor::fromRgbF(0.70, 0.55, 0.85),
            QColor::fromRgbF(0.60, 0.40, 0.75),
            QColor::fromRgbF(0.45, 0.25, 0.60),
            QColor::fromRgbF(0.30, 0.15, 0.45)};
    }
    if (key == QStringLiteral("EarthTones"))
    {
        return {
            QColor::fromRgbF(0.95, 0.90, 0.80),
            QColor::fromRgbF(0.85, 0.75, 0.60),
            QColor::fromRgbF(0.75, 0.60, 0.45),
            QColor::fromRgbF(0.65, 0.45, 0.30),
            QColor::fromRgbF(0.50, 0.30, 0.20),
            QColor::fromRgbF(0.35, 0.18, 0.10)};
    }
    if (key == QStringLiteral("GlacierBlue"))
    {
        return {
            QColor::fromRgbF(0.95, 0.97, 1.00),
            QColor::fromRgbF(0.80, 0.90, 0.98),
            QColor::fromRgbF(0.60, 0.80, 0.95),
            QColor::fromRgbF(0.40, 0.65, 0.90),
            QColor::fromRgbF(0.20, 0.45, 0.80),
            QColor::fromRgbF(0.10, 0.25, 0.60)};
    }
    if (key == QStringLiteral("WarmColdDiverging"))
    {
        return {
            QColor::fromRgbF(0.85, 0.45, 0.30),
            QColor::fromRgbF(0.95, 0.75, 0.55),
            QColor::fromRgbF(0.98, 0.95, 0.90),
            QColor::fromRgbF(0.75, 0.85, 0.90),
            QColor::fromRgbF(0.45, 0.65, 0.80),
            QColor::fromRgbF(0.20, 0.40, 0.65)};
    }
    if (key == QStringLiteral("LavaMetal"))
    {
        return {
            QColor::fromRgbF(0.35, 0.10, 0.05),
            QColor::fromRgbF(0.60, 0.15, 0.10),
            QColor::fromRgbF(0.85, 0.30, 0.15),
            QColor::fromRgbF(0.98, 0.55, 0.20),
            QColor::fromRgbF(1.00, 0.80, 0.30),
            QColor::fromRgbF(1.00, 0.95, 0.60)};
    }
    if (key == QStringLiteral("DarkForest"))
    {
        return {
            QColor::fromRgbF(0.10, 0.20, 0.10),
            QColor::fromRgbF(0.15, 0.35, 0.20),
            QColor::fromRgbF(0.25, 0.50, 0.35),
            QColor::fromRgbF(0.45, 0.65, 0.55),
            QColor::fromRgbF(0.65, 0.75, 0.65),
            QColor::fromRgbF(0.85, 0.88, 0.82)};
    }
    if (key == QStringLiteral("VintageSunset"))
    {
        return {
            QColor::fromRgbF(0.45, 0.25, 0.45),
            QColor::fromRgbF(0.70, 0.35, 0.50),
            QColor::fromRgbF(0.90, 0.55, 0.55),
            QColor::fromRgbF(0.98, 0.75, 0.55),
            QColor::fromRgbF(0.98, 0.88, 0.65),
            QColor::fromRgbF(0.95, 0.95, 0.80)};
    }
    if (key == QStringLiteral("Aurora"))
    {
        return {
            QColor::fromRgbF(0.20, 0.10, 0.35),
            QColor::fromRgbF(0.35, 0.25, 0.60),
            QColor::fromRgbF(0.40, 0.60, 0.75),
            QColor::fromRgbF(0.50, 0.85, 0.80),
            QColor::fromRgbF(0.70, 0.95, 0.85),
            QColor::fromRgbF(0.90, 0.98, 0.95)};
    }
    if (key == QStringLiteral("MineralSand"))
    {
        return {
            QColor::fromRgbF(0.25, 0.18, 0.12),
            QColor::fromRgbF(0.45, 0.30, 0.20),
            QColor::fromRgbF(0.65, 0.45, 0.30),
            QColor::fromRgbF(0.80, 0.60, 0.45),
            QColor::fromRgbF(0.90, 0.75, 0.60),
            QColor::fromRgbF(0.96, 0.88, 0.78)};
    }
    if (key == QStringLiteral("Porcelain"))
    {
        return {
            QColor::fromRgbF(0.98, 0.97, 0.94),
            QColor::fromRgbF(0.85, 0.90, 0.92),
            QColor::fromRgbF(0.60, 0.75, 0.85),
            QColor::fromRgbF(0.35, 0.55, 0.75),
            QColor::fromRgbF(0.15, 0.35, 0.55),
            QColor::fromRgbF(0.05, 0.15, 0.30)};
    }
    if (key == QStringLiteral("AutumnBerry"))
    {
        return {
            QColor::fromRgbF(0.30, 0.10, 0.20),
            QColor::fromRgbF(0.55, 0.15, 0.25),
            QColor::fromRgbF(0.80, 0.30, 0.25),
            QColor::fromRgbF(0.95, 0.50, 0.30),
            QColor::fromRgbF(0.98, 0.70, 0.35),
            QColor::fromRgbF(0.95, 0.85, 0.50)};
    }
    if (key == QStringLiteral("MonochromeJade"))
    {
        return {
            QColor::fromRgbF(0.05, 0.25, 0.15),
            QColor::fromRgbF(0.15, 0.45, 0.30),
            QColor::fromRgbF(0.30, 0.65, 0.45),
            QColor::fromRgbF(0.55, 0.80, 0.60),
            QColor::fromRgbF(0.80, 0.92, 0.80),
            QColor::fromRgbF(0.95, 0.98, 0.92)};
    }
    if (key == QStringLiteral("GraphiteScale"))
    {
        return {
            QColor::fromRgbF(0.10, 0.10, 0.10),
            QColor::fromRgbF(0.25, 0.25, 0.25),
            QColor::fromRgbF(0.45, 0.45, 0.45),
            QColor::fromRgbF(0.65, 0.65, 0.65),
            QColor::fromRgbF(0.85, 0.85, 0.85),
            QColor::fromRgbF(0.98, 0.98, 0.98)};
    }
    if (key == QStringLiteral("Cyberpunk"))
    {
        return {
            QColor::fromRgbF(0.40, 0.00, 0.40),
            QColor::fromRgbF(0.80, 0.10, 0.60),
            QColor::fromRgbF(0.60, 0.20, 0.80),
            QColor::fromRgbF(0.10, 0.50, 0.90),
            QColor::fromRgbF(0.00, 0.80, 0.80),
            QColor::fromRgbF(0.70, 0.90, 0.90)};
    }
    return {};
}

QStringList IDOSGridRenderProvider::dependencyIds() const
{
    QStringList ids = IDOSGridFilter::propertyIds(m_filterCriteria);
    if (!m_previewFilterId.isEmpty())
    {
        ids.append(m_previewFilterId);
    }
    if (m_property != nullptr && !ids.contains(m_property->objectId()))
    {
        ids.append(m_property->objectId());
    }
    return ids;
}

void IDOSGridRenderProvider::setDataReference(IDOSDataObject* data)
{
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
    if (property != nullptr)
    {
        m_filterProperties.insert(property->objectId(), property);
    }
}

QVariantMap IDOSGridRenderProvider::filterCriteria() const
{
    return m_filterCriteria;
}

bool IDOSGridRenderProvider::setFilterCriteria(const QVariantMap& criteria)
{
    QString error;
    if (!IDOSGridFilter::validateCriteria(criteria, error))
    {
        return false;
    }
    m_filterCriteria = criteria;
    m_previewFilterId.clear();
    return true;
}

QString IDOSGridRenderProvider::filterError() const
{
    return m_filterError;
}

int IDOSGridRenderProvider::filteredCellCount() const
{
    return m_filteredCellCount;
}

void IDOSGridRenderProvider::setGridSource(IDOSGrid* grid)
{
    m_dataObject = grid;
}

void IDOSGridRenderProvider::setPropertySource(IDOSGridProperty* property)
{
    m_property = property;
}

QString IDOSGridRenderProvider::previewFilterId() const
{
    return m_previewFilterId;
}

void IDOSGridRenderProvider::setPreviewFilterId(const QString& id)
{
    m_previewFilterId = id;
}
