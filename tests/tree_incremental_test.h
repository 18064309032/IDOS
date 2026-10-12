#ifndef TREE_INCREMENTAL_TEST_H
#define TREE_INCREMENTAL_TEST_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QObject>

#include "idostool.h"
#include "idostoolmetadata.h"
#include "idostoolprovider.h"

class IDOSToolRegistry;
class QMenu;

class IDOSTestInputTool : public IDOSTool
{
    Q_OBJECT
public:
    explicit IDOSTestInputTool(IDOSRenderView* view);
    void observe(vtkObject* source);
    QString exclusiveGroup() const override;
    bool consumed;
    int inputs;
    int observations;
    int observedValue;
    QString group;
    IDOSToolRegistry* registry;
    bool removalAccepted;
    bool stopOnInput;
protected:
    bool onEventFilter(QObject* watched, QEvent* event) override;
    void onVtkEvent(vtkObject* source, unsigned long eventId, void* callData) override;
private slots:
    void onCallbackFinished(bool consumed);
};

class IDOSTestToolProvider : public IDOSToolProvider
{
    Q_OBJECT
public:
    explicit IDOSTestToolProvider(IDOSRenderView* view);
    ~IDOSTestToolProvider() override;
    QList<QAction*> actions() const override;
    QList<IDOSTool*> tools() const override;
    void updateState() override;
private slots:
    void onToggleTool();
private:
    QAction* m_action;
    QMenu* m_menu;
    IDOSTestInputTool* m_tool;
};

class IDOSTestToolMetadata : public IDOSToolMetadata
{
public:
    explicit IDOSTestToolMetadata(const QString& id = QStringLiteral("test.input"));
    QString id() const override;
    QString name() const override;
    QString description() const override;
    QString iconPath() const override;
    QString groupId() const override;
    int order() const override;
    bool supports(const IDOSRenderView* view) const override;
    IDOSToolProvider* create(IDOSRenderView* view) const override;
private:
    QString m_id;
};

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
    void onRegistryLifecycle();
    void onTreeOwnership();
    void onMenuTargets();
    void onToolRegistryAndWindows();
    void onToolInputLifecycle();
    void onToolObserversAndUnregister();

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
