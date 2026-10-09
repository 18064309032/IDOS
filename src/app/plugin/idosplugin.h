#ifndef IDOS_PLUGIN_H
#define IDOS_PLUGIN_H

#include <QString>
#include <QtGlobal>

class IDOSInterface;

#define IDOS_PLUGIN_EXPORT Q_DECL_EXPORT

class IDOSPlugin
{
  public:
    enum PluginType
    {
        UI = 1
    };

    explicit IDOSPlugin(const QString& name = QString(),
                        const QString& description = QString(),
                        const QString& category = QString(),
                        const QString& version = QString(),
                        PluginType type = UI)
        : m_name(name)
        , m_description(description)
        , m_category(category)
        , m_version(version)
        , m_type(type)
    {
    }

    virtual ~IDOSPlugin() = default;

    const QString& name() const
    {
        return m_name;
    }

    QString& name()
    {
        return m_name;
    }

    const QString& description() const
    {
        return m_description;
    }

    QString& description()
    {
        return m_description;
    }

    const QString& category() const
    {
        return m_category;
    }

    QString& category()
    {
        return m_category;
    }

    const QString& version() const
    {
        return m_version;
    }

    QString& version()
    {
        return m_version;
    }

    PluginType type() const
    {
        return m_type;
    }

    virtual void initGui() = 0;
    virtual void unload() = 0;

  private:
    QString m_name;
    QString m_description;
    QString m_category;
    QString m_version;
    PluginType m_type;
};

// Typedefs for the C-linkage functions exported by each plugin library.
typedef IDOSPlugin* IDOSPluginFactoryFunction(IDOSInterface*);
typedef void IDOSPluginUnloadFunction(IDOSPlugin*);
typedef const QString* IDOSPluginNameFunction();
typedef const QString* IDOSPluginDescriptionFunction();
typedef const QString* IDOSPluginCategoryFunction();
typedef int IDOSPluginTypeFunction();
typedef const QString* IDOSPluginVersionFunction();
typedef const QString* IDOSPluginIconFunction();

#endif // IDOS_PLUGIN_H
