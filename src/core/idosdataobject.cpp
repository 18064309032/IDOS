#include "idosdataobject.h"

IDOSDataObject::IDOSDataObject(QObject* parent)
    : IDOSObject(parent)
    , m_visible(true)
{
}

IDOSDataObject::~IDOSDataObject()
{
}

bool IDOSDataObject::isVisible() const
{
    return m_visible;
}

void IDOSDataObject::setVisible(bool visible)
{
    if (m_visible == visible)
    {
        return;
    }
    m_visible = visible;
    emit visibilityChanged(m_visible);
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

QStringList IDOSDataObject::pendingImportPaths() const
{
    return {};
}
