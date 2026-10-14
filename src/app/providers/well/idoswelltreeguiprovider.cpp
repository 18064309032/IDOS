#include <QAction>
#include <QDialog>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVariant>
#include <QVBoxLayout>

#include "command/idoscreatecasecommand.h"
#include "command/idoscreatewellcommand.h"
#include "command/idosdeletecasecommand.h"
#include "command/idosdeletegridcommand.h"
#include "command/idosdeletepropertycommand.h"
#include "command/idosdeletewellcommand.h"
#include "command/idosimportcommands.h"
#include "data/case/idoscaseobject.h"
#include "data/case/idossimulationcaseobject.h"
#include "data/grid/idosgrid.h"
#include "data/grid/idosgridproperty.h"
#include "data/well/idoswell.h"
#include "idoscasedialog.h"
#include "idoscommandmanager.h"
#include "idosdataobject.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"
#include "logger/idoslogger.h"
#include "providers/idosdataprovider.h"
#include "providers/idosdataproviderregistry.h"
#include "providers/idosrenderprovider.h"
#include "tree/idostree.h"
#include "tree/idostreeitem.h"
#include "tree/idostreeview.h"

#include "idoswelltreeguiprovider.h"

QString IDOSWellTreeGuiProvider::name() const
{
    return QStringLiteral("idos.well");
}

QAction* IDOSWellTreeGuiProvider::addAction(QMenu* menu, IDOSTreeView* view, const QString& text)
{
    QAction* action = menu->addAction(text);
    IDOSTreeItem* item = view->menuItem();
    action->setProperty("dataId", item != nullptr ? item->dataId() : QString());
    action->setProperty("caseId", item != nullptr ? item->caseId() : QString());
    action->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    return action;
}

IDOSTreeView* IDOSWellTreeGuiProvider::actionView() const
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

bool IDOSWellTreeGuiProvider::supports(const IDOSTreeItem* item) const
{
    if (item == nullptr)
    {
        return false;
    }
    if (item->key() == QStringLiteral("data.wells"))
    {
        return true;
    }
    return item->tree() != nullptr && item->tree()->project() != nullptr &&
           qobject_cast<IDOSWell*>(item->tree()->project()->object(item->dataId())) != nullptr;
}

bool IDOSWellTreeGuiProvider::isChecked(const IDOSTreeItem* item,
                                       const IDOSTreeView* treeView) const
{
    IDOSRenderView* view = treeView != nullptr ? qobject_cast<IDOSRenderView*>(treeView->targetView()) : nullptr;
    if (item == nullptr || item->tree() == nullptr || item->tree()->project() == nullptr || view == nullptr)
    {
        return false;
    }
    IDOSWell* well = qobject_cast<IDOSWell*>(item->tree()->project()->object(item->dataId()));
    if (well == nullptr)
    {
        return false;
    }
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        if (object->provider()->dataSource() == well)
        {
            return object->visible();
        }
    }
    return false;
}

bool IDOSWellTreeGuiProvider::setChecked(IDOSTreeItem* item,
                                         bool checked,
                                         IDOSTreeView* treeView)
{
    IDOSRenderView* view = treeView != nullptr ? qobject_cast<IDOSRenderView*>(treeView->targetView()) : nullptr;
    if (item == nullptr || item->tree() == nullptr || item->tree()->project() == nullptr || view == nullptr)
    {
        return false;
    }
    IDOSWell* well = qobject_cast<IDOSWell*>(item->tree()->project()->object(item->dataId()));
    if (well == nullptr)
    {
        return false;
    }
    for (IDOSRenderObject* object : view->scene()->objects())
    {
        if (object->provider()->dataSource() == well)
        {
            object->setVisible(checked);
            view->scene()->notifyChanged(object->id());
            if (checked)
            {
                view->resetCamera();
            }
            emit checkStateChanged();
            return true;
        }
    }
    return false;
}

void IDOSWellTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* view)
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
    if (item->key() == QStringLiteral("data.wells"))
    {
        if (view->tree() == nullptr || view->tree()->name() != QStringLiteral("data"))
        {
            return;
        }
        QAction* create = addAction(menu, view, tr("New Well..."));
        create->setObjectName(QStringLiteral("newWellAction"));
        connect(create, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onNewWell);
        QAction* import = addAction(menu, view, tr("Import Well Data..."));
        import->setObjectName(QStringLiteral("importWellAction"));
        connect(import, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onImportWell);
        return;
    }
    IDOSWell* well = qobject_cast<IDOSWell*>(view->project()->object(item->dataId()));
    if (well == nullptr)
    {
        return;
    }
    const QString part = item->partKey();
    const bool isWellItem = item->parent() != nullptr &&
                            item->parent()->key() == QStringLiteral("data.wells") &&
                            item->key().startsWith(QStringLiteral("object:"));
    const bool isLeafItem = item->parent() != nullptr &&
                            item->parent()->key().startsWith(QStringLiteral("idos.well."));
    if (isWellItem)
    {
        QAction* importHeader = addAction(menu, view, tr("Import Well Header..."));
        importHeader->setObjectName(QStringLiteral("importWellHeaderAction"));
        connect(importHeader, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onImportWell);
        QAction* importWell = addAction(menu, view, tr("Import Well Data..."));
        importWell->setObjectName(QStringLiteral("importWellDataSingleAction"));
        connect(importWell, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onImportWell);
        QAction* show = addAction(menu, view, tr("View Well Data..."));
        show->setObjectName(QStringLiteral("viewWellSummaryAction"));
        show->setProperty("part", QStringLiteral("idos.well.summary"));
        connect(show, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onViewWellData);
    }
    else if (!isLeafItem && part == QStringLiteral("idos.well.trajectory"))
    {
        QAction* show = addAction(menu, view, tr("View Trajectory Data..."));
        show->setObjectName(QStringLiteral("viewWellPathAction"));
        show->setProperty("part", part);
        connect(show, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onViewWellData);
        QAction* import = addAction(menu, view, tr("Import or Replace Trajectory..."));
        import->setObjectName(QStringLiteral("importWellPathAction"));
        connect(import, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onImportWell);
        return;
    }
    else if (part == QStringLiteral("idos.well.logs"))
    {
        if (!isLeafItem)
        {
            QAction* import = addAction(menu, view, tr("Import Well Curves..."));
            import->setObjectName(QStringLiteral("importWellCurvesAction"));
            import->setProperty("part", part);
            connect(import, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onImportWell);
        }
        QAction* show = addAction(menu, view, tr("View Curve Data..."));
        show->setObjectName(QStringLiteral("viewWellInputDataAction"));
        show->setProperty("part", part);
        show->setProperty("itemKey", item->itemKey());
        connect(show, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onViewWellData);
    }
    else if (!isLeafItem && (part == QStringLiteral("idos.well.zones") || part == QStringLiteral("idos.well.completions") ||
        part == QStringLiteral("idos.well.checkshots")))
    {
        const QString label = part == QStringLiteral("idos.well.zones") ? tr("Markers")
            : part == QStringLiteral("idos.well.completions") ? tr("Completions") : tr("Checkshots");
        QAction* import = addAction(menu, view, tr("Import %1 Data...").arg(label));
        import->setObjectName(QStringLiteral("importWellPartAction"));
        import->setProperty("part", part);
        connect(import, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onImportWell);
        QAction* show = addAction(menu, view, tr("View %1 Data...").arg(label));
        show->setObjectName(QStringLiteral("viewWellInputDataAction"));
        show->setProperty("part", part);
        connect(show, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onViewWellData);
    }
    else if (isWellItem && part == QStringLiteral("idos.well.trajectory"))
    {
        QAction* importHeader = addAction(menu, view, tr("Import Well Header..."));
        importHeader->setObjectName(QStringLiteral("importWellHeaderAction"));
        connect(importHeader, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onImportWell);
    }
    else if (isLeafItem && part == QStringLiteral("idos.well.zones"))
    {
        QAction* show = addAction(menu, view, tr("View Marker Data..."));
        show->setObjectName(QStringLiteral("viewWellInputDataAction"));
        show->setProperty("part", part);
        show->setProperty("itemKey", item->itemKey());
        connect(show, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onViewWellData);
    }
    if (isWellItem)
    {
        QAction* remove = addAction(menu, view, tr("Delete Well..."));
        remove->setObjectName(QStringLiteral("deleteWellAction"));
        connect(remove, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onDeleteWell);
    }
}

