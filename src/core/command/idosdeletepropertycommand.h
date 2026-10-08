#ifndef IDOS_DELETE_PROPERTY_COMMAND_H
#define IDOS_DELETE_PROPERTY_COMMAND_H

#include <QHash>
#include <QList>
#include <QPointer>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "idoscaseitemref.h"

class IDOSGridProperty;

class CORE_EXPORT IDOSDeletePropertyCommand : public IDOSCommand
{
public:
    IDOSDeletePropertyCommand(IDOSProject* project,
                              QString propertyId,
                              QUndoCommand* parent = nullptr);
    ~IDOSDeletePropertyCommand() override;

protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

private:
    void captureCaseReferences();
    void removeCaseReferences();
    void restoreCaseReferences();

    QString m_propertyId;
    QPointer<IDOSGridProperty> m_property;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
};

class CORE_EXPORT IDOSDeletePropertyCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSDeletePropertyCommandMetadata();
    ~IDOSDeletePropertyCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_DELETE_PROPERTY_COMMAND_H
