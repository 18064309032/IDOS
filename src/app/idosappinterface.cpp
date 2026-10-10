#include <limits>

#include <SARibbonBar.h>
#include <SARibbonCategory.h>
#include <SARibbonMainWindow.h>

#include "idosmainwindow.h"
#include "log/idoslogger.h"

#include "idosappinterface.h"

IDOSAppInterface::IDOSAppInterface(IDOSMainWindow* mainWindow)
    : IDOSInterface(mainWindow)
    , m_mainWindow(mainWindow)
{
}

QMainWindow* IDOSAppInterface::mainWindow() const
{
    return m_mainWindow;
}

SARibbonCategory* IDOSAppInterface::ribbonCategory(const QString& objectName) const
{
    if (m_mainWindow == nullptr || objectName.isEmpty())
    {
        IDOS_WARN(tr("Ribbon category lookup rejected: mainWindow=%1, objectName=%2")
                      .arg(m_mainWindow == nullptr ? tr("null") : tr("valid"),
                           objectName));
        return nullptr;
    }

    SARibbonMainWindow* ribbonMainWindow = qobject_cast<SARibbonMainWindow*>(m_mainWindow);
    if (ribbonMainWindow == nullptr || ribbonMainWindow->ribbonBar() == nullptr)
    {
        IDOS_ERROR(tr("Ribbon category lookup failed: ribbon main window or ribbon bar is null."));
        return nullptr;
    }

    SARibbonCategory* category = ribbonMainWindow->ribbonBar()->categoryByObjectName(objectName);
    IDOS_DEBUG(tr("Ribbon category lookup: objectName=%1, result=0x%2")
                   .arg(objectName,
                        QString::number(reinterpret_cast<quintptr>(category), 16)));
    return category;
}

SARibbonCategory* IDOSAppInterface::addRibbonCategory(const QString& objectName,
                                                       const QString& title)
{
    if (m_mainWindow == nullptr || objectName.isEmpty() || title.isEmpty())
    {
        IDOS_WARN(tr("Ribbon category creation rejected: objectName=%1, title=%2")
                      .arg(objectName, title));
        return nullptr;
    }

    SARibbonMainWindow* ribbonMainWindow = qobject_cast<SARibbonMainWindow*>(m_mainWindow);
    if (ribbonMainWindow == nullptr || ribbonMainWindow->ribbonBar() == nullptr)
    {
        IDOS_ERROR(tr("Ribbon category creation failed: ribbon main window or ribbon bar is null."));
        return nullptr;
    }

    SARibbonBar* ribbonBar = ribbonMainWindow->ribbonBar();
    SARibbonCategory* existingCategory = ribbonBar->categoryByObjectName(objectName);
    if (existingCategory != nullptr)
    {
        IDOS_WARN(tr("Ribbon category creation rejected: duplicate objectName=%1, existing=0x%2")
                      .arg(objectName,
                           QString::number(reinterpret_cast<quintptr>(existingCategory), 16)));
        return nullptr;
    }

    SARibbonCategory* category = ribbonBar->addCategoryPage(title);
    if (category == nullptr)
    {
        IDOS_ERROR(tr("Ribbon category creation failed: objectName=%1, title=%2")
                       .arg(objectName, title));
        return nullptr;
    }

    category->setObjectName(objectName);
    category->setProperty("idosRibbonIndex", std::numeric_limits<int>::max());
    m_ownedRibbonCategories.append(QPointer<SARibbonCategory>(category));
    reorderRibbonCategories();
    IDOS_DEBUG(tr("Ribbon category created: objectName=%1, title=%2, category=0x%3, parent=0x%4")
                   .arg(objectName,
                        title,
                        QString::number(reinterpret_cast<quintptr>(category), 16),
                        QString::number(reinterpret_cast<quintptr>(category->parent()), 16)));
    return category;
}

SARibbonCategory* IDOSAppInterface::insertCategoryPage(const QString& objectName,
                                                        const QString& title,
                                                        int index)
{
    if (m_mainWindow == nullptr || objectName.isEmpty() || title.isEmpty() || index < 0)
    {
        IDOS_WARN(tr("Ribbon category creation rejected: objectName=%1, title=%2")
                      .arg(objectName, title));
        return nullptr;
    }

    SARibbonMainWindow* ribbonMainWindow = qobject_cast<SARibbonMainWindow*>(m_mainWindow);
    if (ribbonMainWindow == nullptr || ribbonMainWindow->ribbonBar() == nullptr)
    {
        IDOS_ERROR(tr("Ribbon category creation failed: ribbon main window or ribbon bar is null."));
        return nullptr;
    }

    SARibbonBar* ribbonBar = ribbonMainWindow->ribbonBar();
    SARibbonCategory* existingCategory = ribbonBar->categoryByObjectName(objectName);
    if (existingCategory != nullptr)
    {
        IDOS_WARN(tr("Ribbon category creation rejected: duplicate objectName=%1, existing=0x%2")
                      .arg(objectName,
                           QString::number(reinterpret_cast<quintptr>(existingCategory), 16)));
        return nullptr;
    }

    SARibbonCategory* category = ribbonBar->insertCategoryPage(title, index);
    if (category == nullptr)
    {
        IDOS_ERROR(tr("Ribbon category creation failed: objectName=%1, title=%2")
                       .arg(objectName, title));
        return nullptr;
    }

    category->setObjectName(objectName);
    category->setProperty("idosRibbonIndex", index);
    m_ownedRibbonCategories.append(QPointer<SARibbonCategory>(category));
    reorderRibbonCategories();
    IDOS_DEBUG(tr("Ribbon category created: objectName=%1, title=%2, category=0x%3, parent=0x%4")
                   .arg(objectName,
                        title,
                        QString::number(reinterpret_cast<quintptr>(category), 16),
                        QString::number(reinterpret_cast<quintptr>(category->parent()), 16)));
    return category;
}

