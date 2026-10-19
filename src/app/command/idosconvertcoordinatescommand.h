#ifndef IDOS_CONVERT_COORDINATES_COMMAND_H
#define IDOS_CONVERT_COORDINATES_COMMAND_H

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class APP_EXPORT IDOSConvertCoordinatesCommand : public IDOSCommand
{
  public:
    IDOSConvertCoordinatesCommand(IDOSProject* project, const QJsonObject& arguments);

  protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    QJsonObject m_arguments;
};

class APP_EXPORT IDOSConvertCoordinatesCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSConvertCoordinatesCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
