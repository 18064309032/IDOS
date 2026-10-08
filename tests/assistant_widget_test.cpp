#include "assistant_widget_test.h"

#include "command/idoscommandmetadata.h"
#include "command/idoscommandmanager.h"
#include "command/idoscommandregistry.h"
#include "command/idoscreatecasecommand.h"
#include "command/idoscreatewellcommand.h"
#include "command/idosdeletecasecommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletepropertycommand.h"
#include "command/idosdeletewellcommand.h"
#include "command/idosimportcommands.h"
#include "idosassistantwidget.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "idoswell.h"

#include <memory>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaObject>
#include <QTextEdit>
#include <QtTest>

void AssistantWidgetTest::onToolCallCreatesWell()
{
    IDOSProject project;
    IDOSAssistantWidget widget;
    widget.setProject(&project);
    registerCommands(widget.commandRegistry());

    const QJsonObject response =
        createToolCallResponse(QStringLiteral("well.create"),
                               QStringLiteral("{\"name\":\"A10\"}"));

    const bool invoked =
        QMetaObject::invokeMethod(&widget,
                                  "onAssistantResponse",
                                  Qt::DirectConnection,
                                  Q_ARG(QJsonObject, response));
    QVERIFY(invoked);

    IDOSWell* well = nullptr;
    const QList<IDOSDataObject*> objects = project.objects();
    for (QList<IDOSDataObject*>::const_iterator iterator = objects.constBegin();
         iterator != objects.constEnd();
         ++iterator)
    {
        IDOSWell* candidate = qobject_cast<IDOSWell*>(*iterator);
        if (candidate != nullptr && candidate->name() == QStringLiteral("A10"))
        {
            well = candidate;
        }
    }

    QVERIFY(well != nullptr);
    QVERIFY(project.commandManager()->canUndo());
    QVERIFY(transcriptText(&widget).contains(QStringLiteral("\"success\":true")));
    QVERIFY(transcriptText(&widget).contains(QStringLiteral("\"command\":\"well.create\"")));
}

void AssistantWidgetTest::onTextCommandCallCreatesWell()
{
    IDOSProject project;
    IDOSAssistantWidget widget;
    widget.setProject(&project);
    registerCommands(widget.commandRegistry());

    const QJsonObject response =
        createTextResponse(QStringLiteral("```json well.create({\"name\":\"A10\"}) ```"));

    const bool invoked =
        QMetaObject::invokeMethod(&widget,
                                  "onAssistantResponse",
                                  Qt::DirectConnection,
                                  Q_ARG(QJsonObject, response));
    QVERIFY(invoked);

    IDOSWell* well = nullptr;
    const QList<IDOSDataObject*> objects = project.objects();
    for (QList<IDOSDataObject*>::const_iterator iterator = objects.constBegin();
         iterator != objects.constEnd();
         ++iterator)
    {
        IDOSWell* candidate = qobject_cast<IDOSWell*>(*iterator);
        if (candidate != nullptr && candidate->name() == QStringLiteral("A10"))
        {
            well = candidate;
        }
    }

    QVERIFY(well != nullptr);
    QVERIFY(project.commandManager()->canUndo());
    QVERIFY(transcriptText(&widget).contains(QStringLiteral("\"success\":true")));
    QVERIFY(transcriptText(&widget).contains(QStringLiteral("\"command\":\"well.create\"")));
}

void AssistantWidgetTest::onInvalidToolArgumentsDoNotCreateCommand()
{
    IDOSProject project;
    IDOSAssistantWidget widget;
    widget.setProject(&project);
    registerCommands(widget.commandRegistry());

    const QJsonObject response =
        createToolCallResponse(QStringLiteral("well.create"),
                               QStringLiteral("{\"name\""));

    const bool invoked =
        QMetaObject::invokeMethod(&widget,
                                  "onAssistantResponse",
                                  Qt::DirectConnection,
                                  Q_ARG(QJsonObject, response));
    QVERIFY(invoked);

    QCOMPARE(project.objects().size(), 0);
    QVERIFY(!project.commandManager()->canUndo());
    QVERIFY(transcriptText(&widget).contains(
        QStringLiteral("The assistant provided invalid command arguments.")));
}

void AssistantWidgetTest::onToolCallWithoutProjectIsReported()
{
    IDOSAssistantWidget widget;
    registerCommands(widget.commandRegistry());

    const QJsonObject response =
        createToolCallResponse(QStringLiteral("well.create"),
                               QStringLiteral("{\"name\":\"A10\"}"));

    const bool invoked =
        QMetaObject::invokeMethod(&widget,
                                  "onAssistantResponse",
                                  Qt::DirectConnection,
                                  Q_ARG(QJsonObject, response));
    QVERIFY(invoked);

    QVERIFY(transcriptText(&widget).contains(
        QStringLiteral("No project is available for command execution.")));
}

void AssistantWidgetTest::registerCommands(IDOSCommandRegistry* registry) const
{
    QVERIFY(registry != nullptr);

    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateCaseCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSCreateWellCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteCaseCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteGridCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeletePropertyCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSDeleteWellCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportCaseCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportGridCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportPropertyCommandMetadata()));
    registry->add(std::unique_ptr<IDOSCommandMetadata>(new IDOSImportWellDataCommandMetadata()));
}

QJsonObject AssistantWidgetTest::createToolCallResponse(const QString& commandName,
                                                        const QString& argumentsText) const
{
    QJsonObject functionObject;
    functionObject.insert(QStringLiteral("name"), commandName);
    functionObject.insert(QStringLiteral("arguments"), argumentsText);

    QJsonObject toolCall;
    toolCall.insert(QStringLiteral("type"), QStringLiteral("function"));
    toolCall.insert(QStringLiteral("function"), functionObject);

    QJsonArray toolCalls;
    toolCalls.append(toolCall);

    QJsonObject message;
    message.insert(QStringLiteral("role"), QStringLiteral("assistant"));
    message.insert(QStringLiteral("content"), QJsonValue());
    message.insert(QStringLiteral("tool_calls"), toolCalls);

    QJsonObject choice;
    choice.insert(QStringLiteral("index"), 0);
    choice.insert(QStringLiteral("message"), message);

    QJsonArray choices;
    choices.append(choice);

    QJsonObject response;
    response.insert(QStringLiteral("choices"), choices);
    return response;
}

QJsonObject AssistantWidgetTest::createTextResponse(const QString& text) const
{
    QJsonObject message;
    message.insert(QStringLiteral("role"), QStringLiteral("assistant"));
    message.insert(QStringLiteral("content"), text);

    QJsonObject choice;
    choice.insert(QStringLiteral("index"), 0);
    choice.insert(QStringLiteral("message"), message);

    QJsonArray choices;
    choices.append(choice);

    QJsonObject response;
    response.insert(QStringLiteral("choices"), choices);
    return response;
}

QString AssistantWidgetTest::transcriptText(IDOSAssistantWidget* widget) const
{
    QVERIFY(widget != nullptr);

    QTextEdit* transcript = widget->findChild<QTextEdit*>();
    QVERIFY(transcript != nullptr);
    return transcript->toPlainText();
}

QTEST_MAIN(AssistantWidgetTest)
