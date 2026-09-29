#ifndef IDOS_ASSISTANT_WIDGET_H
#define IDOS_ASSISTANT_WIDGET_H

#include <QJsonObject>
#include <QPointer>
#include <QWidget>

#include "idos_gui.h"

class IDOSLocalAssistantService;
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
    QString responseText(const QJsonObject& response) const;
    void updateControls();

    IDOSLocalAssistantService* m_service;
    QTextEdit* m_transcript;
    QLineEdit* m_input;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_sendButton;
    QLabel* m_statusLabel;
    QPointer<QNetworkReply> m_activeReply;
};

#endif // IDOS_ASSISTANT_WIDGET_H
