#include "idosmodeltreemenuprovider.h"
#include "idosmodeltreemodel.h"
#include "data/idosdataobject.h"
#include "data/grid/idosgrid.h"
#include <QTreeView>
#include <QMenu>
#include <QAction>

IDOSModelTreeMenuProvider::IDOSModelTreeMenuProvider(QObject* parent)
    : IDOSTreeMenuProvider(parent)
    , m_model(nullptr)
    , m_view(nullptr)
{
}

IDOSModelTreeMenuProvider::~IDOSModelTreeMenuProvider() = default;

void IDOSModelTreeMenuProvider::setModel(IDOSModelTreeModel* model)
{
    m_model = model;
}

void IDOSModelTreeMenuProvider::setTreeView(QTreeView* view)
{
    m_view = view;
}

QMenu* IDOSModelTreeMenuProvider::createContextMenu()
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

    // 类型特定操作
    if (auto* grid = qobject_cast<IDOSGrid*>(obj))
    {
        Q_UNUSED(grid);
        // 网格特定操作暂时没有
    }

    menu->addSeparator();

    // 删除（暂时禁用）
    auto* deleteAction = new QAction(tr("Delete"), menu);
    deleteAction->setEnabled(false);
    deleteAction->setStatusTip(tr("Delete is not available yet"));
    menu->addAction(deleteAction);

    return menu;
}
