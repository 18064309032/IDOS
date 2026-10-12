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
#include "idosgridrenderprovider.h"
#include "idosimportrestartresultscommand.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderprovider.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
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

bool IDOSGridTreeGuiProvider::isChecked(const IDOSTreeItem* item, const IDOSTreeView* treeView) const
{
    IDOSRenderView* view = treeView != nullptr ? qobject_cast<IDOSRenderView*>(treeView->targetView()) : nullptr;
    if (item == nullptr || view == nullptr)
    {
        return false;
    }
    IDOSDataObject* data = item->tree() != nullptr && item->tree()->project() != nullptr
                               ? item->tree()->project()->object(item->dataId())
                               : nullptr;
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
    IDOSRenderObject* object = gridRenderObject(property != nullptr ? property->gridId()
                                                                    : item->dataId(), view);
    IDOSGridRenderProvider* provider = object != nullptr
                                           ? dynamic_cast<IDOSGridRenderProvider*>(object->provider())
                                           : nullptr;
    if (provider == nullptr)
    {
        return false;
    }
    return property != nullptr
               ? provider->property() == property
               : object->visible();
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
        if (item->key() == QStringLiteral("grid.dynamicProperties"))
        {
            QAction* import = addAction(menu, view, tr("Import Dynamic Properties..."));
            import->setObjectName(QStringLiteral("importDynamicPropertiesAction"));
            connect(import, &QAction::triggered, this,
                    &IDOSGridTreeGuiProvider::onImportDynamicProperties);
        }
        else
        {
            QAction* import = addAction(menu, view, tr("Import Property..."));
            import->setObjectName(QStringLiteral("importPropertyAction"));
            connect(import, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onImportProperty);
        }
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
        importProperty(view, gridId, action->property("caseId").toString());
    }
}

void IDOSGridTreeGuiProvider::onImportDynamicProperties()
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
        importDynamicProperties(view, gridId);
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
        deleteProperty(view, id);
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
        deleteGrid(view, id);
    }
}

bool IDOSGridTreeGuiProvider::setChecked(IDOSTreeItem* item,
                                         bool checked,
                                         IDOSTreeView* treeView)
{
    IDOSRenderView* view = treeView != nullptr ? qobject_cast<IDOSRenderView*>(treeView->targetView()) : nullptr;
    if (item == nullptr || item->tree() == nullptr || item->tree()->project() == nullptr || view == nullptr)
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
    IDOSRenderObject* object = gridRenderObject(property != nullptr ? property->gridId()
                                                                   : grid->objectId(), view);
    IDOSGridRenderProvider* provider = object != nullptr
                                           ? dynamic_cast<IDOSGridRenderProvider*>(object->provider())
                                           : nullptr;
    if (provider == nullptr)
    {
        return false;
    }

    if (grid != nullptr)
    {
        object->setVisible(checked);
        if (!checked)
        {
            provider->setProperty(nullptr);
        }
    }
    else if (checked)
    {
        provider->setProperty(property);
        object->setVisible(true);
    }
    else if (provider->property() == property)
    {
        provider->setProperty(nullptr);
    }
    view->scene()->notifyChanged(object->id());
    if (checked)
    {
        view->resetCamera();
    }
    emit checkStateChanged();
    return true;
}

IDOSRenderObject* IDOSGridTreeGuiProvider::gridRenderObject(const QString& gridId,
                                                            IDOSRenderView* view) const
{
    if (view == nullptr)
    {
        return nullptr;
    }
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        IDOSDataObject* source = object->provider()->dataSource();
        if (source != nullptr && source->objectId() == gridId &&
            dynamic_cast<IDOSGridRenderProvider*>(object->provider()) != nullptr)
        {
            return object;
        }
    }
    return nullptr;
}


int IDOSGridTreeGuiProvider::gridPropertyCount(const IDOSTreeView* view, const QString& gridId) const
{
    if (view == nullptr || view->project() == nullptr || gridId.isEmpty())
    {
        return 0;
    }
    int count = 0;
    for (IDOSDataObject* object : view->project()->objects())
    {
        IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(object);
        if (property != nullptr && property->gridId() == gridId)
        {
            ++count;
        }
    }
    return count;
}

bool IDOSGridTreeGuiProvider::deleteGrid(IDOSTreeView* view, const QString& gridId)
{
    if (view == nullptr || view->project() == nullptr || gridId.isEmpty())
    {
        return false;
    }
    QPointer<IDOSProject> target(view->project());
    IDOSDataObject* obj = target->object(gridId);
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(obj);
    if (grid == nullptr)
    {
        return false;
    }
    const int propertyCount = gridPropertyCount(view, gridId);
    const QMessageBox::StandardButton btn =
        QMessageBox::question(view, tr("Delete Grid"),
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

bool IDOSGridTreeGuiProvider::deleteProperty(IDOSTreeView* view, const QString& propertyId)
{
    if (view == nullptr || view->project() == nullptr || propertyId.isEmpty())
    {
        return false;
    }
    QPointer<IDOSProject> target(view->project());
    IDOSDataObject* obj = target->object(propertyId);
    if (qobject_cast<IDOSGridProperty*>(obj) == nullptr)
    {
        return false;
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(view, tr("Delete Property"),
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

bool IDOSGridTreeGuiProvider::importProperty(IDOSTreeView* view, const QString& targetGridId,
                                            const QString& targetCaseId)
{
    if (view == nullptr || view->project() == nullptr)
    {
        return false;
    }
    QPointer<IDOSProject> target(view->project());
    // 当前能产出网格属性的可识别格式为 ECLIPSE .DATA；后续新增 .GRDECL/.INIT/.UNRST
    // provider 后在此扩充过滤项
    const QString file = QFileDialog::getOpenFileName(
        view, tr("Import Grid Property"), QString(),
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
        QMessageBox::warning(view, tr("Import Grid Property"), commandManager->error());
        return false;
    }

    const int importedCount = commandManager->result().value(QStringLiteral("objectCount")).toInt();
    QMessageBox::information(view, tr("Import Grid Property"),
                             tr("Imported %1 properties.").arg(importedCount));
    return true;
}

bool IDOSGridTreeGuiProvider::importDynamicProperties(IDOSTreeView* view, const QString& targetGridId)
{
    if (view == nullptr || view->project() == nullptr)
    {
        return false;
    }
    QPointer<IDOSProject> target(view->project());
    const QString file = QFileDialog::getOpenFileName(
        view, tr("Import Dynamic Properties"), QString(),
        tr("Unified Eclipse restart files (*.UNRST *.unrst)"));
    if (file.isEmpty() || target.isNull())
    {
        return false;
    }

    IDOSCommandManager* commandManager = target->commandManager();
    if (commandManager == nullptr)
    {
        return false;
    }

    const bool success = commandManager->execute(
        new IDOSImportRestartResultsCommand(target, targetGridId, file));
    if (!success)
    {
        QMessageBox::warning(view, tr("Import Dynamic Properties"), commandManager->error());
        return false;
    }

    const QJsonObject result = commandManager->result();
    QMessageBox::information(
        view, tr("Import Dynamic Properties"),
        tr("Imported %1 dynamic properties across %2 report steps (%3–%4).")
            .arg(result.value(QStringLiteral("propertyCount")).toInt())
            .arg(result.value(QStringLiteral("reportStepCount")).toInt())
            .arg(result.value(QStringLiteral("firstReportStep")).toInt())
            .arg(result.value(QStringLiteral("lastReportStep")).toInt()));
    return true;
}
