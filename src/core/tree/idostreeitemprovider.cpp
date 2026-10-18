#include <QUuid>

#include "idosdataobject.h"
#include "idostreeitem.h"
#include "idostreemodel.h"

#include "idostreeitemprovider.h"

IDOSTreeItemProvider::IDOSTreeItemProvider(IDOSDataObject* data)
    :
    QObject(nullptr)
    , m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_dataId(data != nullptr ? data->objectId() : QString())
    , m_group(false)
    , m_reference(false)
    , m_checkable(false)
    , m_checkEnabled(false)
    , m_checkState(Qt::Unchecked)
    , m_item(nullptr)
    , m_data(data)
{
}

IDOSTreeItemProvider::~IDOSTreeItemProvider()
{
}

IDOSTreeItem* IDOSTreeItemProvider::item() const
{
    return m_item;
}

IDOSDataObject* IDOSTreeItemProvider::object() const
{
    return m_data.data();
}

QString IDOSTreeItemProvider::id() const
{
    return m_id;
}

QString IDOSTreeItemProvider::key() const
{
    return m_key;
}

void IDOSTreeItemProvider::setKey(const QString& value)
{
    if (m_key == value)
    {
        return;
    }
    m_key = value;
}

QString IDOSTreeItemProvider::name() const
{
    return m_name;
}

void IDOSTreeItemProvider::setName(const QString& value)
{
    if (m_name == value)
    {
        return;
    }
    m_name = value;
}

QIcon IDOSTreeItemProvider::icon() const
{
    return m_icon;
}

void IDOSTreeItemProvider::setIcon(const QIcon& value)
{
    if (m_icon.cacheKey() == value.cacheKey())
    {
        return;
    }
    m_icon = value;
}

QString IDOSTreeItemProvider::dataId() const
{
    return m_dataId;
}

void IDOSTreeItemProvider::setDataId(const QString& value)
{
    if (m_dataId == value)
    {
        return;
    }
    m_dataId = value;
}

QString IDOSTreeItemProvider::caseId() const
{
    return m_caseId;
}

void IDOSTreeItemProvider::setCaseId(const QString& value)
{
    if (m_caseId == value)
    {
        return;
    }
    m_caseId = value;
}

QString IDOSTreeItemProvider::partKey() const
{
    return m_partKey;
}

void IDOSTreeItemProvider::setPartKey(const QString& value)
{
    if (m_partKey == value)
    {
        return;
    }
    m_partKey = value;
}

QString IDOSTreeItemProvider::itemKey() const
{
    return m_itemKey;
}

void IDOSTreeItemProvider::setItemKey(const QString& value)
{
    if (m_itemKey == value)
    {
        return;
    }
    m_itemKey = value;
}

bool IDOSTreeItemProvider::isGroup() const
{
    return m_group;
}

void IDOSTreeItemProvider::setGroup(bool value)
{
    if (m_group == value)
    {
        return;
    }
    m_group = value;
}

bool IDOSTreeItemProvider::isReference() const
{
    return m_reference;
}

void IDOSTreeItemProvider::setReference(bool value)
{
    if (m_reference == value)
    {
        return;
    }
    m_reference = value;
}

bool IDOSTreeItemProvider::isCheckable() const
{
    if (isGroup())
    {
        if (item() != nullptr)
        {
            for (IDOSTreeItem* child : item()->children())
            {
                if (child->provider()->isCheckable())
                {
                    return true;
                }
            }
        }
        return false;
    }
    return m_checkable && m_checkEnabled;
}

void IDOSTreeItemProvider::setCheckable(bool value)
{
    if (m_checkable == value)
    {
        return;
    }
    m_checkable = value;
}

void IDOSTreeItemProvider::setCheckEnabled(bool enabled)
{
    m_checkEnabled = enabled;
}

Qt::CheckState IDOSTreeItemProvider::checkState() const
{
    if (isGroup() && item() != nullptr)
    {
        bool checked = false;
        bool unchecked = false;
        for (IDOSTreeItem* child : item()->children())
        {
            if (child->provider()->isCheckable())
            {
                const Qt::CheckState state = child->provider()->checkState();
                checked = checked || state != Qt::Unchecked;
                unchecked = unchecked || state != Qt::Checked;
            }
        }
        return checked ? (unchecked ? Qt::PartiallyChecked : Qt::Checked) : Qt::Unchecked;
    }
    return m_checkState;
}

QString IDOSTreeItemProvider::displayName() const
{
    return name();
}

bool IDOSTreeItemProvider::isChecked() const
{
    return checkState() == Qt::Checked;
}

void IDOSTreeItemProvider::setChecked(Qt::CheckState state)
{
    if (isGroup() && item() != nullptr)
    {
        for (IDOSTreeItem* child : item()->children())
        {
            if (child->provider()->isCheckable())
            {
                child->provider()->setChecked(state);
            }
        }
    }
    else
    {
        m_checkState = state;
    }
}

QVariant IDOSTreeItemProvider::data(int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return displayName();
    }
    if (role == Qt::DecorationRole)
    {
        return icon();
    }
    if (role == Qt::CheckStateRole)
    {
        return isCheckable() ? QVariant::fromValue(checkState()) : QVariant();
    }
    if (role == Qt::ToolTipRole && !m_roles.contains(role))
    {
        return displayName();
    }
    return m_roles.value(role);
}

bool IDOSTreeItemProvider::setData(const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole)
    {
        bool valid = false;
        const int state = value.toInt(&valid);
        if (!valid || !isCheckable() ||
            (state != Qt::Unchecked && state != Qt::Checked))
        {
            return false;
        }
        const Qt::CheckState requestedState = static_cast<Qt::CheckState>(state);
        setChecked(requestedState);
        return checkState() == requestedState;
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (!value.canConvert<QString>())
        {
            return false;
        }
        m_name = value.toString();
        return true;
    }
    if (role == Qt::DecorationRole)
    {
        if (!value.canConvert<QIcon>())
        {
            return false;
        }
        m_icon = qvariant_cast<QIcon>(value);
        return true;
    }
    if (m_roles.value(role) != value)
    {
        m_roles.insert(role, value);
    }
    return true;
}

Qt::ItemFlags IDOSTreeItemProvider::flags() const
{
    Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    if (isCheckable())
    {
        result |= Qt::ItemIsUserCheckable;
    }
    return result;
}

QMenu* IDOSTreeItemProvider::createContextMenu(const QList<IDOSTreeItem*>& selectedItems)
{
    Q_UNUSED(selectedItems)
    return nullptr;
}

void IDOSTreeItemProvider::refresh()
{
}

bool IDOSTreeItemProvider::dependsOn(const QStringList& dataIds) const
{
    return !dataId().isEmpty() && dataIds.contains(dataId());
}

bool IDOSTreeItemProvider::setItem(IDOSTreeItem* item)
{
    if (item == nullptr || m_item != nullptr || item->provider() != this)
    {
        return false;
    }
    m_item = item;
    return true;
}

QString IDOSTreeItemProvider::metadataId() const
{
    return m_metadataId;
}

bool IDOSTreeItemProvider::setMetadataId(const QString& id)
{
    if (m_item != nullptr || !m_metadataId.isEmpty() || id.isEmpty())
    {
        return false;
    }
    m_metadataId = id;
    return true;
}
