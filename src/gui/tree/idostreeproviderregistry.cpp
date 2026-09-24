#include "tree/idostreeproviderregistry.h"
#include "tree/idoscasetreeprovider.h"
#include "tree/idosdatatreeprovider.h"

#include <QtAlgorithms>

IDOSTreeProviderRegistry::IDOSTreeProviderRegistry() = default;

IDOSTreeProviderRegistry::~IDOSTreeProviderRegistry()
{
    qDeleteAll(m_dataProviders);
    qDeleteAll(m_caseProviders);
}

void IDOSTreeProviderRegistry::registerDataProvider(IDOSDataTreeProvider* provider)
{
    if (provider == nullptr) return;

    m_dataProviders.insert(provider->typeId(), provider);
}

void IDOSTreeProviderRegistry::registerCaseProvider(IDOSCaseTreeProvider* provider)
{
    if (provider == nullptr) return;

    m_caseProviders.insert(provider->caseTypeId(), provider);
}

IDOSDataTreeProvider* IDOSTreeProviderRegistry::dataProviderFor(const QString& typeId) const
{
    return m_dataProviders.value(typeId, nullptr);
}

IDOSCaseTreeProvider* IDOSTreeProviderRegistry::caseProviderFor(const QString& caseTypeId) const
{
    return m_caseProviders.value(caseTypeId, nullptr);
}
