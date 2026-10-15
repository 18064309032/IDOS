#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QStringList>
#include <QVariant>

#include "command/idosdeletedatacommand.h"
#include "data/fracture/idosfracturematerialcatalog.h"
#include "data/fracture/idosfractureschedule.h"
#include "data/fracture/idosnaturalfractureset.h"
#include "data/grid/idosgridboundary.h"
#include "data/grid/idosgridsurface.h"
#include "data/production/idosproductionhistory.h"
#include "data/seismic/idosmicroseismicset.h"
#include "data/seismic/idosseismicvolume.h"
#include "idoscommandmanager.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "providers/idosrenderprovider.h"
#include "providers/seismic/idosseismicvolumerenderprovider.h"
#include "tree/idostree.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeview.h"

#include "idosimporteddatatreeguiprovider.h"

QString IDOSImportedDataTreeGuiProvider::name() const
{
    return QStringLiteral("idos.importedData");
}

bool IDOSImportedDataTreeGuiProvider::supports(const IDOSTreeItem* item) const
{
    if (item == nullptr || item->tree() == nullptr || item->tree()->name() != QStringLiteral("data"))
    {
        return false;
    }
    const QString key = item->key();
    if (key == QStringLiteral("data.microseismic") ||
        key == QStringLiteral("data.seismic") ||
        key == QStringLiteral("data.hydraulicFractures") ||
        key == QStringLiteral("data.naturalFractures") ||
        key == QStringLiteral("data.production") ||
        key == QStringLiteral("data.fluids") ||
        key == QStringLiteral("data.surfaceProperties") ||
        key == QStringLiteral("data.boundaries"))
    {
        return true;
    }
    IDOSProject* project = item->tree()->project();
    IDOSDataObject* object = project != nullptr ? project->object(item->dataId()) : nullptr;
    return qobject_cast<IDOSMicroseismicSet*>(object) != nullptr ||
           qobject_cast<IDOSFractureSchedule*>(object) != nullptr ||
           qobject_cast<IDOSNaturalFractureSet*>(object) != nullptr ||
           qobject_cast<IDOSProductionHistory*>(object) != nullptr ||
           qobject_cast<IDOSSeismicVolume*>(object) != nullptr ||
           qobject_cast<IDOSFractureMaterialCatalog*>(object) != nullptr ||
           qobject_cast<IDOSGridSurface*>(object) != nullptr ||
           qobject_cast<IDOSGridBoundary*>(object) != nullptr;
}

bool IDOSImportedDataTreeGuiProvider::isCheckable(const IDOSTreeItem* item) const
{
    if (!supports(item) || item == nullptr || item->tree() == nullptr || item->tree()->project() == nullptr)
    {
        return false;
    }
    IDOSDataObject* object = item->tree()->project()->object(item->dataId());
    return qobject_cast<IDOSGridSurface*>(object) != nullptr ||
           qobject_cast<IDOSGridBoundary*>(object) != nullptr ||
           qobject_cast<IDOSNaturalFractureSet*>(object) != nullptr ||
           qobject_cast<IDOSSeismicVolume*>(object) != nullptr;
}

bool IDOSImportedDataTreeGuiProvider::isChecked(const IDOSTreeItem* item, const IDOSTreeView* view) const
{
    IDOSRenderView* renderView = view != nullptr ? qobject_cast<IDOSRenderView*>(view->targetView()) : nullptr;
    if (!isCheckable(item) || renderView == nullptr)
    {
        return false;
    }
    IDOSDataObject* data = item->tree()->project()->object(item->dataId());
    for (IDOSRenderObject* object : renderView->scene()->objects())
    {
        if (object->provider()->dataSource() == data)
        {
            return object->visible();
        }
    }
    return false;
}

bool IDOSImportedDataTreeGuiProvider::setChecked(IDOSTreeItem* item, bool checked, IDOSTreeView* view)
{
    IDOSRenderView* renderView = view != nullptr ? qobject_cast<IDOSRenderView*>(view->targetView()) : nullptr;
    if (!isCheckable(item) || renderView == nullptr)
    {
        return false;
    }
    IDOSDataObject* data = item->tree()->project()->object(item->dataId());
    for (IDOSRenderObject* object : renderView->scene()->objects())
    {
        if (object->provider()->dataSource() == data)
        {
            object->setVisible(checked);
            renderView->scene()->notifyChanged(object->id());
            if (checked)
            {
                renderView->resetCamera();
            }
            emit checkStateChanged();
            return true;
        }
    }
    return false;
}

QAction* IDOSImportedDataTreeGuiProvider::addImportAction(
    QMenu* menu, IDOSTreeView* view, const QString& text,
    const QString& providerId, const QString& dataTypeId, const QString& fileFilter)
{
    QAction* action = menu->addAction(text);
    action->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    action->setProperty("providerId", providerId);
    action->setProperty("dataTypeId", dataTypeId);
    action->setProperty("fileFilter", fileFilter);
    return action;
}

