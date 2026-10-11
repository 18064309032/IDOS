#include "idosdataobject.h"
#include "idosproject.h"
#include "idostree.h"
#include "idostreeitem.h"

#include "idostreeguiprovider.h"

IDOSTreeGuiProvider::IDOSTreeGuiProvider(QObject* parent)
    :
    QObject(parent)
{
}

IDOSTreeGuiProvider::~IDOSTreeGuiProvider()
{
}

bool IDOSTreeGuiProvider::isCheckable(const IDOSTreeItem* item) const
{
    return item != nullptr && item->isCheckable();
}

bool IDOSTreeGuiProvider::isChecked(const IDOSTreeItem* item) const
{
    if (item != nullptr && item->tree() != nullptr && item->tree()->project() != nullptr)
    {
        IDOSDataObject* data = item->tree()->project()->object(item->dataId());
        if (data != nullptr)
        {
            return data->isVisible();
        }
    }
    return item != nullptr && item->isChecked();
}

bool IDOSTreeGuiProvider::setChecked(IDOSTreeItem* item, bool checked)
{
    if (item == nullptr || item->tree() == nullptr || item->tree()->project() == nullptr)
    {
        return false;
    }
    IDOSDataObject* data = item->tree()->project()->object(item->dataId());
    if (data == nullptr)
    {
        return false;
    }
    data->setVisible(checked);
    item->setChecked(checked);
    return true;
}