void IDOSAppInterface::reorderRibbonCategories()
{
    SARibbonMainWindow* ribbonMainWindow = qobject_cast<SARibbonMainWindow*>(m_mainWindow);
    if (ribbonMainWindow == nullptr || ribbonMainWindow->ribbonBar() == nullptr)
    {
        return;
    }

    SARibbonBar* ribbonBar = ribbonMainWindow->ribbonBar();
    for (int index = 0; index < m_ownedRibbonCategories.size(); ++index)
    {
        int lowestIndex = index;
        for (int candidateIndex = index + 1;
             candidateIndex < m_ownedRibbonCategories.size();
             ++candidateIndex)
        {
            SARibbonCategory* candidate = m_ownedRibbonCategories.at(candidateIndex).data();
            SARibbonCategory* current = m_ownedRibbonCategories.at(lowestIndex).data();
            if (candidate != nullptr && current != nullptr
                && candidate->property("idosRibbonIndex").toInt()
                       < current->property("idosRibbonIndex").toInt())
            {
                lowestIndex = candidateIndex;
            }
        }

        if (lowestIndex != index)
        {
            m_ownedRibbonCategories.swapItemsAt(index, lowestIndex);
        }

        SARibbonCategory* category = m_ownedRibbonCategories.at(index).data();
        if (category != nullptr)
        {
            const int currentIndex = ribbonBar->categoryIndex(category);
            const int targetIndex = index + 1;
            if (currentIndex >= 0 && currentIndex != targetIndex)
            {
                ribbonBar->moveCategory(currentIndex, targetIndex);
            }
        }
    }
}

void IDOSAppInterface::removeRibbonCategory(SARibbonCategory* category)
{
    if (m_mainWindow == nullptr || category == nullptr)
    {
        IDOS_WARN(tr("Ribbon category removal rejected: mainWindow=%1, category=0x%2")
                      .arg(m_mainWindow == nullptr ? tr("null") : tr("valid"),
                           QString::number(reinterpret_cast<quintptr>(category), 16)));
        return;
    }

    int ownedCategoryIndex = -1;
    for (int index = 0; index < m_ownedRibbonCategories.size(); ++index)
    {
        if (m_ownedRibbonCategories.at(index).data() == category)
        {
            ownedCategoryIndex = index;
            break;
        }
    }
    if (ownedCategoryIndex < 0)
    {
        IDOS_WARN(tr("Ribbon category removal rejected: category is not owned by this interface, "
                     "objectName=%1, category=0x%2, parent=0x%3")
                      .arg(category->objectName(),
                           QString::number(reinterpret_cast<quintptr>(category), 16),
                           QString::number(reinterpret_cast<quintptr>(category->parent()), 16)));
        return;
    }

    const QString objectName = category->objectName();
    const QString categoryAddress = QString::number(reinterpret_cast<quintptr>(category), 16);
    const QString parentAddress = QString::number(reinterpret_cast<quintptr>(category->parent()), 16);
    m_ownedRibbonCategories.removeAt(ownedCategoryIndex);

    SARibbonMainWindow* ribbonMainWindow = qobject_cast<SARibbonMainWindow*>(m_mainWindow);
    if (ribbonMainWindow != nullptr && ribbonMainWindow->ribbonBar() != nullptr)
    {
        IDOS_DEBUG(tr("Detaching ribbon category: objectName=%1, category=0x%2, parent=0x%3, "
                      "stackedWidget=0x%4")
                       .arg(objectName,
                            categoryAddress,
                            parentAddress,
                            QString::number(reinterpret_cast<quintptr>(
                                                ribbonMainWindow->ribbonBar()->ribbonStackedWidget()),
                                            16)));
        ribbonMainWindow->ribbonBar()->removeCategory(category);
    }
    else
    {
        IDOS_ERROR(tr("Ribbon category is owned but its ribbon bar is unavailable: "
                      "objectName=%1, category=0x%2")
                       .arg(objectName, categoryAddress));
    }

    category->hide();
    IDOS_DEBUG(tr("Deleting detached ribbon category before plugin unload: objectName=%1, "
                  "category=0x%2, parent=0x%3")
                   .arg(objectName, categoryAddress, parentAddress));
    delete category;
    IDOS_DEBUG(tr("Detached ribbon category deleted: objectName=%1, category=0x%2")
                   .arg(objectName, categoryAddress));
}
