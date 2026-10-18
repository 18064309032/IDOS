#ifndef ASSISTANT_WIDGET_TEST_H
#define ASSISTANT_WIDGET_TEST_H

#include <QByteArray>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>

class IDOSAssistantService;
class IDOSAssistantWidget;
class IDOSCommandRegistry;
class IDOSProject;
class QTcpServer;
class QTcpSocket;

class AssistantWidgetTest : public QObject
{
    Q_OBJECT
public:
    AssistantWidgetTest();

private slots:
    void onToolCallCreatesWell();
    void onTextDoesNotExecute();
    void onInvalidToolArgumentsDoNotCreateCommand();
    void onToolCallWithoutProjectIsReported();
    void onProjectSwitchRejectsOldResponse();
    void onQueryExecutionLoop();
    void onQueryPagination();
    void onCancelBetweenCommands();
    void onCancelDuringCommand();
    void onCommandLimit();
    void onInvalidCallIds();
    void onFailureRecovery();
    void onHistoryAndProjectSwitch();

private:
    void onNewConnection();
    void onSocketReadyRead();
    void onSocketDisconnected();
    void onToolResult(const QString& role, const QString& text);
    void onCommandExecuted(const QString& name);
    void configureModel(IDOSAssistantWidget* widget);
    void registerCommands(IDOSCommandRegistry* registry) const;
    QJsonObject createToolCallResponse(const QString& commandName, const QString& argumentsText) const;
    QJsonObject createTextResponse(const QString& text) const;
    QString transcriptText(IDOSAssistantWidget* widget) const;
    QTcpServer* m_server;
    QMap<QTcpSocket*, QByteArray> m_buffers;
    QList<QJsonObject> m_responses;
    QList<QJsonObject> m_requests;
    bool m_repeat;
    QPointer<IDOSAssistantService> m_interruptService;
};

#endif