void IDOSWellTreeGuiProvider::onViewWellData()
{
    IDOSTreeView* view = actionView();
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSWell* well = view != nullptr && action != nullptr
        ? qobject_cast<IDOSWell*>(view->project()->object(action->property("dataId").toString())) : nullptr;
    if (well == nullptr)
    {
        return;
    }
    QDialog* dialog = new QDialog(view);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setObjectName(QStringLiteral("wellInputDataWindow"));
    dialog->setWindowTitle(tr("Well Data - %1").arg(well->name()));
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    QTableWidget* table = new QTableWidget(dialog);
    table->setEditTriggers(QTableWidget::NoEditTriggers);
    const QString part = action->property("part").toString();
    if (part == QStringLiteral("idos.well.summary"))
    {
        table->setColumnCount(2);
        table->setHorizontalHeaderLabels({tr("Data"), tr("Count")});
        QStringList labels;
        QList<int> counts;
        if (well->hasPath())
        {
            labels.append(tr("Trajectory points"));
            counts.append(well->path().pointCount());
        }
        if (well->hasLogs())
        {
            labels.append(tr("Log curves"));
            counts.append(well->logs().channelCount());
        }
        if (well->hasMarkers())
        {
            labels.append(tr("Markers"));
            counts.append(well->markers().markerCount());
        }
        if (well->hasCompletions())
        {
            labels.append(tr("Completions"));
            counts.append(well->completions().size());
        }
        if (well->hasCheckshot())
        {
            labels.append(tr("Checkshot samples"));
            counts.append(well->checkshot().depths().size());
        }
        table->setRowCount(labels.size());
        for (int row = 0; row < labels.size(); ++row)
        {
            table->setItem(row, 0, new QTableWidgetItem(labels.at(row)));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(counts.at(row))));
        }
    }
    else if (part == QStringLiteral("idos.well.trajectory"))
    {
        const QVector<IDOSWellPathPoint> points = well->path().points();
        QStringList attributeHeaders;
        for (const IDOSWellPathPoint& point : points)
        {
            const QStringList keys = point.attributes().keys();
            for (const QString& key : keys)
            {
                if (!attributeHeaders.contains(key))
                {
                    attributeHeaders.append(key);
                }
            }
        }
        table->setColumnCount(5 + attributeHeaders.size());
        QStringList headers = {tr("Measured Depth"), tr("X"), tr("Y"), tr("Z"), tr("True Vertical Depth")};
        headers.append(attributeHeaders);
        table->setHorizontalHeaderLabels(headers);
        table->setRowCount(points.size());
        for (int row = 0; row < points.size(); ++row)
        {
            const IDOSWellPathPoint& point = points.at(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(point.md(), 'g', 15)));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(point.x(), 'g', 15)));
            table->setItem(row, 2, new QTableWidgetItem(QString::number(point.y(), 'g', 15)));
            table->setItem(row, 3, new QTableWidgetItem(QString::number(point.z(), 'g', 15)));
            table->setItem(row, 4, new QTableWidgetItem(QString::number(point.tvd(), 'g', 15)));
            const QVariantMap attributes = point.attributes();
            for (int column = 0; column < attributeHeaders.size(); ++column)
            {
                table->setItem(row, 5 + column,
                               new QTableWidgetItem(attributes.value(attributeHeaders.at(column)).toString()));
            }
        }
    }
    else if (part == QStringLiteral("idos.well.zones"))
    {
        table->setColumnCount(2);
        table->setHorizontalHeaderLabels({tr("Horizon"), tr("Measured depth (m)")});
        QList<IDOSWellMarker> markers = well->markers().markers();
        const QString markerName = action->property("itemKey").toString();
        if (!markerName.isEmpty())
        {
            for (int index = markers.size() - 1; index >= 0; --index)
            {
                if (markers.at(index).name() != markerName)
                {
                    markers.removeAt(index);
                }
            }
        }
        table->setRowCount(markers.size());
        for (int row = 0; row < markers.size(); ++row)
        {
            table->setItem(row, 0, new QTableWidgetItem(markers.at(row).name()));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(markers.at(row).depth(), 'g', 15)));
        }
    }
    else if (part == QStringLiteral("idos.well.completions"))
    {
        const QList<IDOSWellCompletion> completions = well->completions();
        QStringList attributeHeaders;
        for (const IDOSWellCompletion& completion : completions)
        {
            const QStringList keys = completion.attributes().keys();
            for (const QString& key : keys)
            {
                if (!attributeHeaders.contains(key))
                {
                    attributeHeaders.append(key);
                }
            }
        }
        table->setColumnCount(5 + attributeHeaders.size());
        QStringList headers = {tr("Top depth (m)"), tr("Bottom depth (m)"), tr("Grid layer"), tr("Status"), tr("Event date")};
        headers.append(attributeHeaders);
        table->setHorizontalHeaderLabels(headers);
        table->setRowCount(completions.size());
        for (int row = 0; row < completions.size(); ++row)
        {
            const IDOSWellCompletion& completion = completions.at(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(completion.top(), 'g', 15)));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(completion.bottom(), 'g', 15)));
            table->setItem(row, 2, new QTableWidgetItem(completion.k() > 0 ? QString::number(completion.k()) : tr("Not mapped")));
            table->setItem(row, 3, new QTableWidgetItem(completion.isOpen() ? tr("Open") : tr("Shut")));
            table->setItem(row, 4, new QTableWidgetItem(completion.eventDate()));
            const QVariantMap attributes = completion.attributes();
            for (int column = 0; column < attributeHeaders.size(); ++column)
            {
                const QVariant value = attributes.value(attributeHeaders.at(column));
                table->setItem(row, 5 + column, new QTableWidgetItem(value.toString()));
            }
        }
    }
    else if (part == QStringLiteral("idos.well.logs"))
    {
        const QString channelName = action->property("itemKey").toString();
        QList<IDOSWellLogChannel> channels;
        for (const IDOSWellLogChannel& channel : well->logs().channels())
        {
            if (channelName.isEmpty() || channel.name() == channelName)
            {
                channels.append(channel);
            }
        }
        table->setColumnCount(channels.size() + 1);
        QStringList headers = {tr("Depth (m)")};
        for (const IDOSWellLogChannel& channel : channels)
        {
            const QString unit = channel.unit().isEmpty() ? QString() : QStringLiteral(" (") + channel.unit() + QLatin1Char(')');
            headers.append(channel.name() + unit);
        }
        table->setHorizontalHeaderLabels(headers);
        table->setRowCount(channels.isEmpty() ? 0 : channels.first().sampleCount());
        for (int row = 0; row < table->rowCount(); ++row)
        {
            table->setItem(row, 0, new QTableWidgetItem(QString::number(channels.first().depths().at(row), 'g', 15)));
            for (int column = 0; column < channels.size(); ++column)
            {
                const double value = channels.at(column).values().at(row);
                table->setItem(row, column + 1, new QTableWidgetItem(qIsNaN(value) ? QString() : QString::number(value, 'g', 15)));
            }
        }
    }
    else
    {
        const IDOSWellCheckshot& checkshot = well->checkshot();
        table->setColumnCount(2);
        const QString depthUnit = checkshot.depthUnit().isEmpty() ? tr("Unspecified unit") : checkshot.depthUnit();
        const QString timeUnit = checkshot.timeUnit().isEmpty() ? tr("Unspecified unit") : checkshot.timeUnit();
        const QString timeName = checkshot.timeType() == QStringLiteral("TWT") ? tr("Two-way time")
            : checkshot.timeType() == QStringLiteral("OWT") ? tr("One-way time") : tr("Time (type unspecified)");
        const QString depthName = checkshot.depthType() == QStringLiteral("TVD")
            ? tr("True vertical depth (%1)").arg(depthUnit) : tr("Depth (%1)").arg(depthUnit);
        table->setHorizontalHeaderLabels({depthName, tr("%1 (%2)").arg(timeName, timeUnit)});
        table->setRowCount(checkshot.depths().size());
        for (int row = 0; row < checkshot.depths().size(); ++row)
        {
            table->setItem(row, 0, new QTableWidgetItem(QString::number(checkshot.depths().at(row), 'g', 15)));
            table->setItem(row, 1, new QTableWidgetItem(QString::number(checkshot.times().at(row), 'g', 15)));
        }
    }
    table->resizeColumnsToContents();
    layout->addWidget(table);
    dialog->resize(620, 400);
    dialog->show();
}

