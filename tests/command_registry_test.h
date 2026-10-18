#ifndef COMMAND_REGISTRY_TEST_H
#define COMMAND_REGISTRY_TEST_H

#include <QObject>
#include <QVariant>

class IDOSCommandRegistry;
class IDOSSimulationCaseObject;

class CommandRegistryTest : public QObject
{
    Q_OBJECT

  public:
    CommandRegistryTest();
    static void initMain();

private slots:
    void onProjectOwnershipAndHistory();
    void onExistingProjectPerformance();
    void onStoragePerformance();
    void onLegacyProjectStorage();
    void onProjectWindowStorage();
    void onProjectPresentationRoundTrip();
    void onInvalidPresentationState();
    void onWindowPreferences();
    void onProjectStorageRoundTrip();
    void onProjectStorageFailureProtection();
    void onObjectCopies();
    void onPropertyMoves();
    void onCaseReferencesAtomic();
    void onCaseReferenceParts();
    void onDataRegistryOwnsRegisteredMetadata();
    void onFailedCommandPreservesRedo();
    void onDefaultRegistryContainsCreateWell();
    void onExecuteCreateWellFromJson();
    void onExecuteCreateCaseFromJson();
    void onExecuteDeleteWellFromJson();
    void onExecuteDeletePropertyFromJson();
    void onExecuteDeleteGridFromJson();
    void onExecuteDeleteCaseFromJson();
    void onExecuteRenameObjectFromJson();

private:
    bool sameProjectValue(const QVariant& first, const QVariant& second) const;
    void onReferencesPublished();
    void registerAllCommands(IDOSCommandRegistry* registry) const;
    IDOSSimulationCaseObject* m_observedSource;
    IDOSSimulationCaseObject* m_observedTarget;
    bool m_referencesAtomic;
    int m_referenceNotifications;
};

#endif // COMMAND_REGISTRY_TEST_H
