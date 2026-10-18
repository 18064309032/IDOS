#ifndef IDOS_TRANSFER_OBJECTS_COMMAND_H
#define IDOS_TRANSFER_OBJECTS_COMMAND_H

#include <QHash>
#include <QJsonArray>
#include <QList>
#include <QPointer>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "data/case/idoscaseitemref.h"
#include "idos_app.h"

class IDOSDataObject;

class APP_EXPORT IDOSTransferObjectsCommand : public IDOSCommand
{
public:
    IDOSTransferObjectsCommand(IDOSProject* project, const QString& operation, const QJsonObject& arguments);
    ~IDOSTransferObjectsCommand() override;
protected:
    bool validateCommand(QString& error) const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;
private:
    bool prepare(QString& error);
    bool sameReferences(const QList<IDOSCaseItemRef>& first, const QList<IDOSCaseItemRef>& second) const;
    IDOSDataObject* copyObject(IDOSDataObject* source, QHash<QString, QString>& ids, QString& error);
    void remapFilters(const QHash<QString, QString>& ids);
    bool checkState(bool applied, QString& error) const;
    void writeState(bool applied);
    QString uniqueName(const IDOSDataObject* object) const;
    QString m_operation;
    QJsonObject m_arguments;
    QList<QPointer<IDOSDataObject>> m_copies;
    QHash<QString, QList<IDOSCaseItemRef>> m_beforeRefs;
    QHash<QString, QList<IDOSCaseItemRef>> m_afterRefs;
    QHash<QString, QString> m_beforeContainers;
    QHash<QString, QString> m_afterContainers;
    bool m_initialized;
};

class APP_EXPORT IDOSTransferObjectsCommandMetadata : public IDOSCommandMetadata
{
public:
    explicit IDOSTransferObjectsCommandMetadata(const QString& operation);
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
private:
    QString m_operation;
};

#endif
