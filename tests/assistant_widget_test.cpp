#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextBrowser>
#include <QUrl>
#include <QtTest>

#include "command/idoscommandmanager.h"
#include "command/idoscommandmetadata.h"
#include "command/idoscommandregistry.h"
#include "command/idoscreatecasecommand.h"
#include "command/idoscreatewellcommand.h"
#include "command/idosdeletecasecommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletepropertycommand.h"
#include "command/idosdeletewellcommand.h"
#include "command/idosimportcasecommand.h"
#include "command/idosimportgridcommand.h"
#include "command/idosimportgridpropertycommand.h"
#include "command/idosimportwelldatacommand.h"
#include "command/idosinspectobjectcommand.h"
#include "command/idoslistobjectscommand.h"
#include "data/case/idoscaseitemref.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/well/idoswellobject.h"
#include "idosassistantservice.h"
#include "idosassistantwidget.h"
#include "idosdataobject.h"
#include "idoslocalmodelservice.h"
#include "idosproject.h"

#include "assistant_widget_test.h"

AssistantWidgetTest::AssistantWidgetTest()
    :
    m_server(new QTcpServer(this))
    , m_repeat(false)
{
    connect(m_server, &QTcpServer::newConnection, this, &AssistantWidgetTest::onNewConnection);
}

void AssistantWidgetTest::configureModel(IDOSAssistantWidget* widget)
{
    const QList<QTcpSocket*> sockets = m_buffers.keys();
    for (QTcpSocket* socket : sockets)
    {
        disconnect(socket, nullptr, this, nullptr);
        socket->abort();
        socket->deleteLater();
    }
    m_buffers.clear();
    m_requests.clear();
    m_responses.clear();
    m_repeat = false;
    m_interruptService.clear();
    if (!m_server->isListening())
    {
        QVERIFY(m_server->listen(QHostAddress::LocalHost, 0));
    }
    IDOSLocalModelService* model = widget->service()->findChild<IDOSLocalModelService*>();
    QVERIFY(model != nullptr);
    model->setBaseUrl(QUrl(QStringLiteral("http://127.0.0.1:%1").arg(m_server->serverPort())));
}

void AssistantWidgetTest::onNewConnection()
{
    while (m_server->hasPendingConnections())
    {
        QTcpSocket* socket = m_server->nextPendingConnection();
        m_buffers.insert(socket, QByteArray());
        connect(socket, &QTcpSocket::readyRead, this, &AssistantWidgetTest::onSocketReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &AssistantWidgetTest::onSocketDisconnected);
    }
}

void AssistantWidgetTest::onSocketReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    QVERIFY(socket != nullptr);
    QByteArray& buffer = m_buffers[socket];
    buffer.append(socket->readAll());
    const int end = buffer.indexOf("\r\n\r\n");
    if (end < 0)
    {
        return;
    }
    int length = 0;
    const QList<QByteArray> headers = buffer.left(end).split('\n');
    for (const QByteArray& header : headers)
    {
        if (header.toLower().startsWith("content-length:"))
        {
            length = header.mid(header.indexOf(':') + 1).trimmed().toInt();
        }
    }
    if (buffer.size() < end + 4 + length)
    {
        return;
    }
    m_requests.append(QJsonDocument::fromJson(buffer.mid(end + 4, length)).object());
    const QJsonObject response = m_responses.isEmpty() ? createTextResponse(QStringLiteral("Done"))
        : m_repeat ? m_responses.first() : m_responses.takeFirst();
    const QByteArray body = QJsonDocument(response).toJson(QJsonDocument::Compact);
    socket->write(QByteArray("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: ")
                  + QByteArray::number(body.size()) + "\r\nConnection: close\r\n\r\n" + body);
    socket->disconnectFromHost();
}

void AssistantWidgetTest::onSocketDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket != nullptr)
    {
        m_buffers.remove(socket);
        socket->deleteLater();
    }
}

