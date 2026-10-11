#ifndef IDOS_DATA_REGISTRY_H
#define IDOS_DATA_REGISTRY_H

#include <QHash>
#include <QObject>
#include <QString>

#include "idos_core.h"

class IDOSDataMetadata;

class CORE_EXPORT IDOSDataRegistry : public QObject
{
    Q_OBJECT

  public:
    explicit IDOSDataRegistry(QObject* parent = nullptr);
    ~IDOSDataRegistry() override;

    bool add(IDOSDataMetadata* metadata);
    bool remove(const QString& typeId);
    const IDOSDataMetadata* metadata(const QString& typeId) const;

  signals:
    void added(const QString& typeId);
    void aboutToBeRemoved(const QString& typeId);
    void removed(const QString& typeId);

  private:
    Q_DISABLE_COPY(IDOSDataRegistry)

    QHash<QString, IDOSDataMetadata*> m_metadata;
    bool m_changing;
};

#endif
