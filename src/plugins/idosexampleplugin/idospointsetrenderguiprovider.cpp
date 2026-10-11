#include <QAction>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "command/idoscommandmanager.h"
#include "idosdataobject.h"
#include "idospointsetrenderprovider.h"
#include "idosproject.h"
#include "idosrenderobject.h"
#include "idosrenderscene.h"
#include "idosrenderview.h"

#include "idospointsetrenderguiprovider.h"

IDOSPointSetRenderGuiProvider::~IDOSPointSetRenderGuiProvider()
{
    for (const QPointer<QAction>& action : m_actions)
    {
        delete action.data();
    }
}

QString IDOSPointSetRenderGuiProvider::name() const
{
    return QString::fromLatin1("idos.example.pointset");
}

bool IDOSPointSetRenderGuiProvider::supports(const IDOSRenderObject* object) const
{
    IDOSDataObject* data = object != nullptr && object->provider() != nullptr
        ? object->provider()->dataSource() : nullptr;
    return data != nullptr && data->typeId() == name();
}

void IDOSPointSetRenderGuiProvider::populateMenu(QMenu* menu, IDOSRenderView* view)
{
    IDOSRenderObject* object = view->menuObject();
    if (!supports(object) || view->project() == nullptr)
    {
        return;
    }
    QAction* size = menu->addAction(tr("Increase Point Size"));
    m_actions.append(QPointer<QAction>(size));
    size->setObjectName(QString::fromLatin1("increasePointSizeAction"));
    size->setData(object->id());
    size->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    connect(size, &QAction::triggered, this, &IDOSPointSetRenderGuiProvider::onIncreasePointSize);
    QAction* remove = menu->addAction(tr("Delete Point Set"));
    m_actions.append(QPointer<QAction>(remove));
    remove->setObjectName(QString::fromLatin1("deletePointSetAction"));
    remove->setData(object->id());
    remove->setProperty("project", QVariant::fromValue<QObject*>(view->project()));
    connect(remove, &QAction::triggered, this, &IDOSPointSetRenderGuiProvider::onDeletePointSet);
}

IDOSRenderView* IDOSPointSetRenderGuiProvider::actionView() const
{
    QAction* action = qobject_cast<QAction*>(sender());
    QMenu* menu = action != nullptr ? qobject_cast<QMenu*>(action->parent()) : nullptr;
    IDOSRenderView* view = menu != nullptr ? qobject_cast<IDOSRenderView*>(menu->parentWidget()) : nullptr;
    if (view == nullptr || view->project() == nullptr ||
        action->property("project").value<QObject*>() != view->project())
    {
        return nullptr;
    }
    return view;
}

void IDOSPointSetRenderGuiProvider::onIncreasePointSize()
{
    IDOSRenderView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSRenderObject* object = view->scene()->object(action->data().toString());
    IDOSPointSetRenderProvider* provider = object != nullptr
        ? dynamic_cast<IDOSPointSetRenderProvider*>(object->provider()) : nullptr;
    if (provider != nullptr)
    {
        provider->setPointSize(provider->pointSize() + 2.0);
        view->scene()->notifyChanged(object->id());
    }
}

void IDOSPointSetRenderGuiProvider::onDeletePointSet()
{
    IDOSRenderView* view = actionView();
    if (view == nullptr)
    {
        return;
    }
    QAction* action = qobject_cast<QAction*>(sender());
    IDOSRenderObject* object = view->scene()->object(action->data().toString());
    if (!supports(object))
    {
        return;
    }
    QJsonObject arguments;
    arguments.insert(QString::fromLatin1("objectId"), object->provider()->dataSource()->objectId());
    view->project()->commandManager()->execute(QString::fromLatin1("pointset.delete"), arguments);
    if (!view->project()->commandManager()->error().isEmpty())
    {
        QMessageBox::warning(view, tr("Delete Point Set"), view->project()->commandManager()->error());
    }
}

