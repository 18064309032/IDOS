#ifndef IDOSFRACTUREGEOMETRYREPORTDIALOG_H
#define IDOSFRACTUREGEOMETRYREPORTDIALOG_H

#include <QDialog>
#include <QList>
#include <QPointer>
#include <QString>
#include <QVariant>

class IDOSNaturalFractureSet;
class IDOSProject;
class IDOSRenderView;
class QTableWidget;

class IDOSFractureGeometryReportDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit IDOSFractureGeometryReportDialog(const QList<IDOSNaturalFractureSet*>& sets,
        IDOSProject* project, IDOSRenderView* targetView, QWidget* parent = nullptr);
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

#endif // IDOSFRACTUREGEOMETRYREPORTDIALOG_H
