#include "tree/idostreepartkey.h"

IDOSTreePartKey::IDOSTreePartKey() = default;

IDOSTreePartKey::IDOSTreePartKey(const QString& key)
    : m_key(key)
{
}

QString IDOSTreePartKey::toString() const
{
    return m_key;
}

bool IDOSTreePartKey::isEmpty() const
{
    return m_key.isEmpty();
}