void AssistantWidgetTest::onProjectSwitchRejectsOldResponse()
{
    IDOSProject original;
    IDOSProject replacement;
    IDOSCommandRegistry registry;
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    original.commandManager()->setRegistry(&registry);
    widget.setProject(&original);
    registerCommands(widget.service()->commandRegistry());
    const QJsonObject response = createToolCallResponse(
        QStringLiteral("well.create"), QStringLiteral("{\"name\":\"A10\"}"));
    replacement.commandManager()->setRegistry(&registry);
    widget.setProject(&replacement);
    widget.service()->process(response, &original);
    QCOMPARE(original.objects().size(), 0);
    QCOMPARE(replacement.objects().size(), 0);
    QVERIFY(!original.commandManager()->canUndo());
    QVERIFY(!replacement.commandManager()->canUndo());
    widget.service()->process(response, &replacement);
    QTRY_COMPARE(replacement.objects().size(), 1);
    QVERIFY(replacement.commandManager()->canUndo());
}

void AssistantWidgetTest::onToolCallCreatesWell()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    project.commandManager()->setRegistry(&registry);
    widget.setProject(&project);
    registerCommands(widget.service()->commandRegistry());

    const QJsonObject response =
        createToolCallResponse(QStringLiteral("well.create"),
                               QStringLiteral("{\"name\":\"A10\"}"));

    widget.service()->process(response, &project);

    QTRY_COMPARE(project.objects().size(), 1);
    IDOSWellObject* well = nullptr;
    const QList<IDOSDataObject*> objects = project.objects();
    for (QList<IDOSDataObject*>::const_iterator iterator = objects.constBegin();
         iterator != objects.constEnd();
         ++iterator)
    {
        IDOSWellObject* candidate = qobject_cast<IDOSWellObject*>(*iterator);
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

void AssistantWidgetTest::onTextDoesNotExecute()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registerCommands(&registry);
    project.commandManager()->setRegistry(&registry);
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    widget.setProject(&project);
    widget.service()->process(createTextResponse(QStringLiteral("well.create({\"name\":\"A10\"})")), &project);
    QVERIFY(project.objects().isEmpty());
    QVERIFY(!project.commandManager()->canUndo());
    QVERIFY(!widget.service()->isBusy());
}

void AssistantWidgetTest::onInvalidToolArgumentsDoNotCreateCommand()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    project.commandManager()->setRegistry(&registry);
    widget.setProject(&project);
    registerCommands(widget.service()->commandRegistry());

    const QJsonObject response =
        createToolCallResponse(QStringLiteral("well.create"),
                               QStringLiteral("{\"name\""));

    widget.service()->process(response, &project);

    QTRY_VERIFY(!widget.service()->isBusy());
    QCOMPARE(project.objects().size(), 0);
    QVERIFY(!project.commandManager()->canUndo());
    QVERIFY(transcriptText(&widget).contains(
        QStringLiteral("The assistant provided invalid command arguments.")));
}

void AssistantWidgetTest::onToolCallWithoutProjectIsReported()
{
    IDOSCommandRegistry registry;
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    registerCommands(widget.service()->commandRegistry());

    const QJsonObject response =
        createToolCallResponse(QStringLiteral("well.create"),
                               QStringLiteral("{\"name\":\"A10\"}"));

    widget.service()->process(response, nullptr);
    QTRY_VERIFY(!widget.service()->isBusy());

    QVERIFY(transcriptText(&widget).contains(
        QStringLiteral("No project is available for command execution.")));
}

void AssistantWidgetTest::registerCommands(IDOSCommandRegistry* registry) const
{
    QVERIFY(registry != nullptr);

    registry->registerMetadata(new IDOSListObjectsCommandMetadata());
    registry->registerMetadata(new IDOSInspectObjectCommandMetadata());
    registry->registerMetadata(new IDOSCreateCaseCommandMetadata());
    registry->registerMetadata(new IDOSCreateWellCommandMetadata());
    registry->registerMetadata(new IDOSDeleteCaseCommandMetadata());
    registry->registerMetadata(new IDOSDeleteGridCommandMetadata());
    registry->registerMetadata(new IDOSDeletePropertyCommandMetadata());
    registry->registerMetadata(new IDOSDeleteWellCommandMetadata());
    registry->registerMetadata(new IDOSImportCaseCommandMetadata());
    registry->registerMetadata(new IDOSImportGridCommandMetadata());
    registry->registerMetadata(new IDOSImportGridPropertyCommandMetadata());
    registry->registerMetadata(new IDOSImportWellDataCommandMetadata());
}

