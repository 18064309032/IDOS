#include <algorithm>
#include <cmath>
#include <limits>

#include <QJsonArray>
#include <QObject>
#include <QScopedPointer>
#include <QStringList>

#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertymetadata.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosproject.h"

#include "idosfielddifferencecommand.h"

int IDOSFieldDifferenceCommand::s_liveCount = 0;

IDOSFieldDifferenceCommand::IDOSFieldDifferenceCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("analysis.field.difference"), Type::Action, project, QObject::tr("Field difference"))
    , m_arguments(arguments)
{
    ++s_liveCount;
}

IDOSFieldDifferenceCommand::~IDOSFieldDifferenceCommand()
{
    if (!m_output.isNull() && m_output->parent() == nullptr)
    {
        delete m_output.data();
    }
    --s_liveCount;
}

int IDOSFieldDifferenceCommand::liveCount()
{
    return s_liveCount;
}

bool IDOSFieldDifferenceCommand::readInputs(QVector<double>& before, QVector<double>& after, QString& error) const
{
    const IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(project()->object(m_arguments.value(QStringLiteral("gridId")).toString()));
    const IDOSGridPropertyObject* inputs[2] = {
        qobject_cast<IDOSGridPropertyObject*>(project()->object(m_arguments.value(QStringLiteral("beforeId")).toString())),
        qobject_cast<IDOSGridPropertyObject*>(project()->object(m_arguments.value(QStringLiteral("afterId")).toString()))};
    if (grid == nullptr || grid->nx() <= 0 || grid->ny() <= 0 || grid->nz() <= 0
        || qint64(grid->nx()) * grid->ny() > std::numeric_limits<int>::max() / qint64(grid->nz()))
    {
        error = QObject::tr("Select a valid grid and two properties of that grid.");
        return false;
    }
    const int count = grid->totalCellCount();
    if ((!grid->actnum().isEmpty() && grid->actnum().size() != count)
        || m_arguments.value(QStringLiteral("name")).toString().trimmed().isEmpty())
    {
        error = QObject::tr("The output name or grid ACTNUM is invalid.");
        return false;
    }
    QVector<double>* values[2] = {&before, &after};
    const QStringList steps = {QStringLiteral("beforeStep"), QStringLiteral("afterStep")};
    for (int index = 0; index < 2; ++index)
    {
        const IDOSGridPropertyObject* property = inputs[index];
        if (property == nullptr || property->gridId() != grid->objectId()
            || property->numX() != grid->nx() || property->numY() != grid->ny() || property->numZ() != grid->nz())
        {
            error = QObject::tr("Both input properties must belong to the selected grid and match its dimensions.");
            return false;
        }
        if (property->kind() == IDOSGridPropertyObject::Kind::Dynamic)
        {
            const QJsonValue step = m_arguments.value(steps.at(index));
            if (!step.isDouble() || step.toDouble() != step.toInt(-1) || !property->reportSteps().contains(step.toInt(-1)))
            {
                error = QObject::tr("Select an existing report step for each dynamic input.");
                return false;
            }
            *values[index] = property->reportStepValues(step.toInt());
        }
        else
        {
            *values[index] = property->values();
        }
        if (values[index]->size() != count)
        {
            error = QObject::tr("An input property has an invalid value count.");
            return false;
        }
    }
    const bool knownMatch = !inputs[0]->keyword().trimmed().isEmpty()
        && inputs[0]->keyword().compare(inputs[1]->keyword(), Qt::CaseInsensitive) == 0
        && inputs[0]->valueUnits() == IDOSGridPropertyObject::ValueUnits::SI
        && inputs[1]->valueUnits() == IDOSGridPropertyObject::ValueUnits::SI;
    if (!knownMatch && !m_arguments.value(QStringLiteral("confirmUnits")).toBool())
    {
        error = QObject::tr("Confirm matching physical quantities and units. No unit conversion is performed.");
        return false;
    }
    return true;
}

bool IDOSFieldDifferenceCommand::validateCommand(QString& error) const
{
    if (!m_output.isNull())
    {
        const IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(project()->object(m_output->gridId()));
        if (m_output->parent() != nullptr || project()->object(m_output->objectId()) != nullptr || grid == nullptr
            || grid->nx() != m_output->numX() || grid->ny() != m_output->numY() || grid->nz() != m_output->numZ())
        {
            error = QObject::tr("The cached result cannot be restored to its grid.");
            return false;
        }
        return true;
    }
    QVector<double> before;
    QVector<double> after;
    return readInputs(before, after, error);
}

QJsonObject IDOSFieldDifferenceCommand::buildPreview() const
{
    return m_arguments;
}

