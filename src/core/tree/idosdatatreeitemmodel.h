#ifndef IDOS_DATA_TREE_ITEM_MODEL_H
#define IDOS_DATA_TREE_ITEM_MODEL_H

#include <QJsonArray>
#include <QString>
#include <QStringList>

#include "idostreemodel.h"

class CORE_EXPORT IDOSDataTreeItemModel : public IDOSTreeModel
{
    Q_OBJECT

  public:
    explicit IDOSDataTreeItemModel(IDOSTreeItemProviderRegistry* registry, QObject* parent = nullptr);
    bool importDataset(const QString& itemId, const QString& session, const QString& text, const QString& fileFilter,
                       const QString& providerId, const QString& dataTypeId);
    bool selectSeismicInline(const QString& itemId, const QString& session, const QString& dataId);
    bool importFluidRockTables(const QString& itemId, const QString& session, const QString& category);
    bool addFluidRockTableToCase(const QString& itemId, const QString& session, const QString& dataId);
    bool showGridSettings(const QString& itemId, const QString& session, const QString& gridId, const QString& filterId,
                          const QString& propertyId, bool openFilter);
    bool displayGridProperty(const QString& itemId, const QString& session, const QString& gridId,
                             const QString& propertyId);
    bool showWellSettings(const QString& itemId, const QString& session, const QString& wellId, bool trajectory);
    bool removeObjects(const QString& itemId, const QString& session, const QStringList& objectIds);
    bool addToCase(const QString& itemId, const QString& session, const QJsonArray& items);
    bool rename(const QString& itemId, const QString& session, const QString& dataId);
    bool importGridProperty(const QString& itemId, const QString& session, const QString& dataId,
                            const QString& caseId);
    bool importGridDynamicProperties(const QString& itemId, const QString& session, const QString& dataId);
    bool deleteGridProperty(const QString& itemId, const QString& session, const QString& dataId);
    bool deleteGrid(const QString& itemId, const QString& session, const QString& dataId);
    bool deleteGridFilter(const QString& itemId, const QString& session, const QString& dataId);
    bool renameGridFilter(const QString& itemId, const QString& session, const QString& dataId);
    bool viewWellData(const QString& itemId, const QString& session, const QString& dataId, const QString& part,
                      const QString& itemKey);
    bool newWell(const QString& itemId, const QString& session);
    bool importWell(const QString& itemId, const QString& session, const QString& dataId);
    bool deleteWell(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellHeader(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellTrajectory(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellCurves(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellMarkers(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellCompletions(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellCheckshots(const QString& itemId, const QString& session, const QString& dataId);

  signals:
    void importDatasetRequested(const QString& text, const QString& fileFilter, const QString& providerId,
                                const QString& dataTypeId);
    void selectSeismicInlineRequested(const QString& dataId);
    void importFluidRockTablesRequested(const QString& category);
    void addFluidRockTableToCaseRequested(const QString& dataId);
    void showGridSettingsRequested(const QString& gridId, const QString& filterId, const QString& propertyId,
                                   bool openFilter);
    void displayGridPropertyRequested(const QString& gridId, const QString& propertyId);
    void showWellSettingsRequested(const QString& wellId, bool trajectory);
    void removeRequested(const QStringList& objectIds);
    void addToCaseRequested(const QJsonArray& items);
    void renameRequested(const QString& dataId);
    void importGridPropertyRequested(const QString& dataId, const QString& caseId);
    void importGridDynamicPropertiesRequested(const QString& dataId);
    void deleteGridPropertyRequested(const QString& dataId);
    void deleteGridRequested(const QString& dataId);
    void deleteGridFilterRequested(const QString& dataId);
    void renameGridFilterRequested(const QString& dataId);
    void viewWellDataRequested(const QString& dataId, const QString& part, const QString& itemKey);
    void newWellRequested();
    void importWellRequested(const QString& dataId);
    void deleteWellRequested(const QString& dataId);
    void importWellHeaderRequested(const QString& dataId);
    void importWellTrajectoryRequested(const QString& dataId);
    void importWellCurvesRequested(const QString& dataId);
    void importWellMarkersRequested(const QString& dataId);
    void importWellCompletionsRequested(const QString& dataId);
    void importWellCheckshotsRequested(const QString& dataId);
};

#endif
