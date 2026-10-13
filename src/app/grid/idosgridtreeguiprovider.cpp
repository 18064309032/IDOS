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
#include "command/idosrenameobjectcommand.h"
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
#include "idosgridfilter.h"
#include "idosgridfiltercommand.h"
#include "idosgridfilterwidget.h"
#include "idosgridproperty.h"
#include "idosgridrenderprovider.h"
#include "idosgridsettingwidget.h"
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

IDOSGridTreeGuiProvider::IDOSGridTreeGuiProvider(QObject* parent)
    :
    IDOSTreeGuiProvider(parent)
    , m_widget(nullptr)
    , m_filterWidget(nullptr)
    , m_displayProjects()
    , m_displayViews()
    , m_displayTrees()
{
}

IDOSGridTreeGuiProvider::~IDOSGridTreeGuiProvider()
{
    delete m_widget.data();
    delete m_filterWidget.data();
    const QList<QAction*> actions = m_displayProjects.keys();
    for (QAction* action : actions)
    {
        delete action;
    }
}

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
    return qobject_cast<IDOSGrid*>(data) != nullptr || qobject_cast<IDOSGridProperty*>(data) != nullptr || qobject_cast<IDOSGridFilter*>(data) != nullptr;
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
               ? object->visible() && provider->property() == property
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
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(data);
    if (filter != nullptr)
    {
        QAction* edit = addDisplayAction(menu, view, tr("Edit Filter..."), filter->gridId(), QString());
        edit->setProperty("filterId", filter->objectId());
        edit->setProperty("openFilter", true);
        edit->setObjectName(QStringLiteral("gridFilterEditAction"));
        edit->setEnabled(displayRenderObject(edit) != nullptr);
        connect(edit, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onSettingsTriggered);
        QAction* settings = addDisplayAction(menu, view, tr("Display Settings..."), filter->gridId(), QString());
        settings->setProperty("filterId", filter->objectId());
        settings->setObjectName(QStringLiteral("gridSettingsAction"));
        settings->setEnabled(displayRenderObject(settings) != nullptr);
        connect(settings, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onSettingsTriggered);
        QAction* rename = addAction(menu, view, tr("Rename Filter..."));
        rename->setObjectName(QStringLiteral("gridFilterRenameAction"));
        connect(rename, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onRenameFilter);
        QAction* remove = addAction(menu, view, tr("Delete Filter..."));
        remove->setObjectName(QStringLiteral("gridFilterDeleteAction"));
        connect(remove, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onDeleteFilter);
        return;
    }
    if (item->isGroup() && item->key() == QStringLiteral("grid.filters"))
    {
        QAction* action = addDisplayAction(menu, view, tr("Grid Filter..."), item->dataId(), QString());
        action->setObjectName(QStringLiteral("gridFilterAction"));
        action->setProperty("openFilter", true);
        connect(action, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onSettingsTriggered);
        return;
    }
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
    if (!item->isGroup() && data != nullptr)
    {
        populateDisplayMenu(menu, view);
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
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(data);
    if (filter != nullptr)
    {
        IDOSRenderObject* result = gridRenderObject(filter->objectId(), view);
        if (result == nullptr)
        {
            return false;
        }
        result->setVisible(checked);
        IDOSRenderObject* source = gridRenderObject(filter->gridId(), view);
        IDOSGridRenderProvider* sourceProvider = source != nullptr ? dynamic_cast<IDOSGridRenderProvider*>(source->provider()) : nullptr;
        if (checked && sourceProvider != nullptr)
        {
            if (sourceProvider->setFilterCriteria(filter->criteria()))
            {
                for (const QString& propertyId : IDOSGridFilter::propertyIds(filter->criteria()))
                {
                    sourceProvider->setDataReference(project->object(propertyId));
                }
                sourceProvider->setPreviewFilterId(filter->objectId());
                sourceProvider->update();
                view->scene()->notifyChanged(source->id());
            }
        }
        else if (!checked)
        {
            if (sourceProvider != nullptr && sourceProvider->previewFilterId() == filter->objectId())
            {
                sourceProvider->setFilterCriteria(QVariantMap());
                sourceProvider->update();
                view->scene()->notifyChanged(source->id());
            }
        }
        view->scene()->notifyChanged(result->id());
        emit checkStateChanged();
        return true;
    }
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
                                  "and all saved filters, and remove it from its case.")
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

