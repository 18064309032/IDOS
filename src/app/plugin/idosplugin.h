#ifndef IDOS_PLUGIN_H
#define IDOS_PLUGIN_H

#include <QtGlobal>
#include <QString>

class IDOSInterface;

#define IDOS_PLUGIN_EXPORT Q_DECL_EXPORT

class IDOSPlugin
{
  public:
    explicit IDOSPlugin(const QString& name = QString(),
                        const QString& description = QString(),
                        const QString& category = QString(),
                        const QString& version = QString())
        : m_name(name)
        , m_description(description)
        , m_category(category)
        , m_version(version)
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

    const QString& version() const
    {
        return m_version;
    }

    QString& version()
    {
        return m_version;
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

    virtual void initGui() = 0;
    virtual void unload() = 0;

  private:
    QString m_name;
    QString m_description;
    QString m_category;
    QString m_version;
};

typedef IDOSPlugin* (*IDOSPluginFactory)(IDOSInterface*);
typedef const QString* (*IDOSPluginStringMetadata)();

#endif // IDOS_PLUGIN_H
