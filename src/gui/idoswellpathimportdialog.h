#ifndef IDOS_WELL_PATH_IMPORT_DIALOG_H
#define IDOS_WELL_PATH_IMPORT_DIALOG_H
#include "idos_gui.h"
#include <QDialog>
#include <QStringList>
class IDOSWell;
class IDOSProject;
class GUI_EXPORT IDOSWellPathImportDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit IDOSWellPathImportDialog(const QStringList& files, const QList<IDOSWell*>& wells,
                                      const QStringList& errors, const IDOSProject* project, QWidget* parent = nullptr);
    QStringList targetIds() const;
  private slots:
    void onAccepted();
    void onRejected();

  private:
    QStringList m_targetIds;
};
#endif
