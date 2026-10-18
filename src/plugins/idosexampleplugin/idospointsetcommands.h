#ifndef IDOS_POINT_SET_COMMANDS_H
#define IDOS_POINT_SET_COMMANDS_H

#include <QPointer>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"

class IDOSDataRegistry;
class IDOSPointSetObject;

class IDOSCreatePointSetCommand : public IDOSCommand
{
  public:
    IDOSCreatePointSetCommand(IDOSProject* project, const QString& name, IDOSDataRegistry* registry);
    ~IDOSCreatePointSetCommand() override;

  protected:
    bool validateCommand(QString& error) const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    QString m_name;
    QPointer<IDOSDataRegistry> m_registry;
    QPointer<IDOSPointSetObject> m_data;
};

class IDOSCreatePointSetCommandMetadata : public IDOSCommandMetadata
{
  public:
    explicit IDOSCreatePointSetCommandMetadata(IDOSDataRegistry* registry);
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;

  private:
    QPointer<IDOSDataRegistry> m_registry;
};

class IDOSDeletePointSetCommand : public IDOSCommand
{
  public:
    IDOSDeletePointSetCommand(IDOSProject* project, const QString& id);
    ~IDOSDeletePointSetCommand() override;

  protected:
    bool validateCommand(QString& error) const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    QString m_id;
    QPointer<IDOSPointSetObject> m_data;
};

class IDOSDeletePointSetCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSDeletePointSetCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif
