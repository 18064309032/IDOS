#ifndef IDOS_SIMULATION_DECK_BROWSER_WIDGET_H
#define IDOS_SIMULATION_DECK_BROWSER_WIDGET_H

#include <QString>
#include <QWidget>

#include "idos_app.h"

class IDOSSimulationCaseObject;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;

class APP_EXPORT IDOSSimulationDeckBrowserWidget : public QWidget
{
    Q_OBJECT

  public:
    IDOSSimulationDeckBrowserWidget(IDOSSimulationCaseObject* simulationCase, QWidget* parent = nullptr);
    ~IDOSSimulationDeckBrowserWidget() override;

  private slots:
    void onFilterTextChanged(const QString& text);
    void onKeywordSelectionChanged();

  private:
    QLineEdit* m_filter;
    QListWidget* m_keywordList;
    QPlainTextEdit* m_keywordText;
};

#endif // IDOS_SIMULATION_DECK_BROWSER_WIDGET_H
