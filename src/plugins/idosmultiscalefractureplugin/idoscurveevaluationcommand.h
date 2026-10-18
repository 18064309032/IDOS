#ifndef IDOS_CURVE_EVALUATION_COMMAND_H
#define IDOS_CURVE_EVALUATION_COMMAND_H

#include <QJsonObject>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"

class IDOSCurveEvaluationCommand : public IDOSCommand
{
  public:
    IDOSCurveEvaluationCommand(IDOSProject* project, const QJsonObject& arguments);
    ~IDOSCurveEvaluationCommand() override;
    static int liveCount();

  protected:
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    static int s_liveCount;
    QJsonObject m_arguments;
};

class IDOSCurveEvaluationCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSCurveEvaluationCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif // IDOS_CURVE_EVALUATION_COMMAND_H
