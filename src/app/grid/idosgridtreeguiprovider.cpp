#include <QAction>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QVariant>

#include "command/idoscommandmanager.h"
#include "command/idosimportcommands.h"
#include "idoscasedialog.h"
#include "idoscaseobject.h"
#include "idoscreatecasecommand.h"
#include "idoscreatewellcommand.h"
#include "idosdataobject.h"
#include "idosdataprovider.h"
#include "idosdataproviderregistry.h"
#include "idosdeletecasecommand.h"
#include "idosdeletegridcommand.h"
#include "idosdeletepropertycommand.h"
#include "idosdeletewellcommand.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idosproject.h"
#include "idossimulationcaseobject.h"
#include "idostree.h"
#include "idostreeitem.h"
#include "idostreeview.h"
#include "idoswell.h"
#include "log/idoslogger.h"

#include "idosgridtreeguiprovider.h"

QString IDOSGridTreeGuiProvider::name() const
{
    return QStringLiteral("idos.grid");
}

QAction* IDOSGridTreeGuiProvider::addAction(QMenu* menu, IDOSTreeView* view, const QString& text)
{
    QAction* action = menu->addAction(text);
    IDOSTreeItem* item = view->menuItem();
    action->setProperty("dataId", item != nullptr ? item->dataId() : QString());
    action->setProperty("caseId", item != nullptr ? item->caseId() : QString());
    action->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    return action;
}

IDOSTreeView* IDOSGridTreeGuiProvider::actionView() const
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

bool IDOSGridTreeGuiProvider::supports(const IDOSTreeItem* item) const
{
    if (item == nullptr || item->tree() == nullptr || item->tree()->project() == nullptr)
    {
        return false;
    }
    IDOSDataObject* data = item->tree()->project()->object(item->dataId());
    return qobject_cast<IDOSGrid*>(data) != nullptr || qobject_cast<IDOSGridProperty*>(data) != nullptr;
}

void IDOSGridTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* view)
{
    if (view->project() == nullptr)
    {
        return;
    }
    IDOSTreeItem* item = view->menuItem();

    if (item == nullptr)
    {
        return;
    }
    IDOSDataObject* data = view->project()->object(item->dataId());
    if (item->isGroup())
    {
        QAction* import = addAction(menu, view, tr("Import Property..."));
        import->setObjectName(QStringLiteral("importPropertyAction"));
        connect(import, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onImportProperty);
    }
    else if (qobject_cast<IDOSGridProperty*>(data) != nullptr)
    {
        QAction* remove = addAction(menu, view, tr("Delete Property..."));
        remove->setObjectName(QStringLiteral("deletePropertyAction"));
        connect(remove, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onDeleteProperty);
    }
    else if (qobject_cast<IDOSGrid*>(data) != nullptr)
    {
        QAction* remove = addAction(menu, view, tr("Delete Grid..."));
        remove->setObjectName(QStringLiteral("deleteGridAction"));
        connect(remove, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onDeleteGrid);
    }
}

void IDOSGridTreeGuiProvider::onImportProperty()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    const QString gridId = action->property("dataId").toString();
    if (qobject_cast<IDOSGrid*>(view->project()->object(gridId)) != nullptr)
    {
        IDOSGridTreeGuiProvider::importProperty(view->project(), gridId, action->property("caseId").toString(), view);
    }
}

void IDOSGridTreeGuiProvider::onDeleteProperty()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    const QString id = action->property("dataId").toString();
    if (qobject_cast<IDOSGridProperty*>(view->project()->object(id)) != nullptr)
    {
        IDOSGridTreeGuiProvider::deleteProperty(view->project(), id, view);
    }
}

void IDOSGridTreeGuiProvider::onDeleteGrid()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    const QString id = action->property("dataId").toString();
    if (qobject_cast<IDOSGrid*>(view->project()->object(id)) != nullptr)
    {
        IDOSGridTreeGuiProvider::deleteGrid(view->project(), id, view);
    }
}

