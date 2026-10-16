#include "tree/idostreeitem.h"

#include "idoscasetreerootprovider.h"

IDOSCaseTreeRootProvider::IDOSCaseTreeRootProvider()
    :
    IDOSTreeItemProvider()
{
}

void IDOSCaseTreeRootProvider::refresh()
{
    IDOSTreeItem* root = item();
    if (root == nullptr)
    {
        return;
    }
    root->clearChildren();
}
