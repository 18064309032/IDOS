#ifndef IDOS_PROJECT_START_WIDGET_H
#define IDOS_PROJECT_START_WIDGET_H

#include <QStringList>
#include <QWidget>

#include "idos_gui.h"

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPoint;

class GUI_EXPORT IDOSProjectStartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IDOSProjectStartWidget(QWidget* parent = nullptr);
    void setRecentProjects(const QStringList& projectPaths);

signals:
    void newProjectRequested();
    void openProjectRequested();
    void recentProjectOpenRequested(const QString& path);
    void recentProjectRemoveRequested(const QString& path);

private slots:
    void onNewProjectClicked();
    void onOpenProjectClicked();
    void onRecentProjectItemActivated(QListWidgetItem* item);
    void onRecentProjectContextMenuRequested(const QPoint& position);

private:
    QListWidget* m_recentProjectsList;
    QLabel* m_emptyRecentProjectsLabel;
};

#endif // IDOS_PROJECT_START_WIDGET_H