void IDOSWellTreeGuiProvider::onNewWell()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    Q_UNUSED(action);
    IDOSWellTreeGuiProvider::newWell(view->project(), view);
}

void IDOSWellTreeGuiProvider::onImportWell()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    const QString id = action->property("dataId").toString();
    if (id.isEmpty())
    {
        IDOSWellTreeGuiProvider::importWellData(view->project(), view);
    }
    else if (qobject_cast<IDOSWell*>(view->project()->object(id)) != nullptr &&
             (action->objectName() == QStringLiteral("importWellHeaderAction") ||
              action->objectName() == QStringLiteral("importWellPathAction") ||
              action->objectName() == QStringLiteral("importWellCurvesAction") ||
              action->objectName() == QStringLiteral("importWellPartAction")))
    {
        QPointer<IDOSProject> project(view->project());
        QString providerPrefix = action->objectName() == QStringLiteral("importWellHeaderAction")
            ? QStringLiteral("idos.well.header") : QStringLiteral("idos.well.path");
        if (action->objectName() == QStringLiteral("importWellCurvesAction"))
        {
            providerPrefix = QStringLiteral("idos.well.las");
        }
        else if (action->objectName() == QStringLiteral("importWellPartAction"))
        {
            const QString part = action->property("part").toString();
            if (part == QStringLiteral("idos.well.zones"))
            {
                providerPrefix = QStringLiteral("idos.well.tops");
            }
            else if (part == QStringLiteral("idos.well.completions"))
            {
                providerPrefix = QStringLiteral("idos.well.completion");
            }
            else if (part == QStringLiteral("idos.well.checkshots"))
            {
                providerPrefix = QStringLiteral("idos.well.checkshot");
            }
        }
        const QString importTitle = providerPrefix == QStringLiteral("idos.well.path")
            ? tr("Import Well Trajectory")
            : providerPrefix == QStringLiteral("idos.well.header") ? tr("Import Well Header")
            : providerPrefix == QStringLiteral("idos.well.las") ? tr("Import Well Curves")
            : providerPrefix == QStringLiteral("idos.well.tops") ? tr("Import Well Markers")
            : providerPrefix == QStringLiteral("idos.well.completion") ? tr("Import Well Completions")
            : tr("Import Well Checkshots");
        const QString file = QFileDialog::getOpenFileName(
            view, importTitle, QString(),
            IDOSDataProviderRegistry::instance().fileFilters(providerPrefix));
        if (file.isEmpty() || project.isNull())
        {
            return;
        }
        QJsonObject arguments;
        arguments.insert(QStringLiteral("filePath"), file);
        arguments.insert(QStringLiteral("targetWellId"), id);
        IDOSImportWellDataCommandMetadata metadata;
        IDOSCommandManager* manager = project->commandManager();
        if (manager == nullptr || !manager->execute(metadata.create(arguments, project.data())))
        {
            if (manager != nullptr)
            {
                QMessageBox::warning(view, importTitle, manager->error());
            }
        }
    }
    else if (qobject_cast<IDOSWell*>(view->project()->object(id)) != nullptr)
    {
        IDOSWellTreeGuiProvider::importWellData(view->project(), id, view);
    }
}

