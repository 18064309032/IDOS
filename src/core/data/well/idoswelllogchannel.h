#ifndef IDOS_WELL_LOG_CHANNEL_H
#define IDOS_WELL_LOG_CHANNEL_H

#include "idos_core.h"

#include <QString>
#include <QVector>

/**
 * @brief 一根测井曲线通道（Gamma / Perm / Porosity ...）。
 *
 * IDOSWellLogSet 的成员。普通类，值语义。自带深度索引，
 * 不要求等步长——LAS 数据集中 STEP=0、采样深度不规则，
 * 必须逐行存真实深度。
 *
 * NULL 采样（LAS -999.25）以 qQNaN() 存入，消费方用
 * qIsNaN() 判断，不参与统计和渲染。
 */
class CORE_EXPORT IDOSWellLogChannel
{
public:
    IDOSWellLogChannel();
    explicit IDOSWellLogChannel(const QString& name);

    /** 通道名（树显示 / 同名覆盖依据）。 */
    QString name() const;
    void setName(const QString& name);

    /** 值单位（如 "m"、"API"），仅展示用。 */
    QString unit() const;
    void setUnit(const QString& unit);

    /** 深度索引（米，与 values 等长）。 */
    QVector<double> depths() const;
    /** 曲线采样值（NULL 为 qQNaN）。 */
    QVector<double> values() const;
    /** 整体设置深度与值（两者须等长）。 */
    void setData(const QVector<double>& depths, const QVector<double>& values);

    /** 采样点数。 */
    int sampleCount() const;

    /** 在渲染/交会图中是否可见。 */
    bool isVisible() const;
    void setVisible(bool visible);

private:
    QString m_name;
    QString m_unit;
    QVector<double> m_depths;
    QVector<double> m_values;
    bool m_visible;
};

#endif // IDOS_WELL_LOG_CHANNEL_H
