#include <exception>
#include <string>
#include <vector>

#include <QElapsedTimer>
#include <QFileInfo>
#include <QObject>
#include <QVariantMap>
#include <QVector>

#include <opm/common/utility/TimeService.hpp>
#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/SummaryNode.hpp>

#include "data/case/idossimulationsummaryresultsobject.h"

#include "idossimulationsummaryprovider.h"

bool IDOSSimulationSummaryProvider::canRead(const QString& filePath) const
{
    return QFileInfo(filePath).suffix().compare(QStringLiteral("SMSPEC"), Qt::CaseInsensitive) == 0;
}

QList<IDOSDataObject*> IDOSSimulationSummaryProvider::read(const QString& filePath)
{
    setLastError(QString());
    if (!canRead(filePath))
    {
        setLastError(QObject::tr("Select an ECLIPSE summary specification file (.SMSPEC)."));
        return {};
    }

    try
    {
        QElapsedTimer phase;
        phase.start();
        Opm::EclIO::ESmry summary(QFileInfo(filePath).absoluteFilePath().toStdString());
        if (!summary.all_steps_available())
        {
            setLastError(QObject::tr("The ECLIPSE summary results are incomplete."));
            return {};
        }

        reportReadPhase(QStringLiteral("summary-open-index"), phase.nsecsElapsed());
        phase.restart();
        // All series are imported: load them in one pass rather than one disk scan per key.
        summary.loadData();
        reportReadPhase(QStringLiteral("summary-bulk-read"), phase.nsecsElapsed());
        phase.restart();
        const std::vector<Opm::time_point> dates = summary.dates();
        const std::vector<Opm::EclIO::SummaryNode>& nodes = summary.summaryNodeList();
        if (dates.empty() || nodes.empty())
        {
            setLastError(QObject::tr("The ECLIPSE summary contains no time steps or variables."));
            return {};
        }

        QVector<qint64> timeValues;
        timeValues.reserve(static_cast<int>(dates.size()));
        for (const Opm::time_point& date : dates)
        {
            timeValues.append(date.time_since_epoch().count());
        }

        QList<QVariantMap> series;
        for (const Opm::EclIO::SummaryNode& node : nodes)
        {
            const std::vector<float>& sourceValues = summary.get(node);
            if (sourceValues.size() != dates.size())
            {
                continue;
            }
            QVector<double> values;
            values.reserve(static_cast<int>(sourceValues.size()));
            for (const float value : sourceValues)
            {
                values.append(static_cast<double>(value));
            }

            QVariantMap item;
            item.insert(QStringLiteral("key"), QString::fromStdString(node.unique_key()));
            item.insert(QStringLiteral("keyword"), QString::fromStdString(node.keyword));
            item.insert(QStringLiteral("entity"), QString::fromStdString(node.wgname));
            item.insert(QStringLiteral("unit"), QString::fromStdString(summary.get_unit(node)));
            item.insert(QStringLiteral("values"), QVariant::fromValue(values));
            series.append(item);
        }
        if (series.isEmpty())
        {
            setLastError(QObject::tr("The ECLIPSE summary contains no readable variable series."));
            return {};
        }

        reportReadPhase(QStringLiteral("summary-conversion"), phase.nsecsElapsed());
        IDOSSimulationSummaryResultsObject* results = new IDOSSimulationSummaryResultsObject();
        results->setName(QFileInfo(filePath).completeBaseName());
        results->setSourceFile(QFileInfo(filePath).absoluteFilePath());
        results->setTimeValues(timeValues);
        results->setSeries(series);
        return {results};
    }
    catch (const std::exception& exception)
    {
        setLastError(QObject::tr("Unable to read ECLIPSE summary results: %1")
                         .arg(QString::fromUtf8(exception.what())));
        return {};
    }
}