QJsonObject AssistantWidgetTest::createToolCallResponse(const QString& commandName,
                                                        const QString& argumentsText) const
{
    QJsonObject functionObject;
    functionObject.insert(QStringLiteral("name"), commandName);
    functionObject.insert(QStringLiteral("arguments"), argumentsText);

    QJsonObject toolCall;
    toolCall.insert(QStringLiteral("id"), QStringLiteral("call_1"));
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
    if (widget == nullptr)
    {
        return QString();
    }

    QTextBrowser* transcript = widget->findChild<QTextBrowser*>(QStringLiteral("assistantTranscript"));
    if (transcript == nullptr)
    {
        return QString();
    }
    return transcript->toPlainText();
}

void AssistantWidgetTest::onQueryExecutionLoop()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registerCommands(&registry);
    project.commandManager()->setRegistry(&registry);
    IDOSWellObject* well = new IDOSWellObject();
    well->setName(QStringLiteral("Existing"));
    QVERIFY(project.add(well));
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    widget.setProject(&project);
    m_responses.append(createToolCallResponse(QStringLiteral("well.create"), QStringLiteral("{\"name\":\"A10\"}")));
    m_responses.append(createTextResponse(QStringLiteral("Created A10")));
    widget.service()->process(createToolCallResponse(QStringLiteral("project.objects"), QStringLiteral("{}")), &project);
    QVERIFY(widget.service()->isBusy());
    QTRY_VERIFY(!widget.service()->isBusy());
    QCOMPARE(project.objects().size(), 2);
    QCOMPARE(m_requests.size(), 2);
    const QJsonArray messages = m_requests.last().value(QStringLiteral("messages")).toArray();
    QCOMPARE(messages.size(), 5);
    QCOMPARE(messages.at(1).toObject().value(QStringLiteral("role")).toString(), QStringLiteral("assistant"));
    const QJsonObject queryResult = messages.at(2).toObject();
    QCOMPARE(queryResult.value(QStringLiteral("role")).toString(), QStringLiteral("tool"));
    QCOMPARE(queryResult.value(QStringLiteral("tool_call_id")).toString(), QStringLiteral("call_1"));
    const QJsonObject payload = QJsonDocument::fromJson(queryResult.value(QStringLiteral("content")).toString().toUtf8()).object();
    QVERIFY(payload.value(QStringLiteral("success")).toBool());
    QCOMPARE(payload.value(QStringLiteral("result")).toObject().value(QStringLiteral("objects")).toArray()
             .first().toObject().value(QStringLiteral("objectId")).toString(), well->objectId());
    QVERIFY(transcriptText(&widget).contains(QStringLiteral("Created A10")));
    project.commandManager()->undo();
    QCOMPARE(project.objects().size(), 1);
    QVERIFY(!project.commandManager()->canUndo());
    QVERIFY(project.commandManager()->canRedo());
}

void AssistantWidgetTest::onQueryPagination()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registerCommands(&registry);
    project.commandManager()->setRegistry(&registry);
    IDOSWellObject* first = new IDOSWellObject();
    IDOSWellObject* second = new IDOSWellObject();
    first->setName(QStringLiteral("Same"));
    second->setName(QStringLiteral("Same"));
    QVERIFY(project.add(first));
    QVERIFY(project.add(second));
    IDOSSimulationCaseObject* caseObject = new IDOSSimulationCaseObject();
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("well"), first->objectId()));
    caseObject->addItemRef(IDOSCaseItemRef(QStringLiteral("well"), second->objectId()));
    QVERIFY(project.add(caseObject));
    const bool modified = project.isModified();
    const QJsonObject args{{QStringLiteral("name"), QStringLiteral("same")}, {QStringLiteral("limit"), 1}};
    const QJsonObject page = project.commandManager()->execute(QStringLiteral("project.objects"), args)
        .value(QStringLiteral("result")).toObject();
    QCOMPARE(page.value(QStringLiteral("total")).toInt(), 2);
    QCOMPARE(page.value(QStringLiteral("objects")).toArray().size(), 1);
    QCOMPARE(page.value(QStringLiteral("nextOffset")).toInt(), 1);
    QJsonObject next = args;
    next.insert(QStringLiteral("offset"), 1);
    const QJsonObject nextPage = project.commandManager()->execute(QStringLiteral("project.objects"), next)
        .value(QStringLiteral("result")).toObject();
    QVERIFY(page.value(QStringLiteral("objects")) != nextPage.value(QStringLiteral("objects")));
    const QJsonObject details = project.commandManager()->execute(QStringLiteral("project.inspect"),
        QJsonObject{{QStringLiteral("objectId"), caseObject->objectId()}, {QStringLiteral("limit"), 1}})
        .value(QStringLiteral("result")).toObject();
    QCOMPARE(details.value(QStringLiteral("references")).toArray().size(), 1);
    QCOMPARE(details.value(QStringLiteral("total")).toInt(), 2);
    QCOMPARE(details.value(QStringLiteral("nextOffset")).toInt(), 1);
    QVERIFY(!project.commandManager()->canUndo());
    QCOMPARE(project.isModified(), modified);
    QVERIFY(!project.commandManager()->execute(QStringLiteral("project.inspect"),
        QJsonObject{{QStringLiteral("objectId"), QStringLiteral("missing")}}).value(QStringLiteral("success")).toBool());
    QVERIFY(!project.commandManager()->execute(QStringLiteral("project.objects"),
        QJsonObject{{QStringLiteral("offset"), -1}}).value(QStringLiteral("success")).toBool());
}

