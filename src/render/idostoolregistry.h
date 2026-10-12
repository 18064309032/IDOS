#ifndef IDOS_TOOL_REGISTRY_H
#define IDOS_TOOL_REGISTRY_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>

#include "idos_render.h"

class IDOSToolBar;
class IDOSToolMetadata;

class RENDER_EXPORT IDOSToolRegistry : public QObject
{
    Q_OBJECT
public:
    explicit IDOSToolRegistry(QObject* parent = nullptr);
    ~IDOSToolRegistry() override;
    bool add(IDOSToolMetadata* metadata);
    bool remove(const QString& id);
    void clear();
    bool isBusy() const;
    void attach(IDOSToolBar* toolBar);
    void detach(IDOSToolBar* toolBar);
    IDOSToolMetadata* metadata(const QString& id) const;
    QList<IDOSToolMetadata*> metadata() const;
signals:
    void metadataAdded(const QString& id);
    void metadataAboutToBeRemoved(const QString& id);
private:
    QMap<QString, IDOSToolMetadata*> m_metadataById;
    QList<QPointer<IDOSToolBar>> m_toolBars;
    bool m_changing;
};

#endif // IDOS_TOOL_REGISTRY_H