bool IDOSGridTreeGuiProvider::setChecked(IDOSTreeItem* item, bool checked)
{
    if (item == nullptr || item->tree() == nullptr || item->tree()->project() == nullptr)
    {
        return false;
    }
    IDOSProject* project = item->tree()->project();
    IDOSDataObject* data = project->object(item->dataId());
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(data);
    if (data == nullptr || (property == nullptr && grid == nullptr))
    {
        return false;
    }
    project->beginUpdate();
    if (property != nullptr && checked)
    {
        for (IDOSDataObject* candidate : project->objects())
        {
            IDOSGridProperty* other = qobject_cast<IDOSGridProperty*>(candidate);
            if (other != nullptr && other != property)
            {
                other->setVisible(false);
            }
        }
        IDOSDataObject* parentGrid = project->object(property->gridId());
        if (parentGrid != nullptr)
        {
            parentGrid->setVisible(true);
        }
    }
    if (grid != nullptr && !checked)
    {
        for (IDOSDataObject* candidate : project->objects())
        {
            IDOSGridProperty* child = qobject_cast<IDOSGridProperty*>(candidate);
            if (child != nullptr && child->gridId() == grid->objectId())
            {
                child->setVisible(false);
            }
        }
    }
    data->setVisible(checked);
    project->endUpdate();
    return true;
}


QStringList IDOSGridTreeGuiProvider::collectGridPropertyIds(const IDOSProject* project, const QString& gridId)
{
    QStringList ids;
    if (project == nullptr || gridId.isEmpty())
    {
        return ids;
    }
    for (IDOSDataObject* object : project->objects())
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property != nullptr && property->gridId() == gridId)
        {
            ids.append(property->objectId());
        }
    }
    return ids;
}

bool IDOSGridTreeGuiProvider::deleteGrid(IDOSProject* project, const QString& gridId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || gridId.isEmpty())
    {
        return false;
    }
    IDOSDataObject* obj = target->object(gridId);
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(obj);
    if (grid == nullptr)
    {
        return false;
    }
    const int propertyCount = collectGridPropertyIds(target.data(), gridId).size();
    const QMessageBox::StandardButton btn =
        QMessageBox::question(parent, tr("Delete Grid"),
                               tr("Are you sure you want to delete grid \"%1\"?\n"
                                  "It will also delete %2 propert(ies) under this grid "
                                  "and remove it from its case.")
                                   .arg(grid->name())
                                   .arg(propertyCount),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeleteGridCommand(target, gridId));
}

bool IDOSGridTreeGuiProvider::deleteProperty(IDOSProject* project, const QString& propertyId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || propertyId.isEmpty())
    {
        return false;
    }
    IDOSDataObject* obj = target->object(propertyId);
    if (qobject_cast<IDOSGridProperty*>(obj) == nullptr)
    {
        return false;
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(parent, tr("Delete Property"),
                               tr("Are you sure you want to delete property \"%1\"?\n"
                                  "It will be removed from all cases that reference it.")
                                   .arg(obj->name()),
                               QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (btn != QMessageBox::Yes || target.isNull())
    {
        return false;
    }
    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    return commandManager->execute(new IDOSDeletePropertyCommand(target, propertyId));
}

bool IDOSGridTreeGuiProvider::importProperty(IDOSProject* project, const QString& targetGridId,
                                            const QString& targetCaseId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull())
    {
        return false;
    }
    // 当前能产出网格属性的可识别格式为 ECLIPSE .DATA；后续新增 .GRDECL/.INIT/.UNRST
    // provider 后在此扩充过滤项
    const QString file = QFileDialog::getOpenFileName(
        parent, tr("Import Grid Property"), QString(),
        tr("ECLIPSE Data Files (*.DATA *.data);;All Files (*)"));
    if (file.isEmpty() || target.isNull())
    {
        return false;
    }

    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("targetGridId"), targetGridId);
    arguments.insert(QStringLiteral("targetCaseId"), targetCaseId);
    IDOSImportObjectsCommand* command =
        new IDOSImportObjectsCommand(target, QStringLiteral("property.import"), arguments);
    const bool success = commandManager->execute(command);
    if (!success)
    {
        QMessageBox::warning(parent, tr("Import Grid Property"), commandManager->error());
        return false;
    }

    const int importedCount = commandManager->result().value(QStringLiteral("objectCount")).toInt();
    QMessageBox::information(parent, tr("Import Grid Property"),
                             tr("Imported %1 properties.").arg(importedCount));
    return true;
}