#ifndef IDOS_CASE_TREE_ITEM_MODEL_H
#define IDOS_CASE_TREE_ITEM_MODEL_H

#include <QJsonArray>
#include <QString>
#include <QStringList>

#include "idostreemodel.h"

class CORE_EXPORT IDOSCaseTreeItemModel : public IDOSTreeModel
{
    Q_OBJECT

  public:
    explicit IDOSCaseTreeItemModel(IDOSTreeItemProviderRegistry* registry, QObject* parent = nullptr);
    bool showGridSettings(const QString& itemId, const QString& session, const QString& gridId, const QString& filterId,
                          const QString& propertyId, bool openFilter);
    bool displayGridProperty(const QString& itemId, const QString& session, const QString& gridId,
                             const QString& propertyId);
    bool showWellSettings(const QString& itemId, const QString& session, const QString& wellId, bool trajectory);
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
    bool importWell(const QString& itemId, const QString& session, const QString& dataId);
    bool deleteWell(const QString& itemId, const QString& session, const QString& dataId);
    bool generateInput(const QString& itemId, const QString& session, const QString& dataId);
    bool viewPropertyCurves(const QString& itemId, const QString& session, const QString& caseId);
    bool importSummaryResults(const QString& itemId, const QString& session, const QString& dataId);
    bool viewSummaryResults(const QString& itemId, const QString& session, const QString& dataId);
    bool renameCase(const QString& itemId, const QString& session, const QString& dataId);
    bool newCase(const QString& session);
    bool importCase(const QString& session);
    bool importGrid(const QString& itemId, const QString& session, const QString& caseId);
    bool deleteCase(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellHeader(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellTrajectory(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellCurves(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellMarkers(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellCompletions(const QString& itemId, const QString& session, const QString& dataId);

    bool importWellCheckshots(const QString& itemId, const QString& session, const QString& dataId);

  signals:
    void showGridSettingsRequested(const QString& gridId, const QString& filterId, const QString& propertyId,
                                   bool openFilter);
    void displayGridPropertyRequested(const QString& gridId, const QString& propertyId);
    void showWellSettingsRequested(const QString& wellId, bool trajectory);
    void renameRequested(const QString& dataId);
    void importGridPropertyRequested(const QString& dataId, const QString& caseId);
    void importGridDynamicPropertiesRequested(const QString& dataId);
    void deleteGridPropertyRequested(const QString& dataId);
    void deleteGridRequested(const QString& dataId);
    void deleteGridFilterRequested(const QString& dataId);
    void renameGridFilterRequested(const QString& dataId);
    void viewWellDataRequested(const QString& dataId, const QString& part, const QString& itemKey);
    void importWellRequested(const QString& dataId);
    void deleteWellRequested(const QString& dataId);
    void generateInputRequested(const QString& dataId);
    void viewPropertyCurvesRequested(const QString& caseId);
    void importSummaryResultsRequested(const QString& dataId);
    void viewSummaryResultsRequested(const QString& dataId);
    void renameCaseRequested(const QString& dataId);
    void newCaseRequested();
    void importCaseRequested();
    void importGridRequested(const QString& caseId);
    void deleteCaseRequested(const QString& dataId);
    void importWellHeaderRequested(const QString& dataId);
    void importWellTrajectoryRequested(const QString& dataId);
    void importWellCurvesRequested(const QString& dataId);
    void importWellMarkersRequested(const QString& dataId);
    void importWellCompletionsRequested(const QString& dataId);
    void importWellCheckshotsRequested(const QString& dataId);
};

#endif
