#ifndef IDOS_CREATE_GRID_FILTER_COMMAND_H
#define IDOS_CREATE_GRID_FILTER_COMMAND_H

#include <QJsonObject>
#include <QPointer>
#include <QVariantMap>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class IDOSGridFilterObject;

class APP_EXPORT IDOSCreateGridFilterCommand : public IDOSCommand
{
  public:
    IDOSCreateGridFilterCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSCreateGridFilterCommand() override;

  protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    QJsonObject m_arguments;
    QPointer<IDOSGridFilterObject> m_filter;
};

class APP_EXPORT IDOSCreateGridFilterCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSCreateGridFilterCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif // IDOS_CREATE_GRID_FILTER_COMMAND_H
