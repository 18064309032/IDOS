#include "idossimulationcasetreeprovider.h"
#include "idoscaseobject.h"
#include "idossimulationcaseobject.h"
#include "idosproject.h"
#include "idosgrid.h"
#include "idosgridproperty.h"
#include "idostreebuilder.h"

#include <QObject>

QString IDOSSimulationCaseTreeProvider::providerId() const
{
    return QStringLiteral("idos.case.simulation");
}

QString IDOSSimulationCaseTreeProvider::caseTypeId() const
{
    return QStringLiteral("idos.case.simulation");
}

void IDOSSimulationCaseTreeProvider::buildCaseTree(IDOSTreeBuilder& builder, const IDOSCaseObject* caseObject)
{
    if (caseObject == nullptr)
    {
        return;
    }

    IDOSObjectTreeNode* root = builder.addObject(caseObject);
    if (root == nullptr)
    {
        return;
    }
    root->setIcon(QIcon(QStringLiteral(":/images/gui-case.svg")));
    IDOSTreeBuilder childBuilder = builder.childBuilder(root);
    childBuilder.addGroup(QStringLiteral("case.runSettings"), QObject::tr("Run Settings"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-case-parameters.svg")));
    IDOSTreeGroupNode* grids = childBuilder.addGroup(QStringLiteral("case.grids"), QObject::tr("Grids"));
    grids->setIcon(QIcon(QStringLiteral(":/images/gui-grid.svg")));
    IDOSTreeBuilder gridBuilder = childBuilder.childBuilder(grids);
    childBuilder.addGroup(QStringLiteral("case.fluidRock"), QObject::tr("Fluid and Rock Properties"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-fluids.svg")));
    childBuilder.addGroup(QStringLiteral("case.initialState"), QObject::tr("Initial State"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-case-inputs.svg")));
    IDOSTreeGroupNode* wells =
        childBuilder.addGroup(QStringLiteral("case.schedule"), QObject::tr("Wells and Development Plan"));
    wells->setIcon(QIcon(QStringLiteral(":/images/gui-well.svg")));
    IDOSTreeBuilder inputBuilder = childBuilder.childBuilder(wells);
    childBuilder.addGroup(QStringLiteral("case.outputSettings"), QObject::tr("Output Settings"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-case-parameters.svg")));
    childBuilder.addGroup(QStringLiteral("case.summaryResults"), QObject::tr("Summary Results"))
        ->setIcon(QIcon(QStringLiteral(":/images/gui-case-results.svg")));
    const IDOSProject* project = qobject_cast<const IDOSProject*>(caseObject->parent());
    for (const IDOSCaseItemRef& ref : caseObject->itemRefs())
    {
        const IDOSDataObject* object = project != nullptr ? project->objectById(ref.objectId()) : nullptr;
        if (ref.role() == QStringLiteral("case.grid"))
        {
            const IDOSGrid* grid = qobject_cast<const IDOSGrid*>(object);
            const QString gridTitle =
                grid != nullptr ? grid->name() : QObject::tr("Missing object: %1").arg(ref.objectId());
            IDOSTreeReferenceNode* gridNode = gridBuilder.addReference(ref, gridTitle);
            gridNode->setIcon(QIcon(QStringLiteral(":/images/gui-grid.svg")));
            gridNode->setCheckable(grid != nullptr);
            if (grid == nullptr)
            {
                continue;
            }
            IDOSTreeBuilder properties = gridBuilder.childBuilder(gridNode);
            IDOSTreeGroupNode* staticGroup =
                properties.addGroup(QStringLiteral("grid.staticProperties"), QObject::tr("Static Properties"));
            staticGroup->setIcon(QIcon(QStringLiteral(":/images/gui-grid-static-properties.svg")));
            IDOSTreeGroupNode* regions = properties.addGroup(QStringLiteral("grid.regions"), QObject::tr("Regions"));
            regions->setIcon(QIcon(QStringLiteral(":/images/gui-grid-static-properties.svg")));
            IDOSTreeGroupNode* dynamicGroup =
                properties.addGroup(QStringLiteral("grid.dynamicProperties"), QObject::tr("Dynamic Properties"));
            dynamicGroup->setIcon(QIcon(QStringLiteral(":/images/gui-grid-dynamic-properties.svg")));
            const QStringList regionKeywords = {QStringLiteral("SATNUM"), QStringLiteral("ROCKNUM"),
                                                QStringLiteral("PVTNUM"), QStringLiteral("EQLNUM"),
                                                QStringLiteral("FIPNUM")};
            for (const IDOSDataObject* candidate : project->objects())
            {
                const IDOSGridProperty* property = qobject_cast<const IDOSGridProperty*>(candidate);
                if (property == nullptr || property->gridId() != grid->objectId())
                {
                    continue;
                }
                IDOSTreeGroupNode* category = staticGroup;
                if (property->kind() == IDOSGridProperty::Kind::Dynamic)
                {
                    category = dynamicGroup;
                }
                else if (regionKeywords.contains(property->keyword().toUpper()))
                {
                    category = regions;
                }
                IDOSTreeBuilder propertyBuilder = properties.childBuilder(category);
                IDOSTreeReferenceNode* propertyNode =
                    propertyBuilder.addReference(IDOSCaseItemRef(QStringLiteral("case.gridProperty"),
                                                                  property->objectId()),
                                                 property->name());
                propertyNode->setIcon(QIcon(QStringLiteral(":/images/gui-grid-static-properties.svg")));
                propertyNode->setCheckable(true);
            }
            continue;
        }
        if (ref.role() != QStringLiteral("case.well"))
        {
            continue;
        }
        const QString title =
            object != nullptr ? object->name() : QObject::tr("Missing object: %1").arg(ref.objectId());
        inputBuilder.addReference(ref, title)->setIcon(QIcon(QStringLiteral(":/images/core-reference.svg")));
    }
    const IDOSSimulationCaseObject* simulationCase = qobject_cast<const IDOSSimulationCaseObject*>(caseObject);
    if (simulationCase != nullptr)
    {
        for (const QString& name : simulationCase->pendingWellNames())
        {
            inputBuilder.addGroup(QStringLiteral("case.unresolvedWell"), QObject::tr("%1 (unresolved)").arg(name))
                ->setIcon(QIcon(QStringLiteral(":/images/gui-well.svg")));
        }
    }
}
