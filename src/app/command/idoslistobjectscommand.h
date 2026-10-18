#ifndef IDOS_LIST_OBJECTS_COMMAND_H
#define IDOS_LIST_OBJECTS_COMMAND_H

#include <QJsonObject>
#include <QString>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class APP_EXPORT IDOSListObjectsCommand : public IDOSCommand
{
public:
    IDOSListObjectsCommand(IDOSProject* project, const QString& name, const QString& typeId, const QString& containerId, int offset, int limit);
    ~IDOSListObjectsCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    QString m_nameFilter;
    QString m_typeId;
    QString m_containerId;
    int m_offset;
    int m_limit;
};

class APP_EXPORT IDOSListObjectsCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSListObjectsCommandMetadata();
    ~IDOSListObjectsCommandMetadata() override;
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
