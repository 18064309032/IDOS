#ifndef IDOS_WAITING_DIALOG_H
#define IDOS_WAITING_DIALOG_H

#include <QDialog>
#include <QElapsedTimer>
#include <QString>

#include "idos_gui.h"

class QCloseEvent;
class QFrame;
class QLabel;
class QProgressBar;
class QResizeEvent;
class QTimer;

class GUI_EXPORT IDOSWaitingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IDOSWaitingDialog(QWidget* parent = nullptr);
    void begin(const QString& title, const QString& message, const QString& detail = QString());
    void setMessage(const QString& message);
    void setDetail(const QString& detail);
    void setProgress(int value, int maximum = 100);
    void setBusy();
    void finish();
    bool isRunning() const;
    void done(int result) override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onElapsedTimeout();

private:
    void updateDetail();
    QFrame* m_card;
    QLabel* m_title;
    QLabel* m_message;
    QLabel* m_detail;
    QLabel* m_elapsed;
    QProgressBar* m_progress;
    QTimer* m_timer;
    QElapsedTimer m_clock;
    QString m_detailText;
    bool m_running;
};

#endif
