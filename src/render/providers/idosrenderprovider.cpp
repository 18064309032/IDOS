#include <QtGlobal>

#include "idosrenderprovider.h"

IDOSRenderProvider::~IDOSRenderProvider()
{
}

IDOSDisplayMode IDOSRenderProvider::displayMode() const
{
    return IDOSDisplayMode::Surface;
}

bool IDOSRenderProvider::setDisplayMode(IDOSDisplayMode mode)
{
    Q_UNUSED(mode)
    return false;
}

bool IDOSRenderProvider::dependsOn(const QStringList& dataIds) const
{
    Q_UNUSED(dataIds)
    return false;
}

bool IDOSRenderProvider::removeDataReference(const QString& dataId)
{
    Q_UNUSED(dataId)
    return false;
}

QStringList IDOSRenderProvider::dependencyIds() const
{
    return {};
}

void IDOSRenderProvider::setDataReference(IDOSDataObject* data)
{
    Q_UNUSED(data)
}

QVariantMap IDOSRenderProvider::presentationState() const
{
    QVariantMap state;
    state.insert(QStringLiteral("visible"), visible());
    state.insert(QStringLiteral("opacity"), opacity());
    state.insert(QStringLiteral("displayMode"), static_cast<int>(displayMode()));
    return state;
}

bool IDOSRenderProvider::restorePresentationState(const QVariantMap& state)
{
    bool valid = false;
    const double opacityValue = state.value(QStringLiteral("opacity")).toDouble(&valid);
    const int mode = state.value(QStringLiteral("displayMode")).toInt();
    if (!valid || !(opacityValue >= 0.0 && opacityValue <= 1.0) ||
        mode < static_cast<int>(IDOSDisplayMode::Surface) || mode > static_cast<int>(IDOSDisplayMode::Points) ||
        state.value(QStringLiteral("visible")).type() != QVariant::Bool)
    {
        return false;
    }
    setVisible(state.value(QStringLiteral("visible")).toBool());
    setOpacity(opacityValue);
    setDisplayMode(static_cast<IDOSDisplayMode>(mode));
    return true;
}
