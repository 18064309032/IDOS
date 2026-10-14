#ifndef IDOS_RENAME_OBJECT_COMMAND_H
#define IDOS_RENAME_OBJECT_COMMAND_H

#include "idos_app.h"
#include "idoscommand.h"
#include "idoscommandmetadata.h"

class APP_EXPORT IDOSRenameObjectCommand : public IDOSCommand
{
public:
    IDOSRenameObjectCommand(IDOSProject* project,
                            QString objectId,
                            QString newName);
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

class APP_EXPORT IDOSRenameObjectCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSRenameObjectCommandMetadata();
    ~IDOSRenameObjectCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;

private:
    static QJsonObject stringPropertySchema(const QString& description);
};

#endif // IDOS_RENAME_OBJECT_COMMAND_H
