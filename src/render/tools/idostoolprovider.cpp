#include "idosrenderview.h"

#include "idostoolprovider.h"

IDOSToolProvider::IDOSToolProvider(IDOSRenderView* view)
    :
    QObject(nullptr)
    , m_view(view)
{
}

IDOSToolProvider::~IDOSToolProvider()
{
}

IDOSRenderView* IDOSToolProvider::view() const
{
    return m_view.data();
}
