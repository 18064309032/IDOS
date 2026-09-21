#include "idosmodel.h"

IDOSModel::IDOSModel(QObject* parent)
    : IDOSDataObject(parent)
{
}

IDOSModel::~IDOSModel() = default;

QString IDOSModel::gridId() const { return m_gridId; }

void IDOSModel::setGridId(const QString& gridId)
{
    if (m_gridId == gridId) return;
    m_gridId = gridId;
    emit gridRefChanged();
    emit dataChanged();
}

const QStringList& IDOSModel::wellIds() const { return m_wellIds; }

void IDOSModel::addWellRef(const QString& wellId)
{
    if (wellId.isEmpty() || m_wellIds.contains(wellId)) return;

    m_wellIds.append(wellId);
    emit wellRefsChanged();
    emit dataChanged();
}

void IDOSModel::removeWellRef(const QString& wellId)
{
    if (!m_wellIds.contains(wellId)) return;

    m_wellIds.removeAll(wellId);
    emit wellRefsChanged();
    emit dataChanged();
}

bool IDOSModel::references(const QString& objectId) const
{
    return objectId == m_gridId || m_wellIds.contains(objectId);
}
