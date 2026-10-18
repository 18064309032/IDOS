#include <memory>

#include "case/idossimulationcaseeclipsemetadata.h"
#include "case/idossimulationsummaryprovidermetadata.h"
#include "data/case/idossimulationcasemetadata.h"
#include "data/case/idossimulationsummaryresultsmetadata.h"
#include "data/fluidrock/idosfluidrocktablemetadata.h"
#include "data/fracture/idosfracturematerialcatalogdatametadata.h"
#include "data/fracture/idosfracturematerialcatalogobject.h"
#include "data/fracture/idosfracturescheduledatametadata.h"
#include "data/fracture/idosnaturalfracturesetdatametadata.h"
#include "data/grid/idosgridboundarydatametadata.h"
#include "data/grid/idosgridfiltermetadata.h"
#include "data/grid/idosgridmetadata.h"
#include "data/grid/idosgridpropertymetadata.h"
#include "data/grid/idosgridsurfacedatametadata.h"
#include "data/idosdataregistry.h"
#include "data/production/idosproductionhistorydatametadata.h"
#include "data/seismic/idosmicroseismicdatametadata.h"
#include "data/seismic/idosseismicvolumedatametadata.h"
#include "data/well/idoswellmetadata.h"
#include "fracture/idosfracturematerialcatalogmetadata.h"
#include "fracture/idosfractureschedulemetadata.h"
#include "fracture/idosnaturalfracturemetadata.h"
#include "grid/idosgridboundarymetadata.h"
#include "grid/idosgrideclipsemetadata.h"
#include "grid/idosgridpropertyeclipsemetadata.h"
#include "grid/idosgridsurfacemetadata.h"
#include "las/idoswelllasmetadata.h"
#include "logger/idoslogger.h"
#include "production/idosproductionhistorymetadata.h"
#include "providers/idosdataproviderregistry.h"
#include "seismic/idosmicroseismicmetadata.h"
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
    initDataRegistry(m_dataRegistry);
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
        registry.registerMetadata(new IDOSMicroseismicMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.seismic.segy")) == nullptr)
    {
        registry.registerMetadata(new IDOSSegyProviderMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.fracture.schedule.tsv")) == nullptr)
    {
        registry.registerMetadata(new IDOSFractureScheduleMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.naturalFracture.ascii")) == nullptr)
    {
        registry.registerMetadata(new IDOSNaturalFractureMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.production.vol")) == nullptr)
    {
        registry.registerMetadata(new IDOSProductionHistoryMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.fracture.materialCatalog.tsv")) == nullptr)
    {
        registry.registerMetadata(new IDOSFractureMaterialCatalogMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.gridproperty.eclipse")) == nullptr)
    {
        registry.registerMetadata(new IDOSGridPropertyEclipseMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.tops")) == nullptr)
    {
        registry.registerMetadata(new IDOSWellTopsMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.completion")) == nullptr)
    {
        registry.registerMetadata(new IDOSWellCompletionMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.checkshot")) == nullptr)
    {
        registry.registerMetadata(new IDOSWellCheckshotMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.header")) == nullptr)
    {
        registry.registerMetadata(new IDOSWellHeaderMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.path")) == nullptr)
    {
        registry.registerMetadata(new IDOSWellPathMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.well.las")) == nullptr)
    {
        registry.registerMetadata(new IDOSWellLasMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.grid.eclipse")) == nullptr)
    {
        registry.registerMetadata(new IDOSGridEclipseMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.grid.surface.zims")) == nullptr)
    {
        registry.registerMetadata(new IDOSGridSurfaceMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.grid.boundary.xygi")) == nullptr)
    {
        registry.registerMetadata(new IDOSGridBoundaryMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.case.simulation.eclipse")) == nullptr)
    {
        registry.registerMetadata(new IDOSSimulationCaseEclipseMetadata());
    }
    if (registry.metadata(QStringLiteral("idos.case.summary.eclipse")) == nullptr)
    {
        registry.registerMetadata(new IDOSSimulationSummaryProviderMetadata());
    }
}

void IDOSApplication::initDataRegistry(IDOSDataRegistry* registry)
{
    registry->registerMetadata(new IDOSWellMetadata());
    registry->registerMetadata(new IDOSGridMetadata());
    registry->registerMetadata(new IDOSGridPropertyMetadata());
    registry->registerMetadata(new IDOSGridFilterMetadata());
    registry->registerMetadata(new IDOSGridSurfaceDataMetadata());
    registry->registerMetadata(new IDOSGridBoundaryDataMetadata());
    registry->registerMetadata(new IDOSSimulationCaseMetadata());
    registry->registerMetadata(new IDOSSimulationSummaryResultsMetadata());
    registry->registerMetadata(new IDOSFluidRockTableMetadata(IDOSFluidRockTableObject::Category::Fluid));
    registry->registerMetadata(new IDOSFluidRockTableMetadata(IDOSFluidRockTableObject::Category::RockPhysics));
    registry->registerMetadata(new IDOSMicroseismicDataMetadata());
    registry->registerMetadata(new IDOSSeismicVolumeDataMetadata());
    registry->registerMetadata(new IDOSFractureScheduleDataMetadata());
    registry->registerMetadata(new IDOSNaturalFractureSetDataMetadata());
    registry->registerMetadata(new IDOSProductionHistoryDataMetadata());
    registry->registerMetadata(new IDOSFractureMaterialCatalogDataMetadata(
        IDOSFractureMaterialCatalogObject::Kind::Fluid));
    registry->registerMetadata(new IDOSFractureMaterialCatalogDataMetadata(
        IDOSFractureMaterialCatalogObject::Kind::Proppant));
}
