#include "idosmodel.h"
#include "idosproject.h"

#include <QFileInfo>
#include <QString>

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

QStringList IDOSModel::pendingWellNames() const { return m_pendingWellNames; }

void IDOSModel::setPendingWellNames(const QStringList& names)
{
    m_pendingWellNames = names;
}

QString IDOSModel::pendingGridPath() const { return m_pendingGridPath; }

void IDOSModel::setPendingGridPath(const QString& path)
{
    m_pendingGridPath = path;
}

QStringList IDOSModel::pendingImportPaths() const
{
    if (m_pendingGridPath.isEmpty()) return {};
    return QStringList{ m_pendingGridPath };
}

void IDOSModel::resolveReferences(IDOSProject* project)
{
    if (project == nullptr) return;

    // ① gridId：Coordinator 已递归把 .EGRID 加进 Project（同名 basename），
    //    现在按 basename 查找 grid，命中则 setGridId 并清 pending path
    if (!m_pendingGridPath.isEmpty())
    {
        QString gridName = QFileInfo(m_pendingGridPath).completeBaseName();
        IDOSDataObject* grid = project->objectByName(gridName);
        if (grid != nullptr)
        {
            setGridId(grid->objectId());   // setGridId 已发信号
            m_pendingGridPath.clear();
        }
        // 未命中：留 pending path 等下次导入补链（不应发生，因 Coordinator 已先递归）
    }

    // ② wellIds：井名 → objectId
    if (!m_pendingWellNames.isEmpty())
    {
        QStringList unresolved;
        for (const QString& wellName : m_pendingWellNames)
        {
            IDOSDataObject* well = project->objectByName(wellName);
            if (well != nullptr)
            {
                addWellRef(well->objectId());   // 命中：转 objectId（addWellRef 已发信号）
            }
            else
            {
                unresolved.append(wellName);   // 未命中：留 pending 等下次导入补链
            }
        }
        m_pendingWellNames = unresolved;
    }
}
