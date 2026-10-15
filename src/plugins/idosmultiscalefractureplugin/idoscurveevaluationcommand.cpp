#include <QJsonArray>
#include <QObject>
#include <QStringList>

#include "idoscurveevaluation.h"

#include "idoscurveevaluationcommand.h"

int IDOSCurveEvaluationCommand::s_liveCount = 0;

IDOSCurveEvaluationCommand::IDOSCurveEvaluationCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("analysis.curve.evaluate"), Type::Query, project, QObject::tr("Evaluate curve fit"))
    , m_arguments(arguments)
{
    ++s_liveCount;
}

IDOSCurveEvaluationCommand::~IDOSCurveEvaluationCommand()
{
    --s_liveCount;
}

int IDOSCurveEvaluationCommand::liveCount()
{
    return s_liveCount;
}

bool IDOSCurveEvaluationCommand::apply(QString& error)
{
    QJsonObject result;
    if (!IDOSCurveEvaluation::evaluate(project(), m_arguments, result, error))
    {
        return false;
    }
    setResult(result);
    return true;
}

bool IDOSCurveEvaluationCommand::revert(QString& error)
{
    error = QObject::tr("Curve evaluation is read-only and has no undo operation.");
    return false;
}

IDOSCurveEvaluationCommandMetadata::IDOSCurveEvaluationCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("analysis.curve.evaluate"), QObject::tr("Evaluate curve fit"),
        QObject::tr("Evaluate time-aligned simulated and observed curves without changing project data."), IDOSCommand::Type::Query)
{
}

QJsonObject IDOSCurveEvaluationCommandMetadata::schema() const
{
    QJsonObject properties;
    for (const QString& key : QStringList{QStringLiteral("simulatedId"), QStringLiteral("observedId"),
        QStringLiteral("simulatedFile"), QStringLiteral("observedFile"), QStringLiteral("timeMode"), QStringLiteral("origin"), QStringLiteral("unit")})
    {
        properties.insert(key, QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}});
    }
    for (const QString& key : QStringList{QStringLiteral("simulatedSeries"), QStringLiteral("observedSeries"),
        QStringLiteral("simulatedTimeColumn"), QStringLiteral("observedTimeColumn"),
        QStringLiteral("simulatedValueColumn"), QStringLiteral("observedValueColumn")})
    {
        properties.insert(key, QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}});
    }
    for (const QString& key : QStringList{QStringLiteral("header"), QStringLiteral("confirmUnits")})
    {
        properties.insert(key, QJsonObject{{QStringLiteral("type"), QStringLiteral("boolean")}});
    }
    return QJsonObject{{QStringLiteral("type"), QStringLiteral("object")}, {QStringLiteral("properties"), properties},
        {QStringLiteral("required"), QJsonArray{QStringLiteral("unit"), QStringLiteral("confirmUnits")}}};
}

IDOSCommand* IDOSCurveEvaluationCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSCurveEvaluationCommand(project, arguments);
}
