#ifndef IDOS_TREE_PROVIDER_REGISTRY_H
#define IDOS_TREE_PROVIDER_REGISTRY_H

#include "idos_gui.h"

#include <QHash>
#include <QString>

class IDOSCaseTreeProvider;
class IDOSDataTreeProvider;

/**
 * @brief 通用树 Provider 注册表。
 */
class GUI_EXPORT IDOSTreeProviderRegistry
{
public:
    IDOSTreeProviderRegistry();
    ~IDOSTreeProviderRegistry();

    void registerDataProvider(IDOSDataTreeProvider* provider);
    void registerCaseProvider(IDOSCaseTreeProvider* provider);

    IDOSDataTreeProvider* dataProviderFor(const QString& typeId) const;
    IDOSCaseTreeProvider* caseProviderFor(const QString& caseTypeId) const;

private:
    QHash<QString, IDOSDataTreeProvider*> m_dataProviders;
    QHash<QString, IDOSCaseTreeProvider*> m_caseProviders;
};

#endif // IDOS_TREE_PROVIDER_REGISTRY_H
