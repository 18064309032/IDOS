#include "idosexampleplugin.h"
#include "idospointsettreeprovider.h"

#include "idospointsettreemetadata.h"

IDOSPointSetTreeMetadata::IDOSPointSetTreeMetadata(IDOSExamplePlugin* plugin)
    :
    m_plugin(plugin)
{
}

QString IDOSPointSetTreeMetadata::id() const
{
    return QString::fromLatin1("idos.example.pointset");
}

QString IDOSPointSetTreeMetadata::treeName() const
{
    return QString::fromLatin1("data");
}

IDOSTreeItemProvider* IDOSPointSetTreeMetadata::create(IDOSDataObject* data) const
{
    IDOSPointSetTreeProvider* provider = new IDOSPointSetTreeProvider(data);
    if (m_plugin != nullptr)
    {
        QObject::connect(provider, &IDOSPointSetTreeProvider::deletePointSetRequested, m_plugin.data(),
                         &IDOSExamplePlugin::onDeletePointSet);
    }
    return provider;
}
