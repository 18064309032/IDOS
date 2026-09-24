#include "case/providers/idossimulationcasetreeprovider.h"
#include "case/idoscaseobject.h"
#include "tree/idostreebuilder.h"

#include <QObject>

QString IDOSSimulationCaseTreeProvider::providerId() const
{
    return QStringLiteral("idos.case.simulation");
}

QString IDOSSimulationCaseTreeProvider::caseTypeId() const
{
    return QStringLiteral("idos.case.simulation");
}

void IDOSSimulationCaseTreeProvider::buildCaseTree(IDOSTreeBuilder& builder,
                                                   const IDOSCaseObject* caseObject)
{
    if (caseObject == nullptr)
    {
        return;
    }

    auto* root = builder.addObject(caseObject);
    auto childBuilder = builder.childBuilder(root);
    childBuilder.addGroup(QStringLiteral("case.inputs"), QObject::tr("Input Data"));
    childBuilder.addGroup(QStringLiteral("case.parameters"), QObject::tr("Parameters"));
    childBuilder.addGroup(QStringLiteral("case.results"), QObject::tr("Results"));
    childBuilder.addGroup(QStringLiteral("case.views"), QObject::tr("Views"));
}
