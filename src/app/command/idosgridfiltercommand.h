#ifndef IDOS_GRID_FILTER_COMMAND_H
#define IDOS_GRID_FILTER_COMMAND_H

#include <QPointer>
#include <QVariantMap>

#include "idos_app.h"
#include "idoscommand.h"
#include "idoscommandmetadata.h"

class IDOSGridFilter;

class APP_EXPORT IDOSGridFilterCommand : public IDOSCommand
{
  public:
    IDOSGridFilterCommand(IDOSProject* project, const QString& operation, const QJsonObject& arguments);
    ~IDOSGridFilterCommand() override;

  protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    QString m_operation;
    QJsonObject m_arguments;
    QPointer<IDOSGridFilter> m_filter;
    QVariantMap m_previousCriteria;
    bool m_captured;
    bool m_ownsDetached;
};

class APP_EXPORT IDOSGridFilterCommandMetadata : public IDOSCommandMetadata
{
  public:
    explicit IDOSGridFilterCommandMetadata(const QString& operation);
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;

  private:
    QString m_operation;
};

#endif
