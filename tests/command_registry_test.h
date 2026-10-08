#ifndef COMMAND_REGISTRY_TEST_H
#define COMMAND_REGISTRY_TEST_H

#include <QObject>

class IDOSCommandRegistry;

class CommandRegistryTest : public QObject
{
    Q_OBJECT

private slots:
    void onDefaultRegistryContainsCreateWell();
    void onExecuteCreateWellFromJson();
    void onExecuteCreateCaseFromJson();
    void onExecuteDeleteWellFromJson();
    void onExecuteDeletePropertyFromJson();
    void onExecuteDeleteGridFromJson();
    void onExecuteDeleteCaseFromJson();
    void onExecuteRenameObjectFromJson();

private:
    void registerAllCommands(IDOSCommandRegistry* registry) const;
};

#endif // COMMAND_REGISTRY_TEST_H
