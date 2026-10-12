#include <QList>

#include "idosdatametadata.h"

#include "idosinterface.h"

IDOSInterface::IDOSInterface(QObject* parent)
    : QObject(parent)
{
}

IDOSInterface::~IDOSInterface()
{
}

QList<const IDOSDataMetadata*> IDOSInterface::treeDataMetadata() const
{
    return QList<const IDOSDataMetadata*>();
}
