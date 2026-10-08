#ifndef ASSISTANT_WIDGET_TEST_H
#define ASSISTANT_WIDGET_TEST_H

#include <QObject>
#include <QJsonObject>
#include <QString>

class IDOSAssistantWidget;
class IDOSCommandRegistry;

class AssistantWidgetTest : public QObject
{
    Q_OBJECT

private slots:
    void onToolCallCreatesWell();
    void onTextCommandCallCreatesWell();
    void onInvalidToolArgumentsDoNotCreateCommand();
    void onToolCallWithoutProjectIsReported();

private:
    void registerCommands(IDOSCommandRegistry* registry) const;
    QJsonObject createToolCallResponse(const QString& commandName,
                                       const QString& argumentsText) const;
    QJsonObject createTextResponse(const QString& text) const;
    QString transcriptText(IDOSAssistantWidget* widget) const;
};

#endif // ASSISTANT_WIDGET_TEST_H