void IDOSGridTreeGuiProvider::populateDisplayMenu(QMenu* menu, IDOSTreeView* view)
{
    IDOSDataObject* data = view->project()->object(view->menuItem()->dataId());
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(data);
    const QString gridId = property != nullptr ? property->gridId() : data->objectId();
    menu->addSeparator();
    if (property != nullptr)
    {
        QAction* display = addDisplayAction(menu, view, tr("Display in Target View"), gridId, property->objectId());
        display->setObjectName(QStringLiteral("gridDisplayPropertyAction"));
        connect(display, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onDisplayTriggered);
    }
    else
    {
        QMenu* colorMenu = menu->addMenu(tr("Color by"));
        QAction* solid = addDisplayAction(colorMenu, view, tr("Solid color"), gridId, QString());
        connect(solid, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onDisplayTriggered);
        for (IDOSDataObject* object : view->project()->objects())
        {
            IDOSGridProperty* candidate = qobject_cast<IDOSGridProperty*>(object);
            if (candidate != nullptr && candidate->gridId() == gridId)
            {
                QAction* display = addDisplayAction(colorMenu, view, candidate->name(), gridId, candidate->objectId());
                connect(display, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onDisplayTriggered);
            }
        }
    }
    QAction* settings = addDisplayAction(menu, view, tr("Display Settings..."), gridId,
        property != nullptr ? property->objectId() : QString());
    settings->setObjectName(QStringLiteral("gridSettingsAction"));
    connect(settings, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onSettingsTriggered);
    if (property == nullptr)
    {
        QAction* filter = addDisplayAction(menu, view, tr("Grid Filter..."), gridId, QString());
        filter->setObjectName(QStringLiteral("gridFilterAction"));
        filter->setProperty("openFilter", true);
        connect(filter, &QAction::triggered, this, &IDOSGridTreeGuiProvider::onSettingsTriggered);
    }
}

QAction* IDOSGridTreeGuiProvider::addDisplayAction(QMenu* menu, IDOSTreeView* view, const QString& text,
                                                 const QString& gridId, const QString& propertyId)
{
    QAction* action = menu->addAction(text);
    action->setProperty("gridId", gridId);
    action->setProperty("propertyId", propertyId);
    m_displayProjects.insert(action, view->project());
    m_displayViews.insert(action, qobject_cast<IDOSRenderView*>(view->targetView()));
    m_displayTrees.insert(action, view);
    action->setEnabled(displayRenderObject(action) != nullptr);
    connect(action, &QObject::destroyed, this, &IDOSGridTreeGuiProvider::onDisplayActionDestroyed);
    return action;
}

IDOSRenderObject* IDOSGridTreeGuiProvider::displayRenderObject(QAction* action) const
{
    IDOSProject* project = m_displayProjects.value(action).data();
    IDOSRenderView* view = m_displayViews.value(action).data();
    IDOSTreeView* tree = m_displayTrees.value(action).data();
    if (action == nullptr || project == nullptr || view == nullptr || tree == nullptr || tree->project() != project)
    {
        return nullptr;
    }
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(project->object(action->property("gridId").toString()));
    const QString propertyId = action->property("propertyId").toString();
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(project->object(propertyId));
    if (grid == nullptr || (!propertyId.isEmpty() && (property == nullptr || property->gridId() != grid->objectId())))
    {
        return nullptr;
    }
    const QString filterId = action->property("filterId").toString();
    IDOSDataObject* source = grid;
    if (!filterId.isEmpty())
    {
        IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(project->object(filterId));
        if (filter == nullptr || filter->gridId() != grid->objectId())
        {
            return nullptr;
        }
        source = filter;
    }
    IDOSRenderObject* object = gridRenderObject(source->objectId(), view);
    return object != nullptr && object->provider()->dataSource() == source ? object : nullptr;
}

