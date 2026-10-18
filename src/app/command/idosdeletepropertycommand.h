#ifndef IDOS_DELETE_PROPERTY_COMMAND_H
#define IDOS_DELETE_PROPERTY_COMMAND_H

#include <QHash>
#include <QList>
#include <QPointer>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "data/case/idoscaseitemref.h"
#include "idos_app.h"

class IDOSGridPropertyObject;

class APP_EXPORT IDOSDeletePropertyCommand : public IDOSCommand
{
public:
    IDOSDeletePropertyCommand(IDOSProject* project,
                              QString propertyId);
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
    QPointer<IDOSGridPropertyObject> m_property;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
};

class APP_EXPORT IDOSDeletePropertyCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSDeletePropertyCommandMetadata();
    ~IDOSDeletePropertyCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif // IDOS_DELETE_PROPERTY_COMMAND_H
