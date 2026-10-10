#include <exception>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QLibrary>
#include <QPixmap>
#include <QSettings>
#include <QSize>
#include <QStringList>
#include <QVariantMap>

#include "idosinterface.h"
#include "idosplugin.h"
#include "log/idoslogger.h"

#include "idospluginregistry.h"

IDOSPluginRegistry* IDOSPluginRegistry::m_instance = nullptr;

IDOSPluginRegistry::IDOSPluginRegistry()
    : QObject(nullptr)
    , m_idosInterface(nullptr)
{
}

IDOSPluginRegistry::~IDOSPluginRegistry()
{
    unloadAll();
}

IDOSPluginRegistry* IDOSPluginRegistry::instance()
{
    if (m_instance == nullptr)
    {
        m_instance = new IDOSPluginRegistry();
    }
    return m_instance;
}

void IDOSPluginRegistry::setIDosInterface(IDOSInterface* idosInterface)
{
    if (!m_plugins.isEmpty() && m_idosInterface.data() != idosInterface)
    {
        unloadAll();
    }

    m_idosInterface = idosInterface;
}

void IDOSPluginRegistry::restoreSessionPlugins(const QString& directoryPath)
{
    if (m_idosInterface.isNull())
    {
        IDOS_WARN(tr("Cannot restore plugins without a host interface."));
        return;
    }

    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    const QStringList libraryPaths = cppPluginLibraryPaths(QDir(directoryPath));
    for (int index = 0; index < libraryPaths.size(); ++index)
    {
        const QString baseName = QFileInfo(libraryPaths.at(index)).baseName();
        const QString settingKey = QStringLiteral("/plugins/") + baseName;
        if (!settings.contains(settingKey) || settings.value(settingKey).toBool())
        {
            loadCppPlugin(libraryPaths.at(index));
        }
    }
}

