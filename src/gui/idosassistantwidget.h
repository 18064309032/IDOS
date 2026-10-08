#ifndef IDOS_ASSISTANT_WIDGET_H
#define IDOS_ASSISTANT_WIDGET_H

#include <memory>

#include <QJsonArray>
#include <QJsonObject>
#include <QPointer>
#include <QWidget>

#include "idos_gui.h"

class IDOSCommandRegistry;
class IDOSLocalAssistantService;
class IDOSProject;
class QLineEdit;
class QPushButton;
class QTextEdit;
class QNetworkReply;
class QLabel;

class GUI_EXPORT IDOSAssistantWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IDOSAssistantWidget(QWidget* parent = nullptr);
    ~IDOSAssistantWidget() override;

    IDOSAssistantWidget(const IDOSAssistantWidget&) = delete;
    IDOSAssistantWidget& operator=(const IDOSAssistantWidget&) = delete;

    void setProject(IDOSProject* project);
    IDOSCommandRegistry* commandRegistry() const;

signals:
    void statusChanged(const QString& status);

private slots:
    void onStartClicked();
    void onStopClicked();
    void onSendClicked();
    void onAssistantReady();
    void onAssistantStopped();
    void onAssistantResponse(const QJsonObject& response);
    void onAssistantError(const QString& error);
    void onRequestFailed(const QString& error);

private:
    void appendMessage(const QString& role, const QString& message);
    bool handleToolCalls(const QJsonObject& response);
    bool handleTextCommandCall(const QString& text);
    bool executeToolCall(const QJsonObject& toolCall);
    QJsonObject createToolCall(const QString& commandName,
                               const QString& argumentsText) const;
    QJsonObject parseArguments(const QString& argumentsText,
                               bool& ok) const;
    QString commandSystemMessage(const QJsonArray& tools) const;
    QString responseText(const QJsonObject& response) const;
    void updateControls();

    IDOSLocalAssistantService* m_service;
    std::unique_ptr<IDOSCommandRegistry> m_commandRegistry;
    QPointer<IDOSProject> m_project;
    QTextEdit* m_transcript;
    QLineEdit* m_input;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_sendButton;
    QLabel* m_statusLabel;
    QPointer<QNetworkReply> m_activeReply;
};

#endif // IDOS_ASSISTANT_WIDGET_H
