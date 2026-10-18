#ifndef IDOS_FIELD_DIFFERENCE_COMMAND_H
#define IDOS_FIELD_DIFFERENCE_COMMAND_H

#include <QJsonObject>
#include <QPointer>
#include <QVector>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"

class IDOSGridPropertyObject;

class IDOSFieldDifferenceCommand : public IDOSCommand
{
  public:
    IDOSFieldDifferenceCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSFieldDifferenceCommand() override;
    static int liveCount();

  protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    bool readInputs(QVector<double>& before, QVector<double>& after, QString& error) const;
    static int s_liveCount;
    QJsonObject m_arguments;
    QJsonObject m_statistics;
    QPointer<IDOSGridPropertyObject> m_output;
};

class IDOSFieldDifferenceCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSFieldDifferenceCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif // IDOS_FIELD_DIFFERENCE_COMMAND_H
