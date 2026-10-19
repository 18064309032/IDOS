#ifndef IDOS_UPDATE_PROJECT_METADATA_COMMAND_H
#define IDOS_UPDATE_PROJECT_METADATA_COMMAND_H

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"
#include "idosprojectmetadata.h"

class APP_EXPORT IDOSUpdateProjectMetadataCommand : public IDOSCommand
{
  public:
    IDOSUpdateProjectMetadataCommand(IDOSProject* project, const QJsonObject& arguments);

  protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    IDOSProjectMetadata m_before;
    IDOSProjectMetadata m_after;
    QString m_inputError;
    bool m_captured;
};

class APP_EXPORT IDOSUpdateProjectMetadataCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSUpdateProjectMetadataCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
