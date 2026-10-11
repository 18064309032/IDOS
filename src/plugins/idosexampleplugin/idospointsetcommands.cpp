#include <QJsonArray>
#include <QObject>
#include <QVector>
#include <QVector3D>

#include "idosdatametadata.h"
#include "idosdataregistry.h"
#include "idospointset.h"
#include "idosproject.h"

#include "idospointsetcommands.h"

IDOSCreatePointSetCommand::IDOSCreatePointSetCommand(IDOSProject* project, const QString& name, IDOSDataRegistry* registry)
    :
    IDOSCommand(QString::fromLatin1("pointset.create"), Type::Action, project, QObject::tr("Create Point Set"))
    , m_name(name)
    , m_registry(registry)
{
}

IDOSCreatePointSetCommand::~IDOSCreatePointSetCommand()
{
    if (m_data != nullptr && m_data->parent() == nullptr)
    {
        delete m_data.data();
    }
}

bool IDOSCreatePointSetCommand::validateCommand(QString& error) const
{
    if (project() == nullptr || m_registry == nullptr ||
        m_registry->metadata(QString::fromLatin1("idos.example.pointset")) == nullptr || m_name.trimmed().isEmpty())
    {
        error = QObject::tr("The point set name, project or factory is unavailable.");
        return false;
    }
    return true;
}

bool IDOSCreatePointSetCommand::apply(QString& error)
{
    if (m_data == nullptr)
    {
        IDOSDataObject* object = m_registry->metadata(QString::fromLatin1("idos.example.pointset"))->create();
        IDOSPointSet* data = qobject_cast<IDOSPointSet*>(object);
        if (data == nullptr)
        {
            delete object;
            error = QObject::tr("The point set factory returned an invalid object.");
            return false;
        }
        data->setName(m_name.trimmed());
        QVector<QVector3D> points;
        for (int x = 0; x < 2; ++x)
        {
            for (int y = 0; y < 2; ++y)
            {
                for (int z = 0; z < 2; ++z)
                {
                    points.append(QVector3D(x * 100.0f, y * 100.0f, z * 100.0f));
                }
            }
        }
        data->setPoints(points);
        m_data = data;
    }
    if (m_data->parent() != nullptr || project()->object(m_data->objectId()) != nullptr)
    {
        error = QObject::tr("The point set is already attached.");
        return false;
    }
    project()->add(m_data.data());
    QJsonObject result;
    result.insert(QString::fromLatin1("objectId"), m_data->objectId());
    result.insert(QString::fromLatin1("pointCount"), m_data->points().size());
    setResult(result);
    return true;
}

bool IDOSCreatePointSetCommand::revert(QString& error)
{
    if (project() == nullptr || m_data == nullptr || project()->object(m_data->objectId()) != m_data)
    {
        error = QObject::tr("The point set is unavailable.");
        return false;
    }
    project()->take(m_data->objectId());
    return true;
}

IDOSCreatePointSetCommandMetadata::IDOSCreatePointSetCommandMetadata(IDOSDataRegistry* registry)
    :
    IDOSCommandMetadata(QString::fromLatin1("pointset.create"), QObject::tr("Create Point Set"),
                        QObject::tr("Create an example point set containing eight cube corners."), IDOSCommand::Type::Action)
    , m_registry(registry)
{
}

QJsonObject IDOSCreatePointSetCommandMetadata::schema() const
{
    QJsonObject name;
    name.insert(QString::fromLatin1("type"), QString::fromLatin1("string"));
    QJsonObject properties;
    properties.insert(QString::fromLatin1("name"), name);
    QJsonObject schema;
    schema.insert(QString::fromLatin1("type"), QString::fromLatin1("object"));
    schema.insert(QString::fromLatin1("properties"), properties);
    schema.insert(QString::fromLatin1("required"), QJsonArray{QString::fromLatin1("name")});
    return schema;
}

IDOSCommand* IDOSCreatePointSetCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSCreatePointSetCommand(project, arguments.value(QString::fromLatin1("name")).toString(), m_registry);
}

IDOSDeletePointSetCommand::IDOSDeletePointSetCommand(IDOSProject* project, const QString& id)
    :
    IDOSCommand(QString::fromLatin1("pointset.delete"), Type::Action, project, QObject::tr("Delete Point Set"))
    , m_id(id)
{
}

IDOSDeletePointSetCommand::~IDOSDeletePointSetCommand()
{
    if (m_data != nullptr && m_data->parent() == nullptr)
    {
        delete m_data.data();
    }
}

bool IDOSDeletePointSetCommand::validateCommand(QString& error) const
{
    if (project() == nullptr || qobject_cast<IDOSPointSet*>(project()->object(m_id)) == nullptr)
    {
        error = QObject::tr("The point set is unavailable.");
        return false;
    }
    return true;
}

bool IDOSDeletePointSetCommand::apply(QString& error)
{
    Q_UNUSED(error);
    m_data = qobject_cast<IDOSPointSet*>(project()->take(m_id));
    QJsonObject result;
    result.insert(QString::fromLatin1("objectId"), m_id);
    setResult(result);
    return m_data != nullptr;
}

bool IDOSDeletePointSetCommand::revert(QString& error)
{
    if (project() == nullptr || m_data == nullptr || m_data->parent() != nullptr || project()->object(m_id) != nullptr)
    {
        error = QObject::tr("The point set cannot be restored.");
        return false;
    }
    project()->add(m_data.data());
    return true;
}

IDOSDeletePointSetCommandMetadata::IDOSDeletePointSetCommandMetadata()
    :
    IDOSCommandMetadata(QString::fromLatin1("pointset.delete"), QObject::tr("Delete Point Set"),
                        QObject::tr("Delete an example point set."), IDOSCommand::Type::Action)
{
}

QJsonObject IDOSDeletePointSetCommandMetadata::schema() const
{
    QJsonObject id;
    id.insert(QString::fromLatin1("type"), QString::fromLatin1("string"));
    QJsonObject properties;
    properties.insert(QString::fromLatin1("objectId"), id);
    QJsonObject schema;
    schema.insert(QString::fromLatin1("type"), QString::fromLatin1("object"));
    schema.insert(QString::fromLatin1("properties"), properties);
    schema.insert(QString::fromLatin1("required"), QJsonArray{QString::fromLatin1("objectId")});
    return schema;
}

IDOSCommand* IDOSDeletePointSetCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSDeletePointSetCommand(project, arguments.value(QString::fromLatin1("objectId")).toString());
}
