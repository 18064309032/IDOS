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
    if (item == nullptr || item->key() == QStringLiteral("data.wells"))
    {
        return true;
    }
    return item->tree() != nullptr && item->tree()->project() != nullptr &&
           qobject_cast<IDOSWell*>(item->tree()->project()->object(item->dataId())) != nullptr;
}

void IDOSWellTreeGuiProvider::populateMenu(QMenu* menu, IDOSTreeView* view)
{
    if (view->project() == nullptr)
    {
        return;
    }
    IDOSTreeItem* item = view->menuItem();

    if (item == nullptr || item->key() == QStringLiteral("data.wells"))
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
    QAction* import = addAction(menu, view, tr("Import Data..."));
    import->setObjectName(QStringLiteral("importWellDataSingleAction"));
    connect(import, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onImportWell);
    QAction* remove = addAction(menu, view, tr("Delete Well..."));
    remove->setObjectName(QStringLiteral("deleteWellAction"));
    connect(remove, &QAction::triggered, this, &IDOSWellTreeGuiProvider::onDeleteWell);
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
        IDOSDataProviderRegistry::instance().fileFilters());
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
        IDOSDataProviderRegistry::instance().fileFilters());
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