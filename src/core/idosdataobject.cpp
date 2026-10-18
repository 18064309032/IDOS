#include "idosproject.h"

#include "idosdataobject.h"

IDOSDataObject::IDOSDataObject(QObject* parent)
    :
    IDOSObject(parent)
{
}

IDOSDataObject::~IDOSDataObject()
{
}

QString IDOSDataObject::containerId() const
{
    return QString();
}

void IDOSDataObject::mergeFrom(const IDOSDataObject* other)
{
    (void)other;
}

void IDOSDataObject::resolveReferences(IDOSProject* project)
{
    (void)project;
}


bool IDOSDataObject::canClone() const
{
    return false;
}

IDOSDataObject* IDOSDataObject::clone() const
{
    return nullptr;
}

QVariantMap IDOSDataObject::projectData() const
{
    return QVariantMap();
}

bool IDOSDataObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    Q_UNUSED(data);
    error = tr("This object type does not support project storage.");
    return false;
}

bool IDOSDataObject::validateProjectReferences(const IDOSProject* project, QString& error) const
{
    if (!containerId().isEmpty() && project->object(containerId()) == nullptr)
    {
        error = tr("An object refers to a missing container.");
        return false;
    }
    return true;
}
