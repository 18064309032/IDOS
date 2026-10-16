#include <QMimeData>

#include "idostreeview.h"

#include "idostreemenuprovider.h"

IDOSTreeMenuProvider::IDOSTreeMenuProvider(IDOSTreeView* view)
    :
    QObject(view)
    , m_view(view)
{
}

IDOSTreeView* IDOSTreeMenuProvider::view() const
{
    return m_view.data();
}

IDOSTreeMenuProvider::~IDOSTreeMenuProvider()
{
}

QMimeData* IDOSTreeMenuProvider::createMimeData(const QList<IDOSTreeItem*>& items) const
{
    Q_UNUSED(items)
    return nullptr;
}

bool IDOSTreeMenuProvider::canCut(const QMimeData* data) const
{
    Q_UNUSED(data)
    return false;
}

bool IDOSTreeMenuProvider::canPaste(const QMimeData* data, const IDOSTreeItem* target,
                                    Qt::DropAction action) const
{
    Q_UNUSED(data)
    Q_UNUSED(target)
    Q_UNUSED(action)
    return false;
}

bool IDOSTreeMenuProvider::paste(const QMimeData* data, IDOSTreeItem* target,
                                 Qt::DropAction action)
{
    Q_UNUSED(data)
    Q_UNUSED(target)
    Q_UNUSED(action)
    return false;
}
