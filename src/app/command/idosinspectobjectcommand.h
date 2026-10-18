#ifndef IDOS_INSPECT_OBJECT_COMMAND_H
#define IDOS_INSPECT_OBJECT_COMMAND_H

#include <QJsonObject>
#include <QString>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class APP_EXPORT IDOSInspectObjectCommand : public IDOSCommand
{
public:
    IDOSInspectObjectCommand(IDOSProject* project, const QString& objectId, int offset, int limit);
    ~IDOSInspectObjectCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    QString m_objectId;
    int m_offset;
    int m_limit;
};

class APP_EXPORT IDOSInspectObjectCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSInspectObjectCommandMetadata();
    ~IDOSInspectObjectCommandMetadata() override;
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
