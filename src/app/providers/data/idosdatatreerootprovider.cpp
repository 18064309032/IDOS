#include <QIcon>
#include <QObject>

#include "providers/data/idosimporteddatatreeprovider.h"
#include "providers/fluidrock/idosfluidrocktabletreeprovider.h"
#include "tree/idostreeitem.h"

#include "idosdatatreerootprovider.h"

IDOSDataTreeRootProvider::IDOSDataTreeRootProvider()
    :
    QObject(nullptr)
    , IDOSTreeItemProvider()
{
}

void IDOSDataTreeRootProvider::refresh()
{
    IDOSTreeItem* root = item();
    if (root == nullptr)
    {
        return;
    }
    root->clearChildren();
    IDOSTreeItem* wellGroup = nullptr;
    IDOSTreeItem* group0 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group0->provider()->setKey(QStringLiteral("data.wellGroup"));
    group0->provider()->setName(tr("Well Group"));
    group0->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-well-group.svg")));
    group0->provider()->setGroup(true);
    wellGroup = group0;
    IDOSTreeItem* group1 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group1->provider()->setKey(QStringLiteral("data.globalLogs"));
    group1->provider()->setName(tr("Global Well Logs"));
    group1->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-global-logs.svg")));
    group1->provider()->setGroup(true);
    wellGroup->appendChild(group1);
    IDOSTreeItem* group2 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group2->provider()->setKey(QStringLiteral("data.globalCompletions"));
    group2->provider()->setName(tr("Global Completions"));
    group2->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-global-completions.svg")));
    group2->provider()->setGroup(true);
    wellGroup->appendChild(group2);
    IDOSTreeItem* group3 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group3->provider()->setKey(QStringLiteral("data.globalObservations"));
    group3->provider()->setName(tr("Global Observations"));
    group3->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-global-observations.svg")));
    group3->provider()->setGroup(true);
    wellGroup->appendChild(group3);
    IDOSTreeItem* group4 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group4->provider()->setKey(QStringLiteral("data.wellProperties"));
    group4->provider()->setName(tr("Well Properties"));
    group4->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-well-properties.svg")));
    group4->provider()->setGroup(true);
    wellGroup->appendChild(group4);
    IDOSTreeItem* group5 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group5->provider()->setKey(QStringLiteral("data.wells"));
    group5->provider()->setName(tr("Wells"));
    group5->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-wells.svg")));
    group5->provider()->setGroup(true);
    wellGroup->appendChild(group5);
    root->appendChild(wellGroup);
    IDOSTreeItem* group6 = new IDOSTreeItem(new IDOSImportedDataTreeProvider(nullptr));
    group6->provider()->setKey(QStringLiteral("data.seismic"));
    group6->provider()->setName(tr("Seismic"));
    group6->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-seismic.svg")));
    group6->provider()->setGroup(true);
    root->appendChild(group6);
    IDOSTreeItem* group7 = new IDOSTreeItem(new IDOSImportedDataTreeProvider(nullptr));
    group7->provider()->setKey(QStringLiteral("data.surfaceProperties"));
    group7->provider()->setName(tr("Surface Properties"));
    group7->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-surface-properties.svg")));
    group7->provider()->setGroup(true);
    root->appendChild(group7);
    IDOSTreeItem* group8 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group8->provider()->setKey(QStringLiteral("data.faults"));
    group8->provider()->setName(tr("Faults"));
    group8->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-faults.svg")));
    group8->provider()->setGroup(true);
    root->appendChild(group8);
    IDOSTreeItem* group9 = new IDOSTreeItem(new IDOSImportedDataTreeProvider(nullptr));
    group9->provider()->setKey(QStringLiteral("data.boundaries"));
    group9->provider()->setName(tr("Polygons / Boundaries"));
    group9->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-boundaries.svg")));
    group9->provider()->setGroup(true);
    root->appendChild(group9);
    IDOSTreeItem* group10 = new IDOSTreeItem(new IDOSImportedDataTreeProvider(nullptr));
    group10->provider()->setKey(QStringLiteral("data.naturalFractures"));
    group10->provider()->setName(tr("Natural Fractures"));
    group10->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-natural-fractures.svg")));
    group10->provider()->setGroup(true);
    root->appendChild(group10);
    IDOSTreeItem* group11 = new IDOSTreeItem(new IDOSImportedDataTreeProvider(nullptr));
    group11->provider()->setKey(QStringLiteral("data.hydraulicFractures"));
    group11->provider()->setName(tr("Hydraulic Fracturing"));
    group11->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-hydraulic-fractures.svg")));
    group11->provider()->setGroup(true);
    root->appendChild(group11);
    IDOSTreeItem* group12 = new IDOSTreeItem(new IDOSFluidRockTableTreeProvider(nullptr));
    group12->provider()->setKey(QStringLiteral("data.fluids"));
    group12->provider()->setName(tr("Fluids"));
    group12->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-fluids.svg")));
    group12->provider()->setGroup(true);
    root->appendChild(group12);
    IDOSTreeItem* group13 = new IDOSTreeItem(new IDOSFluidRockTableTreeProvider(nullptr));
    group13->provider()->setKey(QStringLiteral("data.rockPhysics"));
    group13->provider()->setName(tr("Rock Physics Functions"));
    group13->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-rock-physics.svg")));
    group13->provider()->setGroup(true);
    root->appendChild(group13);
    IDOSTreeItem* group14 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group14->provider()->setKey(QStringLiteral("data.initialConditions"));
    group14->provider()->setName(tr("Initial Conditions"));
    group14->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-initial-conditions.svg")));
    group14->provider()->setGroup(true);
    root->appendChild(group14);
    IDOSTreeItem* group15 = new IDOSTreeItem(new IDOSImportedDataTreeProvider(nullptr));
    group15->provider()->setKey(QStringLiteral("data.microseismic"));
    group15->provider()->setName(tr("Microseismic"));
    group15->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-microseismic.svg")));
    group15->provider()->setGroup(true);
    root->appendChild(group15);
    IDOSTreeItem* group16 = new IDOSTreeItem(new IDOSImportedDataTreeProvider(nullptr));
    group16->provider()->setKey(QStringLiteral("data.production"));
    group16->provider()->setName(tr("Production Summary"));
    group16->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-production.svg")));
    group16->provider()->setGroup(true);
    root->appendChild(group16);
    IDOSTreeItem* group17 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group17->provider()->setKey(QStringLiteral("data.streamlines"));
    group17->provider()->setName(tr("Flow Diagnostics"));
    group17->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-flow-diagnostics.svg")));
    group17->provider()->setGroup(true);
    root->appendChild(group17);
    IDOSTreeItem* group18 = new IDOSTreeItem(new IDOSTreeItemProvider());
    group18->provider()->setKey(QStringLiteral("data.management"));
    group18->provider()->setName(tr("Field Management Strategies"));
    group18->provider()->setIcon(QIcon(QStringLiteral(":/images/gui-field-management.svg")));
    group18->provider()->setGroup(true);
    root->appendChild(group18);

}
