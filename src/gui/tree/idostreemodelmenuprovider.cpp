#include "idostreemodelmenuprovider.h"
#include "idostreemodel.h"
#include "idostreeitemprovider.h"
#include "data/idosdataobject.h"
#include <QTreeView>
#include <QMenu>
#include <QAction>

IDOSTreeModelMenuProvider::IDOSTreeModelMenuProvider(QObject* parent)
    : IDOSTreeMenuProvider(parent)
    , m_model(nullptr)
    , m_view(nullptr)
{
}

IDOSTreeModelMenuProvider::~IDOSTreeModelMenuProvider() = default;

void IDOSTreeModelMenuProvider::setModel(IDOSTreeModel* model)
{
    m_model = model;
}

void IDOSTreeModelMenuProvider::setTreeView(QTreeView* view)
{
    m_view = view;
}

QMenu* IDOSTreeModelMenuProvider::createContextMenu()
{
    if (m_model == nullptr || m_view == nullptr) return nullptr;

    auto indices = m_view->selectionModel()->selectedIndexes();
    if (indices.isEmpty()) return nullptr;

    auto* menu = new QMenu(nullptr);

    IDOSDataObject* obj = m_model->objectFromIndex(indices.first());
    if (obj == nullptr) return menu;

    // 可见性切换
    auto* visibilityAction = new QAction(menu);
    visibilityAction->setText(obj->isVisible() ? tr("Hide") : tr("Show"));
    visibilityAction->connect(visibilityAction, &QAction::triggered, obj, [obj]() {
        obj->setVisible(!obj->isVisible());
    });
    menu->addAction(visibilityAction);

    menu->addSeparator();

    // 类型特定操作由按 typeId 注册的 provider 提供
    if (obj != nullptr)
    {
        if (auto* provider = m_model->providerFor(obj->typeId()))
        {
            provider->addMenuActions(menu, obj);
        }
    }

    menu->addSeparator();

    // 删除（暂时禁用；后续需引用检查：被模型引用的对象先断引用）
    auto* deleteAction = new QAction(tr("Delete"), menu);
    deleteAction->setEnabled(false);
    deleteAction->setStatusTip(tr("Delete is not available yet"));
    menu->addAction(deleteAction);

    return menu;
}
