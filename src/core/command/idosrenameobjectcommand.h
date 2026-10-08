#ifndef IDOS_RENAME_OBJECT_COMMAND_H
#define IDOS_RENAME_OBJECT_COMMAND_H

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"

class CORE_EXPORT IDOSRenameObjectCommand : public IDOSCommand
{
public:
    IDOSRenameObjectCommand(IDOSProject* project,
                            QString objectId,
                            QString newName,
                            QUndoCommand* parent = nullptr);
    ~IDOSRenameObjectCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    QString m_objectId;
    QString m_newName;
    QString m_oldName;
    bool m_oldNameCaptured;
};

class CORE_EXPORT IDOSRenameObjectCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSRenameObjectCommandMetadata();
    ~IDOSRenameObjectCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_RENAME_OBJECT_COMMAND_H
