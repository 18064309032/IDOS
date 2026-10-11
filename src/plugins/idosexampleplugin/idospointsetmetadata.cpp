#include <QObject>

#include "idospointset.h"

#include "idospointsetmetadata.h"

QString IDOSPointSetMetadata::typeId() const
{
    return QString::fromLatin1("idos.example.pointset");
}

QString IDOSPointSetMetadata::displayName() const
{
    return QObject::tr("Point Set");
}

IDOSDataObject* IDOSPointSetMetadata::create() const
{
    for (int index = m_objects.size() - 1; index >= 0; --index)
    {
        if (m_objects.at(index).isNull())
        {
            m_objects.removeAt(index);
        }
    }
    IDOSDataObject* object = new IDOSPointSet();
    m_objects.append(QPointer<IDOSDataObject>(object));
    return object;
}

bool IDOSPointSetMetadata::isInUse() const
{
    for (const QPointer<IDOSDataObject>& object : m_objects)
    {
        if (object != nullptr)
        {
            return true;
        }
    }
    return false;
}