void IDOSWellTreeGuiProvider::onDeleteWell()
{
    IDOSTreeView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    const QString id = action->property("dataId").toString();
    if (qobject_cast<IDOSWell*>(view->project()->object(id)) != nullptr)
    {
        IDOSWellTreeGuiProvider::deleteWell(view->project(), id, view);
    }
}


int IDOSWellTreeGuiProvider::importWellData(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull())
    {
        return 0;
    }
    const QStringList files = QFileDialog::getOpenFileNames(
        parent, tr("Import Well Data"), QString(),
        IDOSDataProviderRegistry::instance().fileFilters(QStringLiteral("idos.well.")));
    int imported = 0;
    QStringList errors;
    for (const QString& file : files)
    {
        if (target.isNull())
        {
            break;
        }
        QJsonObject arguments;
        arguments.insert(QStringLiteral("filePath"), file);
        IDOSImportWellDataCommandMetadata metadata;
        IDOSCommandManager* manager = target->commandManager();
        if (manager->execute(metadata.create(arguments, target.data())))
        {
            ++imported;
        }
        else
        {
            errors.append(manager->error());
        }
    }
    if (!errors.isEmpty())
    {
        QMessageBox::warning(parent, tr("Import Well Data"), errors.join(QStringLiteral("\n")));
    }
    return imported;
}

