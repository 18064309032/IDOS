#include "idostreepartkey.h"

IDOSTreePartKey::IDOSTreePartKey()
{
}

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
