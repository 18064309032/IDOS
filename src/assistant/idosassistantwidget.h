#ifndef IDOS_ASSISTANT_WIDGET_H
#define IDOS_ASSISTANT_WIDGET_H

#include <QWidget>

#include "idos_assistant.h"

class IDOSAssistantService;
class IDOSProject;
class QLabel;
class QPushButton;
class QTextBrowser;
class QTextEdit;

class ASSISTANT_EXPORT IDOSAssistantWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSAssistantWidget(QWidget* parent = nullptr);
    ~IDOSAssistantWidget() override;
    void setProject(IDOSProject* project);
    void startAssistant();
    IDOSAssistantService* service() const;

  protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

  signals:
    void statusChanged(const QString& status);

  private slots:
    void onSendClicked();
    void onAssistantReady();
    void onAssistantStopped();
    void onAssistantError(const QString& error);
    void onRequestFailed(const QString& error);
    void onMessage(const QString& role, const QString& message);
    void onControlsChanged();

  private:
    IDOSAssistantService* m_service;
    QTextBrowser* m_transcript;
    QTextEdit* m_input;
    QPushButton* m_sendButton;
    QLabel* m_statusLabel;
    bool m_startupPending;
    QString m_pendingMessage;
};

#endif