int IDOSWellTreeGuiProvider::importWellData(IDOSProject* project, const QString& targetWellId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || qobject_cast<IDOSWell*>(target->object(targetWellId)) == nullptr)
    {
        return 0;
    }
    const QString file = QFileDialog::getOpenFileName(
        parent, tr("Import Well Data"), QString(),
        IDOSDataProviderRegistry::instance().fileFilters(QStringLiteral("idos.well.")));
    if (file.isEmpty() || target.isNull())
    {
        return 0;
    }
    QJsonObject arguments;
    arguments.insert(QStringLiteral("filePath"), file);
    arguments.insert(QStringLiteral("targetWellId"), targetWellId);
    IDOSImportWellDataCommandMetadata metadata;
    IDOSCommandManager* manager = target->commandManager();
    if (!manager->execute(metadata.create(arguments, target.data())))
    {
        QMessageBox::warning(parent, tr("Import Well Data"), manager->error());
        return 0;
    }
    return 1;
}
bool IDOSWellTreeGuiProvider::newWell(IDOSProject* project, QWidget* parent)
{
    QPointer<IDOSProject> targetProject(project);
    if (targetProject.isNull())
    {
        return false;
    }
    QInputDialog dialog(parent);
    dialog.setObjectName(QStringLiteral("newWellDialog"));
    dialog.setWindowTitle(tr("New Well"));
    dialog.setWindowIcon(QIcon(QStringLiteral(":/images/gui-well-new.svg")));
    dialog.setLabelText(tr("Well name:"));
    dialog.setInputMode(QInputDialog::TextInput);
    dialog.setOkButtonText(tr("Create"));
    dialog.setCancelButtonText(tr("Cancel"));
    while (dialog.exec() == QDialog::Accepted)
    {
        if (targetProject.isNull())
        {
            return false;
        }
        const QString name = dialog.textValue().trimmed();
        if (name.isEmpty())
        {
            dialog.setLabelText(tr("Please enter a well name."));
            continue;
        }
        bool exists = false;
        for (IDOSDataObject* object : targetProject->objects())
        {
            IDOSWell* existing = qobject_cast<IDOSWell*>(object);
            if (existing != nullptr && existing->name().compare(name, Qt::CaseInsensitive) == 0)
            {
                exists = true;
                break;
            }
        }
        if (exists)
        {
            dialog.setLabelText(tr("A well named \"%1\" already exists. Please enter a different name.").arg(name));
            continue;
        }
        IDOSCommandManager* commandManager = targetProject->commandManager();
        if (commandManager == nullptr)
        {
            return false;
        }

        return commandManager->execute(new IDOSCreateWellCommand(targetProject, name));
    }
    return false;
}

bool IDOSWellTreeGuiProvider::deleteWell(IDOSProject* project, const QString& wellId, QWidget* parent)
{
    QPointer<IDOSProject> target(project);
    if (target.isNull() || wellId.isEmpty())
    {
        return false;
    }
    IDOSDataObject* obj = target->object(wellId);
    IDOSWell* well = qobject_cast<IDOSWell*>(obj);
    if (well == nullptr)
    {
        return false;
    }
    const QMessageBox::StandardButton btn =
        QMessageBox::question(parent, tr("Delete Well"),
                               tr("Are you sure you want to delete well \"%1\"?\n"
                                  "It will be removed from all cases that reference it.")
                                   .arg(well->name()),
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

    return commandManager->execute(new IDOSDeleteWellCommand(target, wellId));
}
