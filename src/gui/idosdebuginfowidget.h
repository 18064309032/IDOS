#ifndef IDOS_DEBUGINFOWIDGET_H
#define IDOS_DEBUGINFOWIDGET_H

#include <QWidget>

#include "idos_gui.h"
#include "log/idosloglevel.h"
#include "log/idoslogrecord.h"

class QPlainTextEdit;

class GUI_EXPORT IDOSDebugInfoWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSDebugInfoWidget(QWidget* parent = nullptr);
    ~IDOSDebugInfoWidget() override;

  private slots:
    void onClearClicked();
    void onCopyClicked();
    void onRecordAppended(const IDOSLogRecord& record);
    void onRecordsCleared();

  private:
    void rebuild();
    void appendRecord(const IDOSLogRecord& record);
    QColor levelColor(IDOSLogLevel level) const;

    QPlainTextEdit* m_textEdit;
};

#endif // IDOS_DEBUGINFOWIDGET_H
