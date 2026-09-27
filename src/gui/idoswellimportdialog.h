#ifndef IDOS_WELL_IMPORT_DIALOG_H
#define IDOS_WELL_IMPORT_DIALOG_H
#include "idos_gui.h"
#include <QDialog>
#include <QList>
#include <QStringList>
class IDOSDataObject;
class GUI_EXPORT IDOSWellImportDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit IDOSWellImportDialog(const QString& filePath, const QList<IDOSDataObject*>& objects,
                                  const QStringList& existingNames, QWidget* parent = nullptr);
    QList<int> selectedRows() const;
  private slots:
    void onAccepted();
    void onRejected();

  private:
    QList<int> m_selectedRows;
};
#endif