void AssistantWidgetTest::onToolResult(const QString& role, const QString& text)
{
    Q_UNUSED(text)
    if (role == QStringLiteral("Tool") && m_interruptService != nullptr)
    {
        m_interruptService->cancel();
    }
}

void AssistantWidgetTest::onCommandExecuted(const QString& name)
{
    Q_UNUSED(name)
    if (m_interruptService != nullptr)
    {
        m_interruptService->cancel();
    }
}

void AssistantWidgetTest::onCancelBetweenCommands()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registerCommands(&registry);
    project.commandManager()->setRegistry(&registry);
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    widget.setProject(&project);
    m_interruptService = widget.service();
    connect(widget.service(), &IDOSAssistantService::message, this, &AssistantWidgetTest::onToolResult);
    QJsonObject response = createToolCallResponse(QStringLiteral("well.create"), QStringLiteral("{\"name\":\"A10\"}"));
    QJsonObject message = response.value(QStringLiteral("choices")).toArray().first().toObject().value(QStringLiteral("message")).toObject();
    QJsonArray calls = message.value(QStringLiteral("tool_calls")).toArray();
    QJsonObject second = calls.first().toObject();
    second.insert(QStringLiteral("id"), QStringLiteral("call_2"));
    calls.append(second);
    message.insert(QStringLiteral("tool_calls"), calls);
    response.insert(QStringLiteral("choices"), QJsonArray{QJsonObject{{QStringLiteral("message"), message}}});
    widget.service()->process(response, &project);
    QTRY_VERIFY(!widget.service()->isBusy());
    QCOMPARE(project.objects().size(), 1);
    QVERIFY(m_requests.isEmpty());
    QVERIFY(project.commandManager()->canUndo());
    disconnect(widget.service(), nullptr, this, nullptr);
    widget.service()->process(createToolCallResponse(QStringLiteral("project.objects"), QStringLiteral("{}")), &project);
    QTRY_VERIFY(!widget.service()->isBusy());
    QCOMPARE(m_requests.size(), 1);
    const QJsonArray messages = m_requests.first().value(QStringLiteral("messages")).toArray();
    QCOMPARE(messages.at(3).toObject().value(QStringLiteral("tool_call_id")).toString(), QStringLiteral("call_2"));
    QVERIFY(messages.at(3).toObject().value(QStringLiteral("content")).toString().contains(QStringLiteral("not_executed")));
}

void AssistantWidgetTest::onCancelDuringCommand()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registerCommands(&registry);
    project.commandManager()->setRegistry(&registry);
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    widget.setProject(&project);
    m_interruptService = widget.service();
    connect(project.commandManager(), &IDOSCommandManager::commandExecuted, this, &AssistantWidgetTest::onCommandExecuted);
    widget.service()->process(createToolCallResponse(QStringLiteral("well.create"), QStringLiteral("{\"name\":\"A10\"}")), &project);
    QTRY_VERIFY(!widget.service()->isBusy());
    QCOMPARE(project.objects().size(), 1);
    QVERIFY(m_requests.isEmpty());
    QVERIFY(transcriptText(&widget).contains(QStringLiteral("\"success\":true")));
}

