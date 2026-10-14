#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QStringList>
#include <QVariant>

#include "command/idosimportcommands.h"
#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/fluidrock/idosfluidrocktable.h"
#include "idoscommandmanager.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "tree/idostree.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeview.h"

#include "idosfluidrocktabletreeguiprovider.h"

QString IDOSFluidRockTableTreeGuiProvider::name() const
{
    return QStringLiteral("idos.fluidRockTable");
}

bool IDOSFluidRockTableTreeGuiProvider::supports(const IDOSTreeItem* item) const
{
    if (item == nullptr || item->tree() == nullptr || item->tree()->name() != QStringLiteral("data"))
    {
        return false;
    }
    if (item->key() == QStringLiteral("data.fluids") ||
        item->key() == QStringLiteral("data.rockPhysics"))
    {
        return true;
    }
    IDOSProject* project = item->tree()->project();
    return project != nullptr &&
           qobject_cast<IDOSFluidRockTable*>(project->object(item->dataId())) != nullptr;
}

QAction* IDOSFluidRockTableTreeGuiProvider::addAction(QMenu* menu, IDOSTreeView* view, const QString& text)
{
    QAction* action = menu->addAction(text);
    IDOSTreeItem* item = view->menuItem();
    action->setProperty("dataId", item != nullptr ? item->dataId() : QString());
    action->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    return action;
}

IDOSTreeView* IDOSFluidRockTableTreeGuiProvider::actionView() const
{
    QAction* action = qobject_cast<QAction*>(sender());
    QMenu* menu = action != nullptr ? qobject_cast<QMenu*>(action->parent()) : nullptr;
    IDOSTreeView* view = menu != nullptr ? qobject_cast<IDOSTreeView*>(menu->parentWidget()) : nullptr;
    if (view == nullptr || view->project() == nullptr ||
        action->property("project").value<QObject*>() != view->project())
    {
        return nullptr;
    }
    return view;
}

void IDOSFluidRockTableTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* view)
{
    if (view->project() == nullptr || view->tree() == nullptr ||
        view->tree()->name() != QStringLiteral("data"))
    {
        return;
    }
    IDOSTreeItem* item = view->menuItem();
    if (item == nullptr)
    {
        return;
    }
    if (item->key() == QStringLiteral("data.fluids") ||
        item->key() == QStringLiteral("data.rockPhysics"))
    {
        const bool isFluidCategory = item->key() == QStringLiteral("data.fluids");
        const QString actionText = isFluidCategory
            ? tr("Import Fluid Tables...") : tr("Import Rock Physics Functions...");
        QAction* import = addAction(menu, view, actionText);
        import->setObjectName(isFluidCategory
                                  ? QStringLiteral("importFluidTablesAction")
                                  : QStringLiteral("importRockPhysicsTablesAction"));
        import->setProperty("category", isFluidCategory
                                             ? QStringLiteral("fluid")
                                             : QStringLiteral("rockPhysics"));
        connect(import, &QAction::triggered, this,
                &IDOSFluidRockTableTreeGuiProvider::onImportTables);
        return;
    }
    IDOSFluidRockTable* table =
        qobject_cast<IDOSFluidRockTable*>(view->project()->object(item->dataId()));
    if (table != nullptr)
    {
        QAction* add = addAction(menu, view, tr("Add to Simulation Case..."));
        add->setObjectName(QStringLiteral("addFluidRockTableToCaseAction"));
        connect(add, &QAction::triggered, this,
                &IDOSFluidRockTableTreeGuiProvider::onAddTableToCase);
    }
}

void IDOSFluidRockTableTreeGuiProvider::onImportTables()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(view->project());
    const QString category = action->property("category").toString();
    const bool isFluidCategory = category == QStringLiteral("fluid");
    const QString title = isFluidCategory
        ? tr("Import Fluid Tables") : tr("Import Rock Physics Functions");
    const QString fileFilter = isFluidCategory
        ? tr("ECLIPSE Fluid Tables (*.TXT *.txt *.DAT *.dat *.PRN *.prn);;All Files (*)")
        : tr("ECLIPSE Rock Physics Tables (*.TXT *.txt *.DAT *.dat *.PRN *.prn);;All Files (*)");
    const QString file = QFileDialog::getOpenFileName(view, title, QString(), fileFilter);
    if (file.isEmpty() || project.isNull() || project->commandManager() == nullptr)
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("category"), category);
    const QJsonObject response = project->commandManager()->execute(
        QStringLiteral("data.fluidRock.import"), arguments);
    if (!response.value(QStringLiteral("success")).toBool())
    {
        QMessageBox::warning(view, title,
                             response.value(QStringLiteral("error")).toString());
    }
}

void IDOSFluidRockTableTreeGuiProvider::onAddTableToCase()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(view->project());
    const QString tableId = action->property("dataId").toString();
    if (project.isNull() || qobject_cast<IDOSFluidRockTable*>(project->object(tableId)) == nullptr)
    {
        return;
    }
    QStringList caseNames;
    QStringList caseIds;
    for (IDOSDataObject* object : project->objects())
    {
        IDOSSimulationCaseObject* simulationCase = qobject_cast<IDOSSimulationCaseObject*>(object);
        if (simulationCase != nullptr)
        {
            caseNames.append(simulationCase->name());
            caseIds.append(simulationCase->objectId());
        }
    }
    if (caseNames.isEmpty())
    {
        QMessageBox::information(view, tr("Add Table to Simulation Case"),
                                 tr("Create or import a simulation case first."));
        return;
    }
    bool accepted = false;
    const QString selectedName = QInputDialog::getItem(
        view, tr("Add Table to Simulation Case"), tr("Simulation case:"),
        caseNames, 0, false, &accepted);
    if (!accepted || project.isNull())
    {
        return;
    }
    const int caseIndex = caseNames.indexOf(selectedName);
    if (caseIndex < 0 || project->commandManager() == nullptr)
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("targetCaseId"), caseIds.at(caseIndex));
    arguments.insert(QStringLiteral("tableId"), tableId);
    const QJsonObject response = project->commandManager()->execute(
        QStringLiteral("case.fluidRock.attach"), arguments);
    if (!response.value(QStringLiteral("success")).toBool())
    {
        QMessageBox::warning(view, tr("Add Table to Simulation Case"),
                             response.value(QStringLiteral("error")).toString());
    }
}
