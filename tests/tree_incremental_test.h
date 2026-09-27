#ifndef TREE_INCREMENTAL_TEST_H
#define TREE_INCREMENTAL_TEST_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>

class TreeIncrementalTest : public QObject
{
    Q_OBJECT

  private slots:
    void onDataTreeIncrementalAddRenameRemove();
    void onSameIdReplacementDoesNotDuplicate();
    void onExpansionStatePreservedAcrossRebuild();
    void onBatchImportRebuildsCaseBranchOnce();
    void onProjectUpdateTransactionBatchesSignals();
    void onCaseTreeReferenceRefreshAndCheckPreserved();

  private slots:
    void onDataTreeReset();
    void onDataRowsInserted();
    void onDataRowsRemoved();
    void onCaseTreeReset();
    void onCaseRowsInserted();
    void onCaseRowsRemoved();

  private:
    void resetSignalCounters();
    static QModelIndex findIndexByName(const QAbstractItemModel* model, const QModelIndex& parent,
                                       const QString& name);
    static void collectDisplayNames(const QAbstractItemModel* model, const QModelIndex& parent,
                                    QStringList& names);

    bool m_dataTreeReset = false;
    bool m_caseTreeReset = false;
    int m_dataRowsInserted = 0;
    int m_dataRowsRemoved = 0;
    int m_caseRowsInserted = 0;
    int m_caseRowsRemoved = 0;
};

#endif // TREE_INCREMENTAL_TEST_H
