#ifndef IDOS_WELL_LOG_SET_H
#define IDOS_WELL_LOG_SET_H

#include "idos_core.h"
#include "idoswelllogchannel.h"

#include <QList>
#include <QString>

/**
 * @brief 一口井的全部测井曲线集合（"Well Logs" 树节点）。
 *
 * IDOSWell 的组合成员，普通类，值语义。通道以值存储。
 * 重复导入按通道名覆盖：同名旧通道替换为新通道。
 */
class CORE_EXPORT IDOSWellLogSet
{
public:
    IDOSWellLogSet();

    /** 全部通道（按导入顺序）。 */
    QList<IDOSWellLogChannel> channels() const;

    /** 按名查通道；找不到返回 nullptr（区分大小写）。 */
    const IDOSWellLogChannel* channel(const QString& name) const;
    IDOSWellLogChannel* channel(const QString& name);

    /**
     * @brief 加入通道；同名通道已存在则替换为新通道。
     */
    void addChannel(const IDOSWellLogChannel& channel);

    int channelCount() const;
    bool isEmpty() const;
    void clear();

private:
    QList<IDOSWellLogChannel> m_channels;
};

#endif // IDOS_WELL_LOG_SET_H
