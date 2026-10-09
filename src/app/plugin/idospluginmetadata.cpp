#include "idosplugin.h"

#include "idospluginmetadata.h"

IDOSPluginMetadata::IDOSPluginMetadata(const QString& name,
                                       const QString& description,
                                       const QString& category,
                                       const QString& version,
                                       IDOSPlugin* plugin,
                                       QLibrary* library)
    : m_name(name)
    , m_description(description)
    , m_category(category)
    , m_version(version)
    , m_plugin(plugin)
    , m_library(library)
{
    m_name.detach();
    m_description.detach();
    m_category.detach();
    m_version.detach();
}

QString IDOSPluginMetadata::name() const
{
    return m_name;
}

QString IDOSPluginMetadata::description() const
{
    return m_description;
}

QString IDOSPluginMetadata::category() const
{
    return m_category;
}

QString IDOSPluginMetadata::version() const
{
    return m_version;
}

IDOSPlugin* IDOSPluginMetadata::plugin() const
{
    return m_plugin;
}

QLibrary* IDOSPluginMetadata::library() const
{
    return m_library;
}