void IDOSPluginRegistry::loadCppPlugin(const QString& libraryPath)
{
    const QString baseName = QFileInfo(libraryPath).baseName();
    m_pluginErrors.insert(baseName, QStringLiteral("Plugin load failed. See the application log for details."));

    if (m_idosInterface.isNull())
    {
        IDOS_WARN(tr("Cannot load plugin without a host interface."));
        return;
    }

    if (isLoaded(baseName))
    {
        m_pluginErrors.remove(baseName);
        return;
    }

    QLibrary* library = new QLibrary(libraryPath, this);
    if (!library->load())
    {
        m_pluginErrors.insert(baseName, library->errorString());
        IDOS_ERROR(tr("Failed to load plugin %1: %2")
                       .arg(libraryPath, library->errorString()));
        delete library;
        return;
    }

    name_t* nameFunction = reinterpret_cast<name_t*>(library->resolve("name"));
    description_t* descriptionFunction =
        reinterpret_cast<description_t*>(library->resolve("description"));
    category_t* categoryFunction =
        reinterpret_cast<category_t*>(library->resolve("category"));
    type_t* typeFunction = reinterpret_cast<type_t*>(library->resolve("type"));
    version_t* versionFunction = reinterpret_cast<version_t*>(library->resolve("version"));
    icon_t* iconFunction = reinterpret_cast<icon_t*>(library->resolve("icon"));
    create_t* factoryFunction = reinterpret_cast<create_t*>(library->resolve("classFactory"));
    if (nameFunction == nullptr || descriptionFunction == nullptr
        || categoryFunction == nullptr || typeFunction == nullptr
        || versionFunction == nullptr || iconFunction == nullptr
        || factoryFunction == nullptr)
    {
        m_pluginErrors.insert(baseName,
                              tr("Required plugin exports are missing."));
        IDOS_WARN(tr("Plugin %1 is missing required exports.").arg(baseName));
        library->unload();
        delete library;
        return;
    }

    const QString* exportedName = nullptr;
    const QString* exportedDescription = nullptr;
    const QString* exportedCategory = nullptr;
    const QString* exportedVersion = nullptr;
    const QString* exportedIcon = nullptr;
    int pluginType = 0;
    try
    {
        exportedName = nameFunction();
        exportedDescription = descriptionFunction();
        exportedCategory = categoryFunction();
        pluginType = typeFunction();
        exportedVersion = versionFunction();
        exportedIcon = iconFunction();
    }
    catch (const std::exception& exception)
    {
        m_pluginErrors.insert(baseName, QString::fromLocal8Bit(exception.what()));
        IDOS_ERROR(tr("Plugin %1 metadata failed: %2")
                       .arg(baseName, QString::fromLocal8Bit(exception.what())));
        library->unload();
        delete library;
        return;
    }
    catch (...)
    {
        m_pluginErrors.insert(baseName,
                              tr("The plugin metadata function threw an unknown exception."));
        IDOS_ERROR(tr("Plugin %1 metadata failed with an unknown exception.")
                       .arg(baseName));
        library->unload();
        delete library;
        return;
    }

    if (exportedName == nullptr || exportedName->trimmed().isEmpty()
        || exportedDescription == nullptr || exportedDescription->trimmed().isEmpty()
        || exportedCategory == nullptr || exportedCategory->trimmed().isEmpty()
        || exportedVersion == nullptr || exportedVersion->trimmed().isEmpty()
        || exportedIcon == nullptr || exportedIcon->trimmed().isEmpty()
        || pluginType != static_cast<int>(IDOSPlugin::UI))
    {
        m_pluginErrors.insert(baseName, tr("The plugin returned invalid metadata."));
        IDOS_WARN(tr("Plugin %1 has invalid metadata.").arg(baseName));
        library->unload();
        delete library;
        return;
    }
    IDOSPlugin* pluginInstance = nullptr;
    try
    {
        pluginInstance = factoryFunction(m_idosInterface.data());
    }
    catch (const std::exception& exception)
    {
        m_pluginErrors.insert(baseName, QString::fromLocal8Bit(exception.what()));
        IDOS_ERROR(tr("Plugin %1 factory failed: %2")
                       .arg(baseName, QString::fromLocal8Bit(exception.what())));
    }
    catch (...)
    {
        m_pluginErrors.insert(baseName,
                              tr("The plugin factory threw an unknown exception."));
        IDOS_ERROR(tr("Plugin %1 factory failed with an unknown exception.")
                       .arg(baseName));
    }
    if (pluginInstance == nullptr)
    {
        m_pluginErrors.insert(baseName, tr("The plugin factory returned null."));
        IDOS_ERROR(tr("Plugin %1 factory returned null.").arg(baseName));
        library->unload();
        delete library;
        return;
    }

    const QString pluginDisplayName = pluginInstance->name().trimmed();
    if (pluginDisplayName.isEmpty())
    {
        m_pluginErrors.insert(baseName, tr("The plugin returned an empty name."));
        IDOS_WARN(tr("Plugin %1 has invalid metadata.").arg(baseName));
        destroyPluginInstance(pluginInstance);
        library->unload();
        delete library;
        return;
    }
    const QString pluginDescription = pluginInstance->description();
    const QString pluginCategory = pluginInstance->category();
    const QString pluginVersion = pluginInstance->version();
    const QString pluginIcon = pluginInstance->icon();
    if (pluginIcon.trimmed().isEmpty())
    {
        m_pluginErrors.insert(baseName, tr("The plugin returned invalid metadata."));
        IDOS_WARN(tr("Plugin %1 has invalid metadata.").arg(baseName));
        destroyPluginInstance(pluginInstance);
        library->unload();
        delete library;
        return;
    }

    IDOS_DEBUG(tr("Calling initGui for plugin %1: instance=0x%2, library=0x%3")
                   .arg(baseName,
                        QString::number(reinterpret_cast<quintptr>(pluginInstance), 16),
                        QString::number(reinterpret_cast<quintptr>(library), 16)));
    try
    {
        pluginInstance->initGui();
    }
    catch (const std::exception& exception)
    {
        m_pluginErrors.insert(baseName, QString::fromLocal8Bit(exception.what()));
        IDOS_ERROR(tr("Plugin %1 initialization failed: %2")
                       .arg(baseName, QString::fromLocal8Bit(exception.what())));
        destroyPluginInstance(pluginInstance);
        library->unload();
        delete library;
        return;
    }
    catch (...)
    {
        m_pluginErrors.insert(baseName,
                              tr("The plugin initialization threw an unknown exception."));
        IDOS_ERROR(tr("Plugin %1 initialization failed with an unknown exception.")
                       .arg(baseName));
        destroyPluginInstance(pluginInstance);
        library->unload();
        delete library;
        return;
    }

    if (m_idosInterface.isNull())
    {
        m_pluginErrors.insert(baseName,
                              QStringLiteral("The host interface was destroyed during initialization."));
        destroyPluginInstance(pluginInstance);
        library->unload();
        delete library;
        return;
    }
    QObject* pluginObject = dynamic_cast<QObject*>(pluginInstance);
    if (pluginObject != nullptr)
    {
        if (pluginObject->objectName().isEmpty())
        {
            pluginObject->setObjectName(QStringLiteral("idos_plugin_%1").arg(baseName));
        }
        if (pluginObject->parent() == nullptr)
        {
            pluginObject->setParent(m_idosInterface->mainWindow());
        }
    }

    m_plugins.insert(baseName,
                     IDOSPluginMetadata(pluginDisplayName,
                                        pluginDescription,
                                        pluginCategory,
                                        pluginVersion,
                                        pluginIcon,
                                        static_cast<IDOSPlugin::PluginType>(pluginType),
                                        pluginInstance,
                                        library));

    IDOS_DEBUG(tr("Plugin initGui completed: key=%1, instance=0x%2, library=0x%3")
                   .arg(baseName,
                        QString::number(reinterpret_cast<quintptr>(pluginInstance), 16),
                        QString::number(reinterpret_cast<quintptr>(library), 16)));

    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    settings.setValue(QStringLiteral("/plugins/") + baseName, true);
    m_pluginErrors.remove(baseName);
    IDOS_MESSAGE(tr("Plugin %1 loaded successfully.").arg(pluginDisplayName),
                 IDOSLogLevel::Info);
}