bool IDOSFieldDifferenceCommand::apply(QString& error)
{
    if (m_output.isNull() && !result().isEmpty())
    {
        error = QObject::tr("The output property is unavailable.");
        return false;
    }

    if (m_output.isNull())
    {
        QVector<double> before;
        QVector<double> after;
        if (!readInputs(before, after, error))
        {
            return false;
        }
        const IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(project()->object(m_arguments.value(QStringLiteral("gridId")).toString()));
        QVector<double> difference(before.size(), std::numeric_limits<double>::quiet_NaN());
        int count = 0;
        double minimum = std::numeric_limits<double>::max();
        double maximum = -std::numeric_limits<double>::max();
        double mean = 0.0;
        for (int index = 0; index < before.size(); ++index)
        {
            if (!grid->actnum().isEmpty() && grid->actnum().at(index) == 0)
            {
                continue;
            }
            const double value = after.at(index) - before.at(index);
            if (!std::isfinite(before.at(index)) || !std::isfinite(after.at(index)) || !std::isfinite(value))
            {
                error = QObject::tr("An active cell has a missing or nonfinite input or difference.");
                return false;
            }
            difference[index] = value;
            ++count;
            minimum = std::min(minimum, value);
            maximum = std::max(maximum, value);
            mean = mean * ((count - 1.0) / count) + value / count;
        }
        if (count == 0 || !std::isfinite(mean))
        {
            error = QObject::tr("No valid active cells are available for comparison.");
            return false;
        }
        const IDOSGridPropertyObject* source = qobject_cast<IDOSGridPropertyObject*>(project()->object(m_arguments.value(QStringLiteral("beforeId")).toString()));
        const IDOSGridPropertyMetadata metadata;
        QScopedPointer<IDOSGridPropertyObject> output(qobject_cast<IDOSGridPropertyObject*>(metadata.create()));
        output->setGridId(grid->objectId());
        output->setName(m_arguments.value(QStringLiteral("name")).toString().trimmed());
        output->setKeyword(QString::fromLatin1("DELTA_") + source->keyword());
        output->setDimensions(grid->nx(), grid->ny(), grid->nz());
        output->setValueUnits(m_arguments.value(QStringLiteral("confirmUnits")).toBool()
            ? IDOSGridPropertyObject::ValueUnits::Unknown : IDOSGridPropertyObject::ValueUnits::SI);
        if (!output->setValues(difference))
        {
            error = QObject::tr("The difference values could not be stored.");
            return false;
        }
        m_statistics = QJsonObject{{QStringLiteral("activeCount"), count}, {QStringLiteral("minimum"), minimum},
            {QStringLiteral("maximum"), maximum}, {QStringLiteral("mean"), mean}};
        m_output = output.take();
    }
    if (!project()->add(m_output.data()))
    {
        error = QObject::tr("The output property is unavailable.");
        return false;
    }
    QJsonObject result = m_statistics;
    result.insert(QStringLiteral("objectId"), m_output->objectId());
    result.insert(QStringLiteral("inputs"), m_arguments);
    setResult(result);
    return true;
}

bool IDOSFieldDifferenceCommand::revert(QString& error)
{
    if (m_output.isNull() || project()->object(m_output->objectId()) != m_output.data())
    {
        error = QObject::tr("The difference result is unavailable.");
        return false;
    }
    project()->take(m_output->objectId());
    return true;
}

IDOSFieldDifferenceCommandMetadata::IDOSFieldDifferenceCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("analysis.field.difference"), QObject::tr("Field difference"),
        QObject::tr("Subtract the before field from the after field on active cells."), IDOSCommand::Type::Action)
{
}

QJsonObject IDOSFieldDifferenceCommandMetadata::schema() const
{
    QJsonObject properties;
    for (const QString& key : QStringList{QStringLiteral("gridId"), QStringLiteral("beforeId"), QStringLiteral("afterId"), QStringLiteral("name")})
    {
        properties.insert(key, QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}});
    }
    for (const QString& key : QStringList{QStringLiteral("beforeStep"), QStringLiteral("afterStep")})
    {
        properties.insert(key, QJsonObject{{QStringLiteral("type"), QStringLiteral("integer")}});
    }
    properties.insert(QStringLiteral("confirmUnits"), QJsonObject{{QStringLiteral("type"), QStringLiteral("boolean")}});
    return QJsonObject{{QStringLiteral("type"), QStringLiteral("object")}, {QStringLiteral("properties"), properties},
        {QStringLiteral("required"), QJsonArray{QStringLiteral("gridId"), QStringLiteral("beforeId"), QStringLiteral("afterId"), QStringLiteral("name")}}};
}

IDOSCommand* IDOSFieldDifferenceCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSFieldDifferenceCommand(project, arguments);
}
