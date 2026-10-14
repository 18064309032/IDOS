#include <memory>

#include "case/idossimulationcaseeclipsemetadata.h"
#include "grid/idosgridboundarymetadata.h"
#include "grid/idosgrideclipsemetadata.h"
#include "grid/idosgridpropertyeclipsemetadata.h"
#include "grid/idosgridsurfacemetadata.h"
#include "idosdataproviderregistry.h"
#include "idosdataregistry.h"
#include "idosfluidrocktablemetadata.h"
#include "idosfracturematerialcatalog.h"
#include "idosfracturematerialcatalogdatametadata.h"
#include "idosfracturematerialcatalogmetadata.h"
#include "idosfracturescheduledatametadata.h"
#include "idosfractureschedulemetadata.h"
#include "idosgridboundarydatametadata.h"
#include "idosgridfiltermetadata.h"
#include "idosgridmetadata.h"
#include "idosgridsurfacedatametadata.h"
#include "idosmicroseismicdatametadata.h"
#include "idosmicroseismicmetadata.h"
#include "idosseismicvolumedatametadata.h"
#include "idosnaturalfracturemetadata.h"
#include "idosnaturalfracturesetdatametadata.h"
#include "idosproductionhistorydatametadata.h"
#include "idosproductionhistorymetadata.h"
#include "idossimulationcasemetadata.h"
#include "idoswellmetadata.h"
#include "las/idoswelllasmetadata.h"
#include "log/idoslogger.h"
#include "seismic/idossegyprovidermetadata.h"
#include "well/idoswellcheckshotmetadata.h"
#include "well/idoswellcompletionmetadata.h"
#include "well/idoswellheadermetadata.h"
#include "well/idoswellpathmetadata.h"
#include "well/idoswelltopsmetadata.h"

#include "idosapplication.h"

IDOSApplication::IDOSApplication(int& argc, char** argv)
    :
    QApplication(argc, argv)
    , m_dataRegistry(new IDOSDataRegistry(this))
{
    qInstallMessageHandler(IDOSApplication::handleQtMessage);
    initProviders();
    m_dataRegistry->add(new IDOSWellMetadata());
    m_dataRegistry->add(new IDOSGridMetadata());
    m_dataRegistry->add(new IDOSGridFilterMetadata());
    m_dataRegistry->add(new IDOSGridSurfaceDataMetadata());
    m_dataRegistry->add(new IDOSGridBoundaryDataMetadata());
    m_dataRegistry->add(new IDOSSimulationCaseMetadata());
    m_dataRegistry->add(new IDOSFluidRockTableMetadata(IDOSFluidRockTable::Category::Fluid));
    m_dataRegistry->add(new IDOSFluidRockTableMetadata(IDOSFluidRockTable::Category::RockPhysics));
    m_dataRegistry->add(new IDOSMicroseismicDataMetadata());
    m_dataRegistry->add(new IDOSSeismicVolumeDataMetadata());
    m_dataRegistry->add(new IDOSFractureScheduleDataMetadata());
    m_dataRegistry->add(new IDOSNaturalFractureSetDataMetadata());
    m_dataRegistry->add(new IDOSProductionHistoryDataMetadata());
    m_dataRegistry->add(new IDOSFractureMaterialCatalogDataMetadata(
        IDOSFractureMaterialCatalog::Kind::Fluid));
    m_dataRegistry->add(new IDOSFractureMaterialCatalogDataMetadata(
        IDOSFractureMaterialCatalog::Kind::Proppant));
}

IDOSDataRegistry* IDOSApplication::dataRegistry() const
{
    return m_dataRegistry;
}

IDOSApplication::~IDOSApplication()
{
    IDOS_INFO(tr("Application shutdown is finishing."));
    qInstallMessageHandler(nullptr);
}

void IDOSApplication::handleQtMessage(QtMsgType type,
                                      const QMessageLogContext& context,
                                      const QString& message)
{
    const QString file = context.file ? QString::fromUtf8(context.file) : QString();
    const QString function = context.function ? QString::fromUtf8(context.function) : QString();

    IDOSLogger& logger = IDOSLogger::instance();
    if (type == QtInfoMsg)
    {
        logger.info(message, file, context.line, function);
    }
    else if (type == QtWarningMsg)
    {
        logger.warn(message, file, context.line, function);
    }
    else if (type == QtCriticalMsg)
    {
        logger.error(message, file, context.line, function);
    }
    else if (type == QtFatalMsg)
    {
        logger.fatal(message, file, context.line, function);
    }
    else
    {
        logger.debug(message, file, context.line, function);
    }
}

void IDOSApplication::initProviders()
{
    IDOSDataProviderRegistry& registry = IDOSDataProviderRegistry::instance();
    if (registry.metadata(QStringLiteral("idos.microseismic.tsv")) == nullptr)
    {
        registry.add(new IDOSMicroseismicMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.seismic.segy")) == nullptr)
    {
        registry.add(new IDOSSegyProviderMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.fracture.schedule.tsv")) == nullptr)
    {
        registry.add(new IDOSFractureScheduleMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.naturalFracture.ascii")) == nullptr)
    {
        registry.add(new IDOSNaturalFractureMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.production.vol")) == nullptr)
    {
        registry.add(new IDOSProductionHistoryMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.fracture.materialCatalog.tsv")) == nullptr)
    {
        registry.add(new IDOSFractureMaterialCatalogMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.gridproperty.eclipse")) == nullptr)
    {
        registry.add(new IDOSGridPropertyEclipseMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.tops")) == nullptr)
    {
        registry.add(new IDOSWellTopsMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.completion")) == nullptr)
    {
        registry.add(new IDOSWellCompletionMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.checkshot")) == nullptr)
    {
        registry.add(new IDOSWellCheckshotMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.header")) == nullptr)
    {
        registry.add(new IDOSWellHeaderMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.path")) == nullptr)
    {
        registry.add(new IDOSWellPathMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.las")) == nullptr)
    {
        registry.add(new IDOSWellLasMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.grid.eclipse")) == nullptr)
    {
        registry.add(new IDOSGridEclipseMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.grid.surface.zims")) == nullptr)
    {
        registry.add(new IDOSGridSurfaceMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.grid.boundary.xygi")) == nullptr)
    {
        registry.add(new IDOSGridBoundaryMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.case.simulation.eclipse")) == nullptr)
    {
        registry.add(new IDOSSimulationCaseEclipseMetadata());
    }
}
