#ifndef IDOS_UPDATE_GRID_FILTER_COMMAND_H
#define IDOS_UPDATE_GRID_FILTER_COMMAND_H

#include <QJsonObject>
#include <QPointer>
#include <QVariantMap>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idos_app.h"

class IDOSGridFilterObject;

class APP_EXPORT IDOSUpdateGridFilterCommand : public IDOSCommand
{
  public:
    IDOSUpdateGridFilterCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSUpdateGridFilterCommand() override;

  protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    QJsonObject m_arguments;
    QPointer<IDOSGridFilterObject> m_filter;
    QVariantMap m_previousCriteria;
    bool m_captured;
};

class APP_EXPORT IDOSUpdateGridFilterCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSUpdateGridFilterCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif // IDOS_UPDATE_GRID_FILTER_COMMAND_H
