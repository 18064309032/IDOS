#include <algorithm>
#include <cmath>
#include <limits>

#include <QSet>
#include <QVector>
#include <QtGlobal>

#ifdef IDOS_ENABLE_RENDER
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#endif

#include "data/seismic/idosseismicvolumeobject.h"

#include "idosseismicvolumerenderprovider.h"

IDOSSeismicVolumeRenderProvider::IDOSSeismicVolumeRenderProvider(IDOSSeismicVolumeObject* volume)
    :
    m_volume(volume)
    , m_actor()
    , m_inlineNumber(0)
    , m_visible(true)
    , m_highlighted(false)
    , m_opacity(1.0)
{
    const QList<int> availableInlines = inlineNumbers();
    if (!availableInlines.isEmpty())
    {
        m_inlineNumber = availableInlines.at(availableInlines.size() / 2);
    }
    update();
}

IDOSSeismicVolumeRenderProvider::~IDOSSeismicVolumeRenderProvider() = default;

IDOSSeismicVolumeObject* IDOSSeismicVolumeRenderProvider::volume() const
{
    return m_volume.data();
}

void IDOSSeismicVolumeRenderProvider::update()
{
#ifdef IDOS_ENABLE_RENDER
    m_actor = nullptr;
    if (m_volume.isNull())
    {
        return;
    }
    const QVector<qint64> sampleOffsets = m_volume->traceSampleOffsets();
    const QVector<int> sampleCounts = m_volume->traceSampleCounts();
    const QVector<int> inlines = m_volume->traceInlineNumbers();
    const QVector<int> crosslines = m_volume->traceCrosslineNumbers();
    const QVector<float> values = m_volume->sampleValues();
    if (sampleOffsets.isEmpty() || sampleOffsets.size() != sampleCounts.size() ||
        sampleOffsets.size() != inlines.size() || sampleOffsets.size() != crosslines.size() ||
        values.isEmpty())
    {
        return;
    }

    float maximumAmplitude = 0.0F;
    int traceCount = 0;
    for (int traceIndex = 0; traceIndex < inlines.size(); ++traceIndex)
    {
        if (inlines.at(traceIndex) != m_inlineNumber)
        {
            continue;
        }
        ++traceCount;
        const qint64 sampleOffset = sampleOffsets.at(traceIndex);
        const int sampleCount = sampleCounts.at(traceIndex);
        if (sampleOffset < 0 || sampleOffset + sampleCount > values.size())
        {
            return;
        }
        for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            const float amplitude = values.at(static_cast<int>(sampleOffset + sampleIndex));
            if (std::isfinite(amplitude))
            {
                maximumAmplitude = qMax(maximumAmplitude, std::abs(amplitude));
            }
        }
    }
    if (traceCount == 0)
    {
        return;
    }
    if (maximumAmplitude <= std::numeric_limits<float>::epsilon())
    {
        maximumAmplitude = 1.0F;
    }

    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkFloatArray> amplitudes = vtkSmartPointer<vtkFloatArray>::New();
    vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
    points->SetDataTypeToFloat();
    amplitudes->SetName("Amplitude");
    for (int traceIndex = 0; traceIndex < inlines.size(); ++traceIndex)
    {
        if (inlines.at(traceIndex) != m_inlineNumber)
        {
            continue;
        }
        const qint64 sampleOffset = sampleOffsets.at(traceIndex);
        const int sampleCount = sampleCounts.at(traceIndex);
        const vtkIdType firstPointId = points->GetNumberOfPoints();
        const double crosslinePosition = crosslines.at(traceIndex) - m_volume->crosslineMinimum();
        const double amplitudeScale = 0.45 / maximumAmplitude;
        for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            const float storedAmplitude = values.at(static_cast<int>(sampleOffset + sampleIndex));
            const float amplitude = std::isfinite(storedAmplitude) ? storedAmplitude : 0.0F;
            const double x = crosslinePosition + amplitude * amplitudeScale;
            const double y = 0.0;
            const double z = -static_cast<double>(sampleIndex) *
                             m_volume->sampleIntervalMicroseconds() / 1000.0;
            points->InsertNextPoint(x, y, z);
            amplitudes->InsertNextValue(amplitude);
        }
        lines->InsertNextCell(sampleCount);
        for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            lines->InsertCellPoint(firstPointId + sampleIndex);
        }
    }
    vtkSmartPointer<vtkPolyData> section = vtkSmartPointer<vtkPolyData>::New();
    section->SetPoints(points);
    section->SetLines(lines);
    section->GetPointData()->SetScalars(amplitudes);
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(section);
    mapper->SetScalarRange(section->GetScalarRange());
    mapper->ScalarVisibilityOn();
    m_actor = vtkSmartPointer<vtkActor>::New();
    m_actor->SetMapper(mapper);
    m_actor->GetProperty()->SetLineWidth(1.0F);
    applyActorProperties();
