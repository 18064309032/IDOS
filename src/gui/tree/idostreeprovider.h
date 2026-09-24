#ifndef IDOS_TREE_PROVIDER_H
#define IDOS_TREE_PROVIDER_H

#include "idos_gui.h"

#include <QString>

/**
 * @brief 树 Provider 基类。
 */
class GUI_EXPORT IDOSTreeProvider
{
public:
    virtual ~IDOSTreeProvider();

    virtual QString providerId() const = 0;
};

#endif // IDOS_TREE_PROVIDER_H