void IDOSGridTreeGuiProvider::onSettingsTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSRenderObject* object = displayRenderObject(action);
    if (object == nullptr)
    {
        return;
    }
    IDOSProject* project = m_displayProjects.value(action).data();
    IDOSGrid* grid = qobject_cast<IDOSGrid*>(project->object(action->property("gridId").toString()));
    IDOSRenderView* view = m_displayViews.value(action).data();
    QWidget* parent = m_displayTrees.value(action)->window();
    if (action->property("openFilter").toBool())
    {
        delete m_filterWidget.data();
        m_filterWidget = new IDOSGridFilterWidget(parent);
        m_filterWidget->setProject(project);
        m_filterWidget->setGrid(grid);
        m_filterWidget->setView(view);
        m_filterWidget->setFilter(action->property("filterId").toString());
        m_filterWidget->show();
    }
    else
    {
        delete m_widget.data();
        m_widget = new IDOSGridSettingWidget(parent);
        m_widget->setProject(project);
        m_widget->setGrid(grid);
        m_widget->setView(view);
        m_widget->setPropertyId(action->property("propertyId").toString());
        if (!action->property("filterId").toString().isEmpty())
        {
            m_widget->setRenderTarget(action->property("filterId").toString());
        }
        m_widget->show();
    }
}

void IDOSGridTreeGuiProvider::onDisplayTriggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSRenderObject* object = displayRenderObject(action);
    if (object == nullptr)
    {
        return;
    }
    IDOSGridRenderProvider* target = dynamic_cast<IDOSGridRenderProvider*>(object->provider());
    IDOSGridProperty* property = qobject_cast<IDOSGridProperty*>(m_displayProjects.value(action)->object(action->property("propertyId").toString()));
    target->setProperty(property);
    object->setVisible(true);
    m_displayViews.value(action)->scene()->notifyChanged(object->id());
    emit checkStateChanged();
}

void IDOSGridTreeGuiProvider::onDisplayActionDestroyed(QObject* object)
{
    QAction* action = static_cast<QAction*>(object);
    m_displayProjects.remove(action);
    m_displayViews.remove(action);
    m_displayTrees.remove(action);
}

void IDOSGridTreeGuiProvider::onDeleteFilter()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(view->project());
    const QString filterId = action->property("dataId").toString();
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(project->object(filterId));
    if (filter == nullptr)
    {
        return;
    }
    const QMessageBox::StandardButton answer = QMessageBox::question(view, tr("Delete Filter"),
        tr("Delete filter \"%1\"?").arg(filter->name()), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::Yes && project != nullptr && project->commandManager() != nullptr)
    {
        QJsonObject arguments;
        arguments.insert(QStringLiteral("filterId"), filterId);
        if (!project->commandManager()->execute(new IDOSGridFilterCommand(project.data(), QStringLiteral("delete"), arguments)))
        {
            QMessageBox::warning(view, tr("Delete Filter"), project->commandManager()->error());
        }
    }
}

void IDOSGridTreeGuiProvider::onRenameFilter()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    IDOSGridFilter* filter = qobject_cast<IDOSGridFilter*>(view->project()->object(action->property("dataId").toString()));
    if (filter == nullptr)
    {
        return;
    }
    QInputDialog* dialog = new QInputDialog(view);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(tr("Rename Filter"));
    dialog->setLabelText(tr("Filter name:"));
    dialog->setTextValue(filter->name());
    dialog->setProperty("filterId", filter->objectId());
    dialog->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    connect(dialog, &QDialog::accepted, this, &IDOSGridTreeGuiProvider::onFilterRenameAccepted);
    connect(view->project(), &QObject::destroyed, dialog, &QObject::deleteLater);
    dialog->open();
}

void IDOSGridTreeGuiProvider::onFilterRenameAccepted()
{
    QInputDialog* dialog = qobject_cast<QInputDialog*>(sender());
    IDOSTreeView* view = dialog != nullptr ? qobject_cast<IDOSTreeView*>(dialog->parentWidget()) : nullptr;
    if (view == nullptr || view->project() == nullptr || dialog->property("project").value<QObject*>() != view->project())
    {
        return;
    }
    const QString filterId = dialog->property("filterId").toString();
    if (qobject_cast<IDOSGridFilter*>(view->project()->object(filterId)) == nullptr)
    {
        return;
    }
    view->project()->commandManager()->execute(new IDOSRenameObjectCommand(view->project(), filterId, dialog->textValue()));
}