#endif
}

QList<vtkProp*> IDOSSeismicVolumeRenderProvider::props() const
{
    QList<vtkProp*> result;
#ifdef IDOS_ENABLE_RENDER
    if (m_actor != nullptr)
    {
        result.append(m_actor.GetPointer());
    }
#endif
    return result;
}


bool IDOSSeismicVolumeRenderProvider::isVisible() const
{
    return m_visible;
}

void IDOSSeismicVolumeRenderProvider::setVisible(bool visible)
{
    m_visible = visible;
    applyActorProperties();
}

bool IDOSSeismicVolumeRenderProvider::isSelected() const
{
    return m_highlighted;
}

void IDOSSeismicVolumeRenderProvider::setSelected(bool highlighted)
{
    m_highlighted = highlighted;
    applyActorProperties();
}

double IDOSSeismicVolumeRenderProvider::opacity() const
{
    return m_opacity;
}

void IDOSSeismicVolumeRenderProvider::setOpacity(double opacity)
{
    m_opacity = qBound(0.0, opacity, 1.0);
    applyActorProperties();
}

int IDOSSeismicVolumeRenderProvider::inlineNumber() const
{
    return m_inlineNumber;
}

bool IDOSSeismicVolumeRenderProvider::setInlineNumber(int inlineNumber)
{
    if (inlineNumber == m_inlineNumber)
    {
        return true;
    }
    if (!inlineNumbers().contains(inlineNumber))
    {
        return false;
    }
    m_inlineNumber = inlineNumber;
    update();
    return true;
}

QList<int> IDOSSeismicVolumeRenderProvider::inlineNumbers() const
{
    QList<int> result;
    if (m_volume.isNull())
    {
        return result;
    }
    QSet<int> uniqueNumbers;
    for (int inlineNumber : m_volume->traceInlineNumbers())
    {
        uniqueNumbers.insert(inlineNumber);
    }
    result = uniqueNumbers.values();
    std::sort(result.begin(), result.end());
    return result;
}

void IDOSSeismicVolumeRenderProvider::applyActorProperties()
{
#ifdef IDOS_ENABLE_RENDER
    if (m_actor == nullptr)
    {
        return;
    }
    m_actor->SetVisibility(m_visible ? 1 : 0);
    m_actor->GetProperty()->SetOpacity(m_opacity);
    m_actor->GetProperty()->SetEdgeVisibility(m_highlighted ? 1 : 0);
    m_actor->GetProperty()->SetEdgeColor(1.0, 0.85, 0.0);
#endif
}

QVariantMap IDOSSeismicVolumeRenderProvider::saveState() const
{
    QVariantMap state;
    state.insert(QStringLiteral("visible"), isVisible());
    state.insert(QStringLiteral("highlighted"), isSelected());
    state.insert(QStringLiteral("opacity"), opacity());
    state.insert(QStringLiteral("inlineNumber"), inlineNumber());
    return state;
}

bool IDOSSeismicVolumeRenderProvider::restoreState(const QVariantMap& state)
{
    const int number = state.value(QStringLiteral("inlineNumber")).toInt();
    if (!inlineNumbers().isEmpty() && !setInlineNumber(number))
    {
        return false;
    }
    bool validOpacity = false;
    const double opacityValue = state.value(QStringLiteral("opacity")).toDouble(&validOpacity);
    if (!validOpacity || !(opacityValue >= 0.0 && opacityValue <= 1.0) ||
        state.value(QStringLiteral("visible")).type() != QVariant::Bool)
    {
        return false;
    }
    setVisible(state.value(QStringLiteral("visible")).toBool());
    setSelected(state.value(QStringLiteral("highlighted")).toBool());
    setOpacity(opacityValue);
    return true;
}
