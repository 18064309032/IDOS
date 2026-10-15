#ifndef IDOS_DELETE_DATA_COMMAND_H
#define IDOS_DELETE_DATA_COMMAND_H

#include <QHash>
#include <QList>
#include <QPointer>
#include <QStringList>

#include "data/case/idoscaseitemref.h"
#include "idos_app.h"
#include "idoscommand.h"
#include "idoscommandmetadata.h"

class IDOSDataObject;

class APP_EXPORT IDOSDeleteDataCommand : public IDOSCommand
{
  public:
    IDOSDeleteDataCommand(IDOSProject* project, const QStringList& objectIds);
    ~IDOSDeleteDataCommand() override;
    static bool supportsObject(const IDOSDataObject* object);

  protected:
    bool validateCommand(QString& error) const override;
    QJsonObject buildPreview() const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;

  private:
    void restoreReferences();
    QStringList m_objectIds;
    QList<QPointer<IDOSDataObject>> m_objects;
    QHash<QString, QList<IDOSCaseItemRef>> m_caseReferences;
};

class APP_EXPORT IDOSDeleteDataCommandMetadata : public IDOSCommandMetadata
{
  public:
    IDOSDeleteDataCommandMetadata();
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
};

#endif // IDOS_DELETE_DATA_COMMAND_H