void IDOSPluginRegistry::unloadCppPlugin(const QString& libraryPath)
{
    const QString baseName = QFileInfo(libraryPath).baseName();
    unloadPluginByKey(baseName, true);
}

void IDOSPluginRegistry::setPluginEnabled(const QString& libraryPath, bool enabled)
{
    const QString baseName = QFileInfo(libraryPath).baseName();
    const QString absolutePath = QFileInfo(libraryPath).absoluteFilePath();
    IDOS_DEBUG(tr("setPluginEnabled entered: key=%1, enabled=%2, loaded=%3, path=%4")
                   .arg(baseName)
                   .arg(enabled)
                   .arg(isLoaded(baseName))
                   .arg(absolutePath));

    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
    settings.setValue(QStringLiteral("/plugins/") + baseName, enabled);
    if (enabled)
    {
        IDOS_DEBUG(tr("Enabling plugin through registry: key=%1").arg(baseName));
        loadCppPlugin(absolutePath);
        return;
    }

    m_pluginErrors.remove(baseName);
    IDOS_DEBUG(tr("Disabling plugin through registry: key=%1, loaded=%2")
                   .arg(baseName)
                   .arg(isLoaded(baseName)));
    unloadPluginByKey(baseName, false);
    IDOS_DEBUG(tr("setPluginEnabled finished: key=%1, loaded=%2")
                   .arg(baseName)
                   .arg(isLoaded(baseName)));
}

void IDOSPluginRegistry::unloadPluginByKey(const QString& key, bool updateSettings)
{
    if (!isLoaded(key))
    {
        return;
    }

    if (updateSettings)
    {
        QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));
        settings.setValue(QStringLiteral("/plugins/") + key, false);
    }

    QMap<QString, IDOSPluginMetadata>::iterator iterator = m_plugins.find(key);
    if (iterator == m_plugins.end())
    {
        return;
    }

    IDOSPlugin* pluginInstance = iterator.value().plugin();
    QLibrary* library = iterator.value().library();
    m_plugins.erase(iterator);
    IDOS_DEBUG(tr("Starting plugin unload: key=%1, instance=0x%2, library=0x%3")
                   .arg(key,
                        QString::number(reinterpret_cast<quintptr>(pluginInstance), 16),
                        QString::number(reinterpret_cast<quintptr>(library), 16)));
    destroyPluginInstance(pluginInstance);
    IDOS_DEBUG(tr("Plugin instance destroyed: key=%1, library=0x%2")
                   .arg(key, QString::number(reinterpret_cast<quintptr>(library), 16)));

    if (library != nullptr)
    {
        const bool unloaded = library->unload();
        if (unloaded)
        {
            IDOS_DEBUG(tr("Plugin library unloaded: key=%1, library=0x%2")
                           .arg(key, QString::number(reinterpret_cast<quintptr>(library), 16)));
        }
        else
        {
            IDOS_ERROR(tr("Plugin library unload failed: key=%1, library=%2, error=%3")
                           .arg(key, library->fileName(), library->errorString()));
        }
        delete library;
    }
}

