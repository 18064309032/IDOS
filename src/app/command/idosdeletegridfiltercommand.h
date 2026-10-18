#ifndef IDOS_DELETE_GRID_FILTER_COMMAND_H
#define IDOS_DELETE_GRID_FILTER_COMMAND_H

#include <QJsonObject>
#include <QPointer>
#include <QVariantMap>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class IDOSGridFilterObject;

class APP_EXPORT IDOSDeleteGridFilterCommand : public IDOSCommand
{
  public:
    IDOSDeleteGridFilterCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSDeleteGridFilterCommand() override;

  protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    QJsonObject m_arguments;
    QPointer<IDOSGridFilterObject> m_filter;
};

class APP_EXPORT IDOSDeleteGridFilterCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSDeleteGridFilterCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif // IDOS_DELETE_GRID_FILTER_COMMAND_H