void AssistantWidgetTest::onCommandLimit()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registerCommands(&registry);
    project.commandManager()->setRegistry(&registry);
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    widget.setProject(&project);
    const QJsonObject query = createToolCallResponse(QStringLiteral("project.objects"), QStringLiteral("{}"));
    m_repeat = true;
    m_responses.append(query);
    widget.service()->process(query, &project);
    QTRY_VERIFY(!widget.service()->isBusy());
    QCOMPARE(m_requests.size(), 16);
    QVERIFY(!project.commandManager()->canUndo());
    QVERIFY(transcriptText(&widget).contains(QStringLiteral("command limit")));
}

void AssistantWidgetTest::onInvalidCallIds()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registerCommands(&registry);
    project.commandManager()->setRegistry(&registry);
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    widget.setProject(&project);
    QJsonObject response = createToolCallResponse(QStringLiteral("well.create"), QStringLiteral("{\"name\":\"A10\"}"));
    QJsonObject message = response.value(QStringLiteral("choices")).toArray().first().toObject().value(QStringLiteral("message")).toObject();
    QJsonArray calls = message.value(QStringLiteral("tool_calls")).toArray();
    calls.append(calls.first());
    message.insert(QStringLiteral("tool_calls"), calls);
    response.insert(QStringLiteral("choices"), QJsonArray{QJsonObject{{QStringLiteral("message"), message}}});
    widget.service()->process(response, &project);
    QVERIFY(!widget.service()->isBusy());
    QVERIFY(project.objects().isEmpty());
    QVERIFY(m_requests.isEmpty());
}

void AssistantWidgetTest::onFailureRecovery()
{
    IDOSProject project;
    IDOSCommandRegistry registry;
    registerCommands(&registry);
    project.commandManager()->setRegistry(&registry);
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    widget.setProject(&project);
    m_responses.append(createToolCallResponse(QStringLiteral("well.create"), QStringLiteral("{\"name\":\"A10\"}")));
    widget.service()->process(createToolCallResponse(QStringLiteral("well.create"), QStringLiteral("{\"name\"")), &project);
    QTRY_VERIFY(!widget.service()->isBusy());
    QCOMPARE(project.objects().size(), 1);
    QCOMPARE(m_requests.size(), 2);
    const QJsonArray messages = m_requests.last().value(QStringLiteral("messages")).toArray();
    const QJsonObject failed = QJsonDocument::fromJson(messages.at(2).toObject()
        .value(QStringLiteral("content")).toString().toUtf8()).object();
    QVERIFY(!failed.value(QStringLiteral("success")).toBool());
    QCOMPARE(failed.value(QStringLiteral("errorCode")).toString(), QStringLiteral("invalid_arguments"));
    const QJsonObject succeeded = QJsonDocument::fromJson(messages.at(4).toObject()
        .value(QStringLiteral("content")).toString().toUtf8()).object();
    QVERIFY(succeeded.value(QStringLiteral("success")).toBool());
    project.commandManager()->undo();
    QVERIFY(project.objects().isEmpty());
    QVERIFY(!project.commandManager()->canUndo());
}

void AssistantWidgetTest::onHistoryAndProjectSwitch()
{
    IDOSProject original;
    IDOSProject replacement;
    IDOSCommandRegistry registry;
    registerCommands(&registry);
    original.commandManager()->setRegistry(&registry);
    replacement.commandManager()->setRegistry(&registry);
    IDOSAssistantWidget widget(&registry);
    configureModel(&widget);
    widget.setProject(&original);
    widget.service()->process(createTextResponse(QStringLiteral("Original project marker")), &original);
    widget.service()->process(createToolCallResponse(QStringLiteral("project.objects"), QStringLiteral("{}")), &original);
    QTRY_VERIFY(!widget.service()->isBusy());
    QVERIFY(QJsonDocument(m_requests.last()).toJson().contains("Original project marker"));
    widget.setProject(&replacement);
    QVERIFY(transcriptText(&widget).isEmpty());
    widget.service()->process(createToolCallResponse(QStringLiteral("project.objects"), QStringLiteral("{}")), &replacement);
    QTRY_VERIFY(!widget.service()->isBusy());
    QVERIFY(!QJsonDocument(m_requests.last()).toJson().contains("Original project marker"));
    QVERIFY(!original.commandManager()->canUndo());
    QVERIFY(!replacement.commandManager()->canUndo());
}

QTEST_MAIN(AssistantWidgetTest)