void IDOSPluginRegistry::unloadAll()
{
    const QStringList pluginKeys = m_plugins.keys();
    for (int index = 0; index < pluginKeys.size(); ++index)
    {
        unloadPluginByKey(pluginKeys.at(index), false);
    }
}

QDir IDOSPluginRegistry::libraryDir() const
{
    return QDir(QCoreApplication::applicationDirPath() + QStringLiteral("/plugins"));
}

bool IDOSPluginRegistry::isLoaded(const QString& key) const
{
    return m_plugins.contains(key);
}

QString IDOSPluginRegistry::pluginError(const QString& key) const
{
    return m_pluginErrors.value(key);
}

QVariantList IDOSPluginRegistry::pluginCatalog(const QString& directoryPath)
{
    QVariantList result;
    const QStringList libraryPaths = cppPluginLibraryPaths(QDir(directoryPath));
    QSettings settings(QStringLiteral("pluginRegistry"), QStringLiteral("idos"));

    for (int index = 0; index < libraryPaths.size(); ++index)
    {
        const QString libraryPath = libraryPaths.at(index);
        const QString key = QFileInfo(libraryPath).baseName();

        QVariantMap pluginInfo;
        pluginInfo.insert(QStringLiteral("key"), key);
        pluginInfo.insert(QStringLiteral("path"), libraryPath);
        pluginInfo.insert(QStringLiteral("enabled"),
                          !settings.contains(QStringLiteral("/plugins/") + key)
                              || settings.value(QStringLiteral("/plugins/") + key).toBool());
        pluginInfo.insert(QStringLiteral("loaded"), isLoaded(key));
        pluginInfo.insert(QStringLiteral("error"), m_pluginErrors.value(key));

        QMap<QString, IDOSPluginMetadata>::const_iterator loadedIterator = m_plugins.constFind(key);
        if (loadedIterator != m_plugins.constEnd())
        {
            const IDOSPluginMetadata& metadata = loadedIterator.value();
            pluginInfo.insert(QStringLiteral("name"), metadata.name());
            pluginInfo.insert(QStringLiteral("description"), metadata.description());
            pluginInfo.insert(QStringLiteral("category"), metadata.category());
            pluginInfo.insert(QStringLiteral("version"), metadata.version());
            pluginInfo.insert(QStringLiteral("icon"), metadata.icon());
            const QPixmap iconPixmap = QIcon(metadata.icon()).pixmap(QSize(64, 64));
            if (!iconPixmap.isNull())
            {
                pluginInfo.insert(QStringLiteral("iconPixmap"), iconPixmap);
            }
            pluginInfo.insert(QStringLiteral("type"), static_cast<int>(metadata.type()));
        }
        else
        {
            const QVariantMap exportedMetadata = pluginExportMetadata(libraryPath);
            pluginInfo.insert(QStringLiteral("name"),
                              exportedMetadata.value(QStringLiteral("name"), key));
            pluginInfo.insert(QStringLiteral("description"),
                              exportedMetadata.value(QStringLiteral("description")));
            pluginInfo.insert(QStringLiteral("category"),
                              exportedMetadata.value(QStringLiteral("category")));
            pluginInfo.insert(QStringLiteral("version"),
                              exportedMetadata.value(QStringLiteral("version")));
            pluginInfo.insert(QStringLiteral("icon"),
                              exportedMetadata.value(QStringLiteral("icon")));
            pluginInfo.insert(QStringLiteral("iconPixmap"),
                              exportedMetadata.value(QStringLiteral("iconPixmap")));
            pluginInfo.insert(QStringLiteral("type"),
                              exportedMetadata.value(QStringLiteral("type"), 0));
        }
        result.append(pluginInfo);
    }
    return result;
}

QStringList IDOSPluginRegistry::cppPluginNameFilters()
{
#if defined(Q_OS_WIN) || defined(__CYGWIN__)
    return QStringList() << QStringLiteral("*.dll");
#else
    return QStringList() << QStringLiteral("*.so*");
#endif
}

