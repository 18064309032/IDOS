#include <QObject>

#include "idosrendertoolprovider.h"

#include "idosrendertoolmetadata.h"

QString IDOSRenderToolMetadata::id() const
{
    return QStringLiteral("idos.view.navigation");
}

QString IDOSRenderToolMetadata::name() const
{
    return QObject::tr("View");
}

QString IDOSRenderToolMetadata::description() const
{
    return QObject::tr("Navigate and adjust the current view.");
}

QString IDOSRenderToolMetadata::iconPath() const
{
    return QStringLiteral(":/images/render-standard-views.svg");
}

QString IDOSRenderToolMetadata::groupId() const
{
    return QStringLiteral("navigation");
}

int IDOSRenderToolMetadata::order() const
{
    return 10;
}

bool IDOSRenderToolMetadata::supports(const IDOSRenderView* view) const
{
    return view != nullptr;
}

IDOSToolProvider* IDOSRenderToolMetadata::create(IDOSRenderView* view) const
{
    if (view == nullptr)
    {
        return nullptr;
    }
    return new IDOSRenderToolProvider(view);
}
