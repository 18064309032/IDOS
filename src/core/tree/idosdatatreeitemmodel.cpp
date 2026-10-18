#include "idosdatatreeitemmodel.h"

IDOSDataTreeItemModel::IDOSDataTreeItemModel(IDOSTreeItemProviderRegistry* registry, QObject* parent)
    :
    IDOSTreeModel(registry, parent)
{
    setName(QStringLiteral("data"));
}

bool IDOSDataTreeItemModel::importDataset(const QString& itemId, const QString& session, const QString& text,
                                          const QString& fileFilter, const QString& providerId,
                                          const QString& dataTypeId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importDatasetRequested(text, fileFilter, providerId, dataTypeId);
    return true;
}

bool IDOSDataTreeItemModel::selectSeismicInline(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit selectSeismicInlineRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::importFluidRockTables(const QString& itemId, const QString& session,
                                                  const QString& category)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importFluidRockTablesRequested(category);
    return true;
}

bool IDOSDataTreeItemModel::addFluidRockTableToCase(const QString& itemId, const QString& session,
                                                    const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit addFluidRockTableToCaseRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::showGridSettings(const QString& itemId, const QString& session, const QString& gridId,
                                             const QString& filterId, const QString& propertyId, bool openFilter)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit showGridSettingsRequested(gridId, filterId, propertyId, openFilter);
    return true;
}

bool IDOSDataTreeItemModel::displayGridProperty(const QString& itemId, const QString& session, const QString& gridId,
                                                const QString& propertyId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit displayGridPropertyRequested(gridId, propertyId);
    return true;
}

bool IDOSDataTreeItemModel::showWellSettings(const QString& itemId, const QString& session, const QString& wellId,
                                             bool trajectory)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit showWellSettingsRequested(wellId, trajectory);
    return true;
}

bool IDOSDataTreeItemModel::removeObjects(const QString& itemId, const QString& session, const QStringList& objectIds)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit removeRequested(objectIds);
    return true;
}

bool IDOSDataTreeItemModel::addToCase(const QString& itemId, const QString& session, const QJsonArray& items)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit addToCaseRequested(items);
    return true;
}

bool IDOSDataTreeItemModel::rename(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit renameRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::importGridProperty(const QString& itemId, const QString& session, const QString& dataId,
                                               const QString& caseId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importGridPropertyRequested(dataId, caseId);
    return true;
}

bool IDOSDataTreeItemModel::importGridDynamicProperties(const QString& itemId, const QString& session,
                                                        const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importGridDynamicPropertiesRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::deleteGridProperty(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit deleteGridPropertyRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::deleteGrid(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit deleteGridRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::deleteGridFilter(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit deleteGridFilterRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::renameGridFilter(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit renameGridFilterRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::viewWellData(const QString& itemId, const QString& session, const QString& dataId,
                                         const QString& part, const QString& itemKey)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit viewWellDataRequested(dataId, part, itemKey);
    return true;
}

bool IDOSDataTreeItemModel::newWell(const QString& itemId, const QString& session)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit newWellRequested();
    return true;
}

bool IDOSDataTreeItemModel::importWell(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::deleteWell(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit deleteWellRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::importWellHeader(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellHeaderRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::importWellTrajectory(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellTrajectoryRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::importWellCurves(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellCurvesRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::importWellMarkers(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellMarkersRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::importWellCompletions(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellCompletionsRequested(dataId);
    return true;
}

bool IDOSDataTreeItemModel::importWellCheckshots(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellCheckshotsRequested(dataId);
    return true;
}
