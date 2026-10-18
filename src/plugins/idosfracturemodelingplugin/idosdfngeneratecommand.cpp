#include <QJsonArray>
#include <QObject>
#include <QScopedPointer>
#include <QStringList>

#include "data/fracture/idosnaturalfracturesetobject.h"
#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"
#include "idosdfngenerator.h"
#include "idosproject.h"

#include "idosdfngeneratecommand.h"

int IDOSDfnGenerateCommand::s_liveCount = 0;

IDOSDfnGenerateCommand::IDOSDfnGenerateCommand(IDOSProject* project, const QJsonObject& parameters)
    :
    IDOSCommand(QStringLiteral("fracture.dfn.generate"), Type::Action, project, QObject::tr("Generate DFN"))
    , m_parameters(parameters)
{
    ++s_liveCount;
}

IDOSDfnGenerateCommand::~IDOSDfnGenerateCommand()
{
    if (!m_data.isNull() && m_data->parent() == nullptr)
    {
        delete m_data.data();
    }
    --s_liveCount;
}

int IDOSDfnGenerateCommand::liveCount()
{
    return s_liveCount;
}

bool IDOSDfnGenerateCommand::validateCommand(QString& error) const
{
    if (!m_data.isNull())
    {
        if (m_data->parent() != nullptr || project()->object(m_data->objectId()) != nullptr)
        {
            error = QObject::tr("The generated fracture set is already attached to a project.");
            return false;
        }
        return true;
    }
    if (m_parameters.value(QStringLiteral("name")).toString().trimmed().isEmpty())
    {
        error = QObject::tr("Enter a name for the generated fracture set.");
        return false;
    }
    const IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(project()->object(m_parameters.value(QStringLiteral("gridId")).toString()));
    const QString densityId = m_parameters.value(QStringLiteral("densityId")).toString();
    const IDOSGridPropertyObject* density = densityId.isEmpty() ? nullptr
        : qobject_cast<IDOSGridPropertyObject*>(project()->object(densityId));
    if ((m_parameters.contains(QStringLiteral("densityId")) && !m_parameters.value(QStringLiteral("densityId")).isString())
        || (!densityId.isEmpty() && density == nullptr))
    {
        error = QObject::tr("The selected density weight property is unavailable.");
        return false;
    }
    return IDOSDfnGenerator::validate(grid, density, m_parameters, error);
}

QJsonObject IDOSDfnGenerateCommand::buildPreview() const
{
    return m_parameters;
}

bool IDOSDfnGenerateCommand::apply(QString& error)
{
    if (m_data.isNull() && !result().isEmpty())
    {
        error = QObject::tr("The generated fracture set is unavailable.");
        return false;
    }

    if (m_data.isNull())
    {
        const IDOSGridObject* grid = qobject_cast<IDOSGridObject*>(project()->object(m_parameters.value(QStringLiteral("gridId")).toString()));
        const IDOSGridPropertyObject* density = qobject_cast<IDOSGridPropertyObject*>(project()->object(m_parameters.value(QStringLiteral("densityId")).toString()));
        QVariantList fractures;
        if (!IDOSDfnGenerator::generate(grid, density, m_parameters, fractures, error))
        {
            return false;
        }
        QScopedPointer<IDOSNaturalFractureSetObject> data(new IDOSNaturalFractureSetObject());
        data->setName(m_parameters.value(QStringLiteral("name")).toString().trimmed());
        // Copy nested strings out of plugin literal storage before publishing platform data.
        data->setFractures(QJsonArray::fromVariantList(fractures).toVariantList());
        for (const QVariant& value : data->geometryReport())
        {
            if (!value.toMap().value(QStringLiteral("valid")).toBool())
            {
                error = QObject::tr("The generated geometry is degenerate at this coordinate scale. Adjust the lengths or grid.");
                return false;
            }
        }
        m_data = data.take();
    }
    if (!project()->add(m_data.data()))
    {
        error = QObject::tr("The generated fracture set is unavailable.");
        return false;
    }
    QJsonObject result;
    result.insert(QStringLiteral("objectId"), m_data->objectId());
    result.insert(QStringLiteral("count"), m_data->fractures().size());
    result.insert(QStringLiteral("gridId"), m_parameters.value(QStringLiteral("gridId")));
    setResult(result);
    return true;
}

bool IDOSDfnGenerateCommand::revert(QString& error)
{
    if (m_data.isNull() || project()->object(m_data->objectId()) != m_data.data())
    {
        error = QObject::tr("The generated fracture set is unavailable.");
        return false;
    }
    project()->take(m_data->objectId());
    return true;
}

IDOSDfnGenerateCommandMetadata::IDOSDfnGenerateCommandMetadata()
    :
    IDOSCommandMetadata(QStringLiteral("fracture.dfn.generate"), QObject::tr("Generate DFN"),
        QObject::tr("Generate rectangular stochastic fractures from active grid cells."), IDOSCommand::Type::Action)
{
}

QJsonObject IDOSDfnGenerateCommandMetadata::schema() const
{
    QJsonObject properties;
    const QStringList strings = {QStringLiteral("gridId"), QStringLiteral("densityId"), QStringLiteral("name")};
    for (const QString& key : strings)
    {
        properties.insert(key, QJsonObject{{QStringLiteral("type"), QStringLiteral("string")}});
    }
    const QStringList integers = {QStringLiteral("count"), QStringLiteral("seed"), QStringLiteral("firstLayer"), QStringLiteral("lastLayer")};
    const QStringList reals = {QStringLiteral("minimumLength"), QStringLiteral("maximumLength"), QStringLiteral("aspectRatio"),
        QStringLiteral("strike"), QStringLiteral("dip"), QStringLiteral("strikeSpread"), QStringLiteral("dipSpread")};
    QJsonArray required{QStringLiteral("gridId"), QStringLiteral("name")};
    for (const QString& key : integers + reals)
    {
        properties.insert(key, QJsonObject{{QStringLiteral("type"), integers.contains(key) ? QStringLiteral("integer") : QStringLiteral("number")}});
        required.append(key);
    }
    return QJsonObject{{QStringLiteral("type"), QStringLiteral("object")},
        {QStringLiteral("properties"), properties}, {QStringLiteral("required"), required}};
}

IDOSCommand* IDOSDfnGenerateCommandMetadata::create(const QJsonObject& arguments, IDOSProject* project) const
{
    return new IDOSDfnGenerateCommand(project, arguments);
}
