#include <memory>

#include "eclipse/idosgrideclipsemetadata.h"
#include "eclipse/idossimulationcaseeclipsemetadata.h"
#include "idosdataproviderregistry.h"
#include "idosdataregistry.h"
#include "idosgridfiltermetadata.h"
#include "idosgridmetadata.h"
#include "idossimulationcasemetadata.h"
#include "idoswellmetadata.h"
#include "las/idoswelllasmetadata.h"
#include "log/idoslogger.h"
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
    m_dataRegistry->add(new IDOSSimulationCaseMetadata());
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
    if (registry.metadata(QStringLiteral("idos.case.simulation.eclipse")) == nullptr)
    {
        registry.add(new IDOSSimulationCaseEclipseMetadata());
    }
}
