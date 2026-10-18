#ifndef IDOS_CASE_REFERENCE_COMMAND_H
#define IDOS_CASE_REFERENCE_COMMAND_H

#include <QHash>
#include <QJsonArray>
#include <QList>

#include "command/idoscommand.h"
#include "command/idoscommandmetadata.h"
#include "data/case/idoscaseitemref.h"
#include "idos_app.h"

class APP_EXPORT IDOSCaseReferenceCommand : public IDOSCommand
{
public:
    IDOSCaseReferenceCommand(IDOSProject* project, const QString& operation,
                             const QJsonObject& arguments);
    static bool matches(const IDOSCaseItemRef& reference, const QJsonObject& item);
protected:
    bool validateCommand(QString& error) const override;
    bool apply(QString& error) override;
    bool revert(QString& error) override;
private:
    bool supportsObject(const QJsonObject& item) const;
    bool prepare(QHash<QString, QList<IDOSCaseItemRef>>& before,
                 QHash<QString, QList<IDOSCaseItemRef>>& after, QString& error) const;
    bool checkState(const QHash<QString, QList<IDOSCaseItemRef>>& expected, QString& error) const;
    bool writeState(const QHash<QString, QList<IDOSCaseItemRef>>& state, QString& error);
    bool sameList(const QList<IDOSCaseItemRef>& first, const QList<IDOSCaseItemRef>& second) const;
    QString m_operation;
    QJsonObject m_arguments;
    QHash<QString, QList<IDOSCaseItemRef>> m_before;
    QHash<QString, QList<IDOSCaseItemRef>> m_after;
    bool m_initialized;
};

class APP_EXPORT IDOSCaseReferenceCommandMetadata : public IDOSCommandMetadata
{
public:
    explicit IDOSCaseReferenceCommandMetadata(const QString& operation);
    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments, IDOSProject* project) const override;
private:
    QString m_operation;
};

class APP_EXPORT IDOSAttachFluidRockTableCommandMetadata : public IDOSCommandMetadata
{
public:
    IDOSAttachFluidRockTableCommandMetadata();
    ~IDOSAttachFluidRockTableCommandMetadata() override;

    QJsonObject schema() const override;
    IDOSCommand* create(const QJsonObject& arguments,
                        IDOSProject* project) const override;
};

#endif
