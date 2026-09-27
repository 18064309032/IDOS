#ifndef IDOS_SIMULATION_CASE_ECLIPSE_PROVIDER_H
#define IDOS_SIMULATION_CASE_ECLIPSE_PROVIDER_H

#include "idosdataprovider.h"
#include "idosgridproperty.h"

#include <QVector>
#include <vector>

class IDOSGrid;
class IDOSSimulationCaseObject;

namespace Opm
{
class Deck;
class EclipseGrid;
class FieldPropsManager;
} // namespace Opm

/**
 * @brief ECLIPSE 主输入文件（.DATA）工况解析 provider。
 *
 * 用 OPM (opm-common) 的 Parser 把 .DATA 解析成 Deck，提取：
 * - TITLE 关键字 → 工况名（缺失则用文件 base name）
 * - SCHEDULE 段所有 WELSPECS 关键字 → 井名清单（pending）
 *
 * 不直接解析 grid 引用，井名暂存进 IDOSSimulationCaseObject，导入落库后再解析成 objectId 引用。
 */
class PROVIDERS_EXPORT IDOSSimulationCaseEclipseProvider : public IDOSDataProvider
{
  public:
    QList<IDOSDataObject*> read(const QString& filePath) override;

  private:
    bool convertEclipseGrid(const Opm::EclipseGrid& eclipseGrid, QVector<double>* coord, QVector<double>* zcorn,
                            QVector<int>* actnum, QString* error) const;
    QVector<double> doubleVectorToQVector(const std::vector<double>& values) const;
    QVector<double> intVectorToDoubleQVector(const std::vector<int>& values) const;
    void appendDoubleProperty(QList<IDOSDataObject*>* result, const Opm::FieldPropsManager& fieldProps, IDOSGrid* grid,
                              const QString& keyword, IDOSGridProperty::Kind kind) const;
    void appendIntProperty(QList<IDOSDataObject*>* result, const Opm::FieldPropsManager& fieldProps, IDOSGrid* grid,
                           const QString& keyword) const;
    void appendActnumProperty(QList<IDOSDataObject*>* result, IDOSGrid* grid) const;
    void appendGridObjects(QList<IDOSDataObject*>* result, IDOSSimulationCaseObject* simulationCase,
                           const Opm::Deck& deck) const;
};

#endif // IDOS_SIMULATION_CASE_ECLIPSE_PROVIDER_H

