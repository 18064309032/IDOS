#ifndef IDOS_PLUGIN_METADATA_H
#define IDOS_PLUGIN_METADATA_H

#include <QString>

#include "idosplugin.h"

class QLibrary;

class IDOSPluginMetadata
{
  public:
    IDOSPluginMetadata(const QString& name,
                       const QString& description,
                       const QString& category,
                       const QString& version,
                       const QString& icon,
                       IDOSPlugin::PluginType type,
                       IDOSPlugin* plugin,
                       QLibrary* library);
    QString name() const;
    QString description() const;
    QString category() const;
    QString version() const;
    QString icon() const;
    IDOSPlugin::PluginType type() const;
    IDOSPlugin* plugin() const;
    QLibrary* library() const;

  private:
    QString m_name;
    QString m_description;
    QString m_category;
    QString m_version;
    QString m_icon;
    IDOSPlugin::PluginType m_type;
    IDOSPlugin* m_plugin;
    QLibrary* m_library;
};

#endif // IDOS_PLUGIN_METADATA_H
