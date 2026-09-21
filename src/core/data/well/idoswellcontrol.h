#ifndef IDOS_WELL_CONTROL_H
#define IDOS_WELL_CONTROL_H

#include "idos_core.h"

/**
 * @brief 井控参数。
 *
 * 对应 Eclipse WLCON 关键字。
 * 描述井和地层连通的物理参数，模拟器计算使用。
 */
class CORE_EXPORT IDOSWellControl
{
public:
    IDOSWellControl();

    /** 井指数（Well Index），井与地层连通的传导能力。 */
    double wellIndex() const;
    void setWellIndex(double wi);

    /** 皮肤因子（Skin Factor），钻井完井对地层的伤害。 */
    double skinFactor() const;
    void setSkinFactor(double skin);

    /** D-factor，非达西流系数。 */
    double dFactor() const;
    void setDFactor(double d);

    /** 是否启用井控参数。 */
    bool isActive() const;
    void setActive(bool active);

private:
    double m_wellIndex;
    double m_skinFactor;
    double m_dFactor;
    bool m_active;
};

#endif // IDOS_WELL_CONTROL_H
