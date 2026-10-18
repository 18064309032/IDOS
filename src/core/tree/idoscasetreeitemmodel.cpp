#include "idoscasetreeitemmodel.h"

IDOSCaseTreeItemModel::IDOSCaseTreeItemModel(IDOSTreeItemProviderRegistry* registry, QObject* parent)
    :
    IDOSTreeModel(registry, parent)
{
    setName(QStringLiteral("case"));
}

bool IDOSCaseTreeItemModel::showGridSettings(const QString& itemId, const QString& session, const QString& gridId,
                                             const QString& filterId, const QString& propertyId, bool openFilter)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit showGridSettingsRequested(gridId, filterId, propertyId, openFilter);
    return true;
}

bool IDOSCaseTreeItemModel::displayGridProperty(const QString& itemId, const QString& session, const QString& gridId,
                                                const QString& propertyId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit displayGridPropertyRequested(gridId, propertyId);
    return true;
}

bool IDOSCaseTreeItemModel::showWellSettings(const QString& itemId, const QString& session, const QString& wellId,
                                             bool trajectory)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit showWellSettingsRequested(wellId, trajectory);
    return true;
}

bool IDOSCaseTreeItemModel::rename(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit renameRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::importGridProperty(const QString& itemId, const QString& session, const QString& dataId,
                                               const QString& caseId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importGridPropertyRequested(dataId, caseId);
    return true;
}

bool IDOSCaseTreeItemModel::importGridDynamicProperties(const QString& itemId, const QString& session,
                                                        const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importGridDynamicPropertiesRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::deleteGridProperty(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit deleteGridPropertyRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::deleteGrid(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit deleteGridRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::deleteGridFilter(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit deleteGridFilterRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::renameGridFilter(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit renameGridFilterRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::viewWellData(const QString& itemId, const QString& session, const QString& dataId,
                                         const QString& part, const QString& itemKey)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit viewWellDataRequested(dataId, part, itemKey);
    return true;
}

bool IDOSCaseTreeItemModel::importWell(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::deleteWell(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit deleteWellRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::generateInput(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit generateInputRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::viewPropertyCurves(const QString& itemId, const QString& session, const QString& caseId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit viewPropertyCurvesRequested(caseId);
    return true;
}

bool IDOSCaseTreeItemModel::importSummaryResults(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importSummaryResultsRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::viewSummaryResults(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit viewSummaryResultsRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::renameCase(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit renameCaseRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::newCase(const QString& session)
{
    if (!acceptsContext(session, QString()))
    {
        return false;
    }
    emit newCaseRequested();
    return true;
}

bool IDOSCaseTreeItemModel::importCase(const QString& session)
{
    if (!acceptsContext(session, QString()))
    {
        return false;
    }
    emit importCaseRequested();
    return true;
}

bool IDOSCaseTreeItemModel::importGrid(const QString& itemId, const QString& session, const QString& caseId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importGridRequested(caseId);
    return true;
}

bool IDOSCaseTreeItemModel::deleteCase(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit deleteCaseRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::importWellHeader(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellHeaderRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::importWellTrajectory(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellTrajectoryRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::importWellCurves(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellCurvesRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::importWellMarkers(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellMarkersRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::importWellCompletions(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellCompletionsRequested(dataId);
    return true;
}

bool IDOSCaseTreeItemModel::importWellCheckshots(const QString& itemId, const QString& session, const QString& dataId)
{
    if (itemId.isEmpty() || !acceptsContext(session, itemId))
    {
        return false;
    }
    emit importWellCheckshotsRequested(dataId);
    return true;
}
