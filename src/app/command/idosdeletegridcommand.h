#ifndef IDOS_DELETE_GRID_COMMAND_H
#define IDOS_DELETE_GRID_COMMAND_H

#include <QHash>
#include <QList>
#include <QPointer>
#include <QStringList>

#include "data/case/idoscaseitemref.h"
#include "idos_app.h"
#include "idoscommand.h"
#include "idoscommandmetadata.h"

class IDOSGrid;
class IDOSGridFilter;
class IDOSGridProperty;

class APP_EXPORT IDOSDeleteGridCommand : public IDOSCommand
{
public:
    IDOSDeleteGridCommand(IDOSProject* project,
                          QString gridId);
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
    QList<QPointer<IDOSGridFilter>> m_filters;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
};

class APP_EXPORT IDOSDeleteGridCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSDeleteGridCommandMetadata();
    ~IDOSDeleteGridCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_DELETE_GRID_COMMAND_H