QStringList IDOSPluginRegistry::cppPluginLibraryPaths(const QDir& pluginRoot)
{
    QStringList result;
    const QStringList filters = cppPluginNameFilters();
    const QFileInfoList rootLibraries = pluginRoot.entryInfoList(
        filters, QDir::Files | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase);
    for (int index = 0; index < rootLibraries.size(); ++index)
    {
        result.append(rootLibraries.at(index).absoluteFilePath());
    }

    const QFileInfoList packageDirectories = pluginRoot.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase);
    for (int directoryIndex = 0; directoryIndex < packageDirectories.size(); ++directoryIndex)
    {
        const QDir packageDirectory(packageDirectories.at(directoryIndex).absoluteFilePath());
        const QFileInfoList packageLibraries = packageDirectory.entryInfoList(
            filters, QDir::Files | QDir::NoSymLinks, QDir::Name | QDir::IgnoreCase);
        for (int libraryIndex = 0; libraryIndex < packageLibraries.size(); ++libraryIndex)
        {
            result.append(packageLibraries.at(libraryIndex).absoluteFilePath());
        }
    }
    return result;
}

QVariantMap IDOSPluginRegistry::pluginExportMetadata(const QString& libraryPath)
{
    QVariantMap metadata;
    QLibrary library(libraryPath);
    if (!library.load())
    {
        return metadata;
    }

    name_t* nameFunction = reinterpret_cast<name_t*>(library.resolve("name"));
    description_t* descriptionFunction =
        reinterpret_cast<description_t*>(library.resolve("description"));
    category_t* categoryFunction =
        reinterpret_cast<category_t*>(library.resolve("category"));
    type_t* typeFunction = reinterpret_cast<type_t*>(library.resolve("type"));
    version_t* versionFunction = reinterpret_cast<version_t*>(library.resolve("version"));
    icon_t* iconFunction = reinterpret_cast<icon_t*>(library.resolve("icon"));
    if (nameFunction == nullptr || descriptionFunction == nullptr
        || categoryFunction == nullptr || typeFunction == nullptr
        || versionFunction == nullptr || iconFunction == nullptr)
    {
        library.unload();
        return metadata;
    }

    try
    {
        const QString* nameValue = nameFunction();
        const QString* descriptionValue = descriptionFunction();
        const QString* categoryValue = categoryFunction();
        const int typeValue = typeFunction();
        const QString* versionValue = versionFunction();
        const QString* iconValue = iconFunction();
        if (nameValue != nullptr && descriptionValue != nullptr
            && categoryValue != nullptr && versionValue != nullptr
            && iconValue != nullptr)
        {
            metadata.insert(QStringLiteral("name"), *nameValue);
            metadata.insert(QStringLiteral("description"), *descriptionValue);
            metadata.insert(QStringLiteral("category"), *categoryValue);
            metadata.insert(QStringLiteral("type"), typeValue);
            metadata.insert(QStringLiteral("version"), *versionValue);
            metadata.insert(QStringLiteral("icon"), *iconValue);
            const QPixmap iconPixmap = QIcon(*iconValue).pixmap(QSize(64, 64));
            if (!iconPixmap.isNull())
            {
                metadata.insert(QStringLiteral("iconPixmap"), iconPixmap);
            }
        }
    }
    catch (...)
    {
        metadata.clear();
    }

    library.unload();
    return metadata;
}

void IDOSPluginRegistry::destroyPluginInstance(IDOSPlugin* pluginInstance)
{
    if (pluginInstance == nullptr)
    {
        return;
    }

    try
    {
        IDOS_DEBUG(tr("Calling plugin unload(): instance=0x%1")
                       .arg(QString::number(reinterpret_cast<quintptr>(pluginInstance), 16)));
        pluginInstance->unload();
        IDOS_DEBUG(tr("Plugin unload() returned: instance=0x%1")
                       .arg(QString::number(reinterpret_cast<quintptr>(pluginInstance), 16)));
    }
    catch (const std::exception& exception)
    {
        IDOS_ERROR(tr("Plugin unload failed: %1")
                       .arg(QString::fromLocal8Bit(exception.what())));
    }
    catch (...)
    {
        IDOS_ERROR(tr("Plugin unload failed with an unknown exception."));
    }

    const QString pluginAddress = QString::number(reinterpret_cast<quintptr>(pluginInstance), 16);
    IDOS_DEBUG(tr("Deleting plugin object: instance=0x%1").arg(pluginAddress));
    delete pluginInstance;
    IDOS_DEBUG(tr("Plugin object deleted: instance=0x%1")
                   .arg(pluginAddress));
}
