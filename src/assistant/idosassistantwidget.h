#ifndef IDOS_ASSISTANT_WIDGET_H
#define IDOS_ASSISTANT_WIDGET_H

#include <QWidget>

#include "idos_assistant.h"

class IDOSAssistantService;
class IDOSProject;
class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;

class ASSISTANT_EXPORT IDOSAssistantWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSAssistantWidget(QWidget* parent = nullptr);
    ~IDOSAssistantWidget() override;
    void setProject(IDOSProject* project);
    IDOSAssistantService* service() const;

  signals:
    void statusChanged(const QString& status);

  private slots:
    void onStartClicked();
    void onStopClicked();
    void onSendClicked();
    void onAssistantReady();
    void onAssistantStopped();
    void onAssistantError(const QString& error);
    void onRequestFailed(const QString& error);
    void onMessage(const QString& role, const QString& message);
    void onControlsChanged();

  private:
    IDOSAssistantService* m_service;
    QTextEdit* m_transcript;
    QLineEdit* m_input;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_sendButton;
    QLabel* m_statusLabel;
};

#endif
