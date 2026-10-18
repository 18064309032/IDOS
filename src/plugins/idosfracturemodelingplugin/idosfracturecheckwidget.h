#ifndef IDOS_FRACTURE_CHECK_WIDGET_H
#define IDOS_FRACTURE_CHECK_WIDGET_H

#include <QList>
#include <QPointer>
#include <QString>
#include <QVariant>
#include <QWidget>

class IDOSInterface;
class IDOSProject;
class IDOSRenderView;
class QTableWidget;

class IDOSFractureCheckWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSFractureCheckWidget(IDOSInterface* interface);
    IDOSProject* project() const;
    IDOSRenderView* targetView() const;

  signals:
    void issueActivated(const QString& objectId, const QVariantList& center);

  private slots:
    void onIssueActivated(int row, int column);

  private:
    QPointer<IDOSProject> m_project;
    QPointer<IDOSRenderView> m_targetView;
    QTableWidget* m_table;
};

#endif // IDOS_FRACTURE_CHECK_WIDGET_H
