#ifndef IDOS_RUNTIMEINFOWIDGET_H
#define IDOS_RUNTIMEINFOWIDGET_H

#include <QWidget>

#include "idos_gui.h"
#include "log/idosloglevel.h"

class QPlainTextEdit;

class GUI_EXPORT IDOSRuntimeInfoWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSRuntimeInfoWidget(QWidget* parent = nullptr);
    ~IDOSRuntimeInfoWidget() override;

  private slots:
    void onClearClicked();
    void onMessagePosted(const QString& message, IDOSLogLevel level);
    void onRecordsCleared();

  private:
    void appendLine(IDOSLogLevel level, const QString& text);
    QColor levelColor(IDOSLogLevel level) const;

    QPlainTextEdit* m_textEdit;
};

#endif // IDOS_RUNTIMEINFOWIDGET_H
