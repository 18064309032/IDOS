#ifndef IDOS_PLUGIN_REGISTRY_H
#define IDOS_PLUGIN_REGISTRY_H

#include <QDir>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include "idospluginmetadata.h"

class IDOSInterface;
class IDOSPlugin;
class QLibrary;

class IDOSPluginRegistry : public QObject
{
    Q_OBJECT

  public:
    ~IDOSPluginRegistry() override;

    static IDOSPluginRegistry* instance();
    void setIDosInterface(IDOSInterface* idosInterface);

    void restoreSessionPlugins(const QString& directoryPath);
    void loadCppPlugin(const QString& libraryPath);
    void unloadCppPlugin(const QString& libraryPath);
    void setPluginEnabled(const QString& libraryPath, bool enabled);
    void unloadAll();
    QDir libraryDir() const;
    bool isLoaded(const QString& key) const;
    QString pluginError(const QString& key) const;
    QVariantList pluginCatalog(const QString& directoryPath);

  private:
    IDOSPluginRegistry();
    static QStringList cppPluginNameFilters();
    static QStringList cppPluginLibraryPaths(const QDir& pluginRoot);
    static void destroyPluginInstance(IDOSPlugin* plugin);
    void unloadPluginByKey(const QString& key, bool updateSettings);

    QPointer<IDOSInterface> m_idosInterface;
    QMap<QString, IDOSPluginMetadata> m_plugins;
    QMap<QString, QString> m_pluginErrors;

    static IDOSPluginRegistry* m_instance;
};

#endif // IDOS_PLUGIN_REGISTRY_H
