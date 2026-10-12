#ifndef IDOS_PYTHON_CONSOLE_WIDGET_H
#define IDOS_PYTHON_CONSOLE_WIDGET_H

#include <QWidget>

#include "idos_gui.h"

class QPlainTextEdit;

class GUI_EXPORT IDOSPythonConsoleWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSPythonConsoleWidget(QWidget* parent = nullptr);
    ~IDOSPythonConsoleWidget() override;

  private slots:
    void onClearOutputClicked();

  private:
    QPlainTextEdit* m_output;
    QPlainTextEdit* m_input;
};

#endif // IDOS_PYTHON_CONSOLE_WIDGET_H
