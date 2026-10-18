#include <QJsonArray>
#include <QObject>

#include "data/grid/idosgridfilterobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosproject.h"

#include "idosdeletegridfiltercommand.h"

IDOSDeleteGridFilterCommand::IDOSDeleteGridFilterCommand(IDOSProject* project, const QJsonObject& arguments)
    :
    IDOSCommand(QStringLiteral("grid.filter.delete"), Type::Action, project, QObject::tr("Grid filter"))
    , m_arguments(arguments)
{
}

IDOSDeleteGridFilterCommand::~IDOSDeleteGridFilterCommand()
{
    if (m_filter != nullptr && m_filter->parent() == nullptr)
    {
        delete m_filter.data();
    }
}

bool IDOSDeleteGridFilterCommand::validateCommand(QString& error) const
{
    if (project() == nullptr)
    {
        error = QObject::tr("The target project is unavailable.");
        return false;
    }
    IDOSGridFilterObject* filter = qobject_cast<IDOSGridFilterObject*>(project()->object(m_arguments.value(QStringLiteral("filterId")).toString()));
    if (filter == nullptr || (m_filter != nullptr && m_filter != filter))
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    return true;

}

QJsonObject IDOSDeleteGridFilterCommand::buildPreview() const
{
    return m_arguments;
}

bool IDOSDeleteGridFilterCommand::apply(QString& error)
{
    m_filter = qobject_cast<IDOSGridFilterObject*>(project()->object(m_arguments.value(QStringLiteral("filterId")).toString()));
    if (m_filter == nullptr)
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    project()->take(m_filter->objectId());
    QJsonObject commandResult;
    commandResult.insert(QStringLiteral("filterId"), m_filter->objectId());
    setResult(commandResult);
    return true;
}

bool IDOSDeleteGridFilterCommand::revert(QString& error)
{
    if (project() == nullptr || m_filter == nullptr || m_filter->parent() != nullptr ||
        !project()->add(m_filter.data()))
    {
        error = QObject::tr("The target filter is unavailable.");
        return false;
    }
    return true;
}

IDOSDeleteGridFilterCommandMetadata::IDOSDeleteGridFilterCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("grid.filter.delete"), QObject::tr("Grid filter"),
        QObject::tr("Create, update or delete a saved grid filter."), IDOSCommand::Type::Action)
{
}

QJsonObject IDOSDeleteGridFilterCommandMetadata::schema() const
{
    QJsonObject properties;
    QJsonArray required;
    const QStringList keys{QStringLiteral("filterId")};
    for (const QString& key : keys)
    {
        QJsonObject property;
        property.insert(QStringLiteral("type"), key == QStringLiteral("criteria") ? QStringLiteral("object") : QStringLiteral("string"));
        properties.insert(key, property);
        required.append(key);
    }
    QJsonObject schema;
    schema.insert(QStringLiteral("type"), QStringLiteral("object"));
    schema.insert(QStringLiteral("properties"), properties);
    schema.insert(QStringLiteral("required"), required);
    return schema;
}

IDOSCommand* IDOSDeleteGridFilterCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSDeleteGridFilterCommand(project, arguments);
}
