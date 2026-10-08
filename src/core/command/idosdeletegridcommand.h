#ifndef IDOS_DELETE_GRID_COMMAND_H
#define IDOS_DELETE_GRID_COMMAND_H

#include <QHash>
#include <QList>
#include <QPointer>
#include <QStringList>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idoscaseitemref.h"

class IDOSGrid;
class IDOSGridProperty;

class CORE_EXPORT IDOSDeleteGridCommand : public IDOSCommand
{
public:
    IDOSDeleteGridCommand(IDOSProject* project,
                          QString gridId,
                          QUndoCommand* parent = nullptr);
    ~IDOSDeleteGridCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    void captureCaseReferences();
    void removeCaseReferencesForObject(const QString& objectId);
    void restoreCaseReferences();
    QStringList collectPropertyIds() const;
    void clearDetachedObjects();

    QString m_gridId;
    QPointer<IDOSGrid> m_grid;
    QList<QPointer<IDOSGridProperty>> m_properties;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
};

class CORE_EXPORT IDOSDeleteGridCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSDeleteGridCommandMetadata();
    ~IDOSDeleteGridCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_DELETE_GRID_COMMAND_H
