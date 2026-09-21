#ifndef IDOS_WELL_COMPLETION_H
#define IDOS_WELL_COMPLETION_H

#include "idos_core.h"

/**
 * @brief 油井射孔段数据。
 *
 * 对应 Eclipse COMPDAT 关键字的一条射孔记录。
 * 独立类但不继承 IDOSObject，保持值语义以便拷贝和容器存储。
 */
class CORE_EXPORT IDOSWellCompletion
{
public:
    IDOSWellCompletion();
    IDOSWellCompletion(int k, bool open, double bottom, double top);

    /** 网格层号（从 1 开始）。 */
    int k() const;
    void setK(int k);

    /** 该层是否打开。 */
    bool isOpen() const;
    void setOpen(bool open);

    /** 射孔底深（米）。 */
    double bottom() const;
    void setBottom(double bottom);

    /** 射孔顶深（米）。 */
    double top() const;
    void setTop(double top);

private:
    int m_k;
    bool m_open;
    double m_bottom;
    double m_top;
};

#endif // IDOS_WELL_COMPLETION_H