IDOSTreeView* IDOSImportedDataTreeGuiProvider::actionView() const
{
    QAction* action = qobject_cast<QAction*>(sender());
    QMenu* menu = action != nullptr ? qobject_cast<QMenu*>(action->parent()) : nullptr;
    IDOSTreeView* view = menu != nullptr ? qobject_cast<IDOSTreeView*>(menu->parentWidget()) : nullptr;
    if (action == nullptr || view == nullptr || view->project() == nullptr ||
        action->property("project").value<QObject*>() != view->project())
    {
        return nullptr;
    }
    return view;
}

void IDOSImportedDataTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* view)
{
    if (menu == nullptr || view == nullptr || view->project() == nullptr || view->menuItem() == nullptr)
    {
        return;
    }
    const QString key = view->menuItem()->key();
    QAction* action = nullptr;
    if (key == QStringLiteral("data.microseismic"))
    {
        action = addImportAction(menu, view, tr("Import Microseismic Events..."),
                                 QStringLiteral("idos.microseismic.tsv"),
                                 QStringLiteral("idos.microseismic.set"),
                                 tr("Microseismic Event Tables (*.txt *.TXT);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.seismic"))
    {
        action = addImportAction(menu, view, tr("Import SEG-Y Seismic Volume..."),
                                 QStringLiteral("idos.seismic.segy"),
                                 QStringLiteral("idos.seismic.volume"),
                                 tr("SEG-Y Seismic Volumes (*.sgy *.SGY *.segy *.SEGY);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.hydraulicFractures"))
    {
        action = addImportAction(menu, view, tr("Import Fracture Pump Schedule..."),
                                 QStringLiteral("idos.fracture.schedule.tsv"),
                                 QStringLiteral("idos.fracture.schedule"),
                                 tr("Fracture Pump Schedules (*.txt *.TXT *.xlsx *.XLSX);;All Files (*)"));
        QAction* catalogAction = addImportAction(
            menu, view, tr("Import Proppant Catalog..."),
            QStringLiteral("idos.fracture.materialCatalog.tsv"),
            QStringLiteral("idos.fracture.proppantCatalog"),
            tr("Proppant Catalog (*.txt *.TXT);;All Files (*)"));
        connect(catalogAction, &QAction::triggered, this,
                &IDOSImportedDataTreeGuiProvider::onImportDataset);
    }
    else if (key == QStringLiteral("data.naturalFractures"))
    {
        action = addImportAction(menu, view, tr("Import Natural Fractures..."),
                                 QStringLiteral("idos.naturalFracture.ascii"),
                                 QStringLiteral("idos.naturalFracture.set"),
                                 tr("ASCII Natural Fracture Files (*.fab *.FAB);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.production"))
    {
        action = addImportAction(menu, view, tr("Import Production History..."),
                                 QStringLiteral("idos.production.vol"),
                                 QStringLiteral("idos.production.history"),
                                 tr("Production History (*.vol *.VOL);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.fluids"))
    {
        action = addImportAction(menu, view, tr("Import Fluid Catalog..."),
                                 QStringLiteral("idos.fracture.materialCatalog.tsv"),
                                 QStringLiteral("idos.fracture.fluidCatalog"),
                                 tr("Fluid Catalog (*.txt *.TXT);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.surfaceProperties"))
    {
        action = addImportAction(menu, view, tr("Import Grid Surface..."),
                                 QStringLiteral("idos.grid.surface.zims"),
                                 QStringLiteral("idos.grid.surface"),
                                 tr("ZIMS Grid Surface (*);;All Files (*)"));
    }
    else if (key == QStringLiteral("data.boundaries"))
    {
        action = addImportAction(menu, view, tr("Import Grid Boundary..."),
                                 QStringLiteral("idos.grid.boundary.xygi"),
                                 QStringLiteral("idos.grid.boundary"),
                                 tr("Grid Boundary Point Files (*);;All Files (*)"));
    }
    else if (key.startsWith(QStringLiteral("object:")) &&
             qobject_cast<IDOSSeismicVolume*>(
                 view->project()->object(view->menuItem()->dataId())) != nullptr)
    {
        QAction* inlineAction = menu->addAction(tr("Select Inline Section..."));
        inlineAction->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
        inlineAction->setProperty("dataId", view->menuItem()->dataId());
        connect(inlineAction, &QAction::triggered, this,
                &IDOSImportedDataTreeGuiProvider::onSelectSeismicInline);
    }
    if (action != nullptr)
    {
        connect(action, &QAction::triggered, this,
                &IDOSImportedDataTreeGuiProvider::onImportDataset);
    }
}

void IDOSImportedDataTreeGuiProvider::onSelectSeismicInline()
{
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSTreeView* view = actionView();
    IDOSProject* project = view != nullptr ? view->project() : nullptr;
    IDOSSeismicVolume* volume = project != nullptr
        ? qobject_cast<IDOSSeismicVolume*>(project->object(action->property("dataId").toString()))
        : nullptr;
    IDOSRenderView* renderView = view != nullptr
        ? qobject_cast<IDOSRenderView*>(view->targetView()) : nullptr;
    if (action == nullptr || volume == nullptr || renderView == nullptr)
    {
        return;
    }
    IDOSSeismicVolumeRenderProvider* renderProvider = nullptr;
    IDOSRenderObject* renderObject = nullptr;
    for (IDOSRenderObject* candidate : renderView->scene()->objects())
    {
        IDOSSeismicVolumeRenderProvider* candidateProvider =
            static_cast<IDOSSeismicVolumeRenderProvider*>(candidate->provider());
        if (candidateProvider != nullptr && candidateProvider->dataSource() == volume)
        {
            renderProvider = candidateProvider;
            renderObject = candidate;
            break;
        }
    }
    if (renderProvider == nullptr || renderObject == nullptr)
    {
        return;
    }
    QStringList inlineOptions;
    for (int inlineNumber : renderProvider->inlineNumbers())
    {
        inlineOptions.append(QString::number(inlineNumber));
    }
    if (inlineOptions.isEmpty())
    {
        return;
    }
    bool accepted = false;
    const QString selectedInline = QInputDialog::getItem(
        view, tr("Select Inline Section"), tr("Inline number:"), inlineOptions,
        inlineOptions.indexOf(QString::number(renderProvider->inlineNumber())), false, &accepted);
    if (!accepted)
    {
        return;
    }
    bool valid = false;
    const int inlineNumber = selectedInline.toInt(&valid);
    if (!valid || !renderProvider->setInlineNumber(inlineNumber))
    {
        return;
    }
    renderView->scene()->notifyChanged(renderObject->id());
    renderView->resetCamera();
}

void IDOSImportedDataTreeGuiProvider::onImportDataset()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    if (view == nullptr || action == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(view->project());
    const QString providerId = action->property("providerId").toString();
    const QString dataTypeId = action->property("dataTypeId").toString();
    const QString title = action->text();
    const QStringList files = QFileDialog::getOpenFileNames(
        view, title, QString(), action->property("fileFilter").toString());
    if (files.isEmpty() || project.isNull() || project->commandManager() == nullptr)
    {
        return;
    }
    QStringList failures;
    for (const QString& file : files)
    {
        QJsonObject arguments;
        arguments.insert(QStringLiteral("filePath"), file);
        arguments.insert(QStringLiteral("providerId"), providerId);
        arguments.insert(QStringLiteral("dataTypeId"), dataTypeId);
        const QJsonObject response = project->commandManager()->execute(
            QStringLiteral("data.import"), arguments);
        if (!response.value(QStringLiteral("success")).toBool())
        {
            failures.append(tr("Failed to import %1:\n%2")
                .arg(file, response.value(QStringLiteral("error")).toString()));
        }
    }
    if (!failures.isEmpty())
    {
        QMessageBox::warning(view, title, failures.join(QStringLiteral("\n\n")));
    }
}

void IDOSImportedDataTreeGuiProvider::populateSelectionMenu(QMenu* menu, IDOSTreeView* view)
{
    if (menu == nullptr || view == nullptr || view->project() == nullptr)
    {
        return;
    }
    const QList<IDOSTreeItem*> items = view->selectedItems();
    QStringList ids;
    for (IDOSTreeItem* item : items)
    {
        IDOSDataObject* object = view->project()->object(item->dataId());
        if (!IDOSDeleteDataCommand::supportsObject(object))
        {
            return;
        }
        if (!ids.contains(object->objectId()))
        {
            ids.append(object->objectId());
        }
    }
    if (ids.isEmpty())
    {
        return;
    }
    QAction* action = menu->addAction(tr("Delete Data..."));
    action->setObjectName(QStringLiteral("deleteDataAction"));
    action->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    action->setProperty("objectIds", ids);
    connect(action, &QAction::triggered, this, &IDOSImportedDataTreeGuiProvider::onDeleteDatasets);
}

void IDOSImportedDataTreeGuiProvider::onDeleteDatasets()
{
    QPointer<IDOSTreeView> view(actionView());
    QAction* action = qobject_cast<QAction*>(sender());
    if (view.isNull() || action == nullptr)
    {
        return;
    }
    QPointer<IDOSProject> project(view->project());
    const QStringList ids = action->property("objectIds").toStringList();
    QStringList names;
    for (const QString& id : ids)
    {
        IDOSDataObject* object = project->object(id);
        if (!IDOSDeleteDataCommand::supportsObject(object))
        {
            return;
        }
        names.append(object->name());
    }
    if (ids.isEmpty())
    {
        return;
    }
    const QMessageBox::StandardButton answer = QMessageBox::question(view.data(), tr("Delete Data"),
        tr("Delete %1 selected dataset(s)?\nTheir case references will also be removed. You can undo this operation.\n\n%2")
            .arg(ids.size()).arg(names.join(QStringLiteral("\n"))),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes || view.isNull() || project.isNull()
        || view->project() != project.data() || project->commandManager() == nullptr)
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("objectIds"), QJsonArray::fromStringList(ids));
    const QJsonObject response = project->commandManager()->execute(QStringLiteral("data.delete"), arguments);
    if (!response.value(QStringLiteral("success")).toBool())
    {
        QMessageBox::warning(view.data(), tr("Delete Data"), response.value(QStringLiteral("error")).toString());
    }
}
