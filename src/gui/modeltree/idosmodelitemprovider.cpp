#include "idosmodelitemprovider.h"
#include "tree/idostreemodel.h"
#include "tree/idosobjecttreenode.h"
#include "tree/idostreenode.h"
#include "data/idosdataobject.h"
#include "data/model/idosmodel.h"
#include "idosproject.h"
#include <QObject>

QString IDOSModelItemProvider::memberSignature(IDOSProject* project,
                                               const IDOSModel* modelObj)
{
    QStringList parts;
    const auto appendPart = [&parts, project](const QString& refId) {
        const auto* refObj = project->objectById(refId);
        parts << refId + QStringLiteral("|")
            + (refObj != nullptr ? refObj->name() : QObject::tr("<missing>"));
    };

    if (!modelObj->gridId().isEmpty()) appendPart(modelObj->gridId());
    for (const QString& wellId : modelObj->wellIds()) appendPart(wellId);
    return parts.join(QChar(';'));
}

QString IDOSModelItemProvider::typeId() const
{
    return QStringLiteral("idos.model");
}

void IDOSModelItemProvider::buildChildren(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                                          IDOSDataObject* obj)
{
    auto* modelObj = qobject_cast<IDOSModel*>(obj);
    if (modelObj == nullptr || model == nullptr || parentNode == nullptr) return;

    IDOSProject* project = model->project();
    if (project == nullptr) return;

    // 网格分组 + 引用影子节点
    auto* gridGroup = new IDOSObjectTreeNode(parentNode);
    gridGroup->setName(QObject::tr("Grid"));
    parentNode->appendChild(gridGroup);

    if (!modelObj->gridId().isEmpty())
    {
        const auto* gridObj = project->objectById(modelObj->gridId());
        auto* shadow = new IDOSObjectTreeNode(gridGroup);
        shadow->setName(gridObj != nullptr ? gridObj->name()
                                           : QObject::tr("<missing>"));
        shadow->setTag(modelObj->gridId());   // 引用 objectId（跳转/导航用）
        gridGroup->appendChild(shadow);
    }

    // 井分组 + 引用影子节点列表
    auto* wellGroup = new IDOSObjectTreeNode(parentNode);
    wellGroup->setName(QObject::tr("Wells"));
    parentNode->appendChild(wellGroup);

    for (const QString& wellId : modelObj->wellIds())
    {
        const auto* wellObj = project->objectById(wellId);
        auto* shadow = new IDOSObjectTreeNode(wellGroup);
        shadow->setName(wellObj != nullptr ? wellObj->name()
                                           : QObject::tr("<missing>"));
        shadow->setTag(wellId);
        wellGroup->appendChild(shadow);
    }

    parentNode->setTag(memberSignature(project, modelObj));
}

bool IDOSModelItemProvider::needsRebuild(IDOSTreeModel* model, IDOSTreeNode* parentNode,
                                         IDOSDataObject* obj)
{
    auto* modelObj = qobject_cast<IDOSModel*>(obj);
    if (modelObj == nullptr || model == nullptr || parentNode == nullptr) return false;
    if (model->project() == nullptr) return false;

    const QString expected = memberSignature(model->project(), modelObj);
    return parentNode->tag() != expected;
}
