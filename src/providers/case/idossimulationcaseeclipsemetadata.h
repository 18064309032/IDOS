#ifndef IDOS_SIMULATION_CASE_ECLIPSE_METADATA_H
#define IDOS_SIMULATION_CASE_ECLIPSE_METADATA_H

#include "idosprovidermetadata.h"

/**
 * @brief ECLIPSE 主输入文件（.DATA）工况 provider 的元数据 + 工厂。
 *
 * 扩展名匹配 *.DATA / *.data（不区分大小写）；canHandle 默认按扩展名匹配。
 */
class PROVIDERS_EXPORT IDOSSimulationCaseEclipseMetadata : public IDOSProviderMetadata
{
public:
    QString id() const override { return QStringLiteral("idos.case.simulation.eclipse"); }
    QString displayName() const override { return QStringLiteral("ECLIPSE Simulation Case"); }
    QStringList fileExtensions() const override
    {
        return QStringList{ QStringLiteral("*.DATA"), QStringLiteral("*.data") };
    }

    std::unique_ptr<IDOSDataProvider> createProvider() const override;
};

#endif // IDOS_SIMULATION_CASE_ECLIPSE_METADATA_H
