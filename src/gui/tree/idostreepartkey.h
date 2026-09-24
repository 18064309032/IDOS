#ifndef IDOS_TREE_PART_KEY_H
#define IDOS_TREE_PART_KEY_H

#include "idos_gui.h"

#include <QString>

/**
 * @brief 树上对象子项的稳定语义键。
 *
 * 例如 well.logs、well.completions、grid.staticProperties。
 * 对外以常量形式使用，避免二次开发直接手写散乱字符串。
 */
class GUI_EXPORT IDOSTreePartKey
{
public:
    IDOSTreePartKey();
    explicit IDOSTreePartKey(const QString& key);

    QString toString() const;
    bool isEmpty() const;

private:
    QString m_key;
};

#endif // IDOS_TREE_PART_KEY_H
