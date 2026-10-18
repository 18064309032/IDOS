#include <algorithm>
#include <cmath>
#include <limits>
#include <random>

#include <QObject>
#include <QStringList>

#include "data/grid/idosgridobject.h"
#include "data/grid/idosgridpropertyobject.h"

#include "idosdfngenerator.h"

bool IDOSDfnGenerator::validate(const IDOSGridObject* grid, const IDOSGridPropertyObject* density,
                              const QJsonObject& parameters, QString& error)
{
    error.clear();
    if (grid == nullptr || grid->type() != IDOSGridObject::Type::CornerPoint
        || grid->nx() <= 0 || grid->ny() <= 0 || grid->nz() <= 0)
    {
        error = QObject::tr("Select a valid corner-point grid.");
        return false;
    }
    const qint64 plane = qint64(grid->nx()) * grid->ny();
    const qint64 cellLimit = std::numeric_limits<int>::max() / 8;
    if (plane > cellLimit || grid->nz() > cellLimit / plane)
    {
        error = QObject::tr("The grid geometry or ACTNUM size is invalid.");
        return false;
    }
    const qint64 cells = plane * grid->nz();
    const qint64 pillars = (qint64(grid->nx()) + 1) * (qint64(grid->ny()) + 1) * 6;
    if (cells > std::numeric_limits<int>::max() / 8 || grid->coord().size() != pillars
        || grid->zcorn().size() != cells * 8
        || (!grid->actnum().isEmpty() && grid->actnum().size() != cells))
    {
        error = QObject::tr("The grid geometry or ACTNUM size is invalid.");
        return false;
    }
    if (density != nullptr && (density->gridId() != grid->objectId()
        || density->kind() != IDOSGridPropertyObject::Kind::Static
        || density->numX() != grid->nx() || density->numY() != grid->ny()
        || density->numZ() != grid->nz() || density->values().size() != cells))
    {
        error = QObject::tr("The density weight must be a static property of the selected grid.");
        return false;
    }
    const QStringList numbers = {QStringLiteral("count"), QStringLiteral("seed"),
        QStringLiteral("firstLayer"), QStringLiteral("lastLayer"), QStringLiteral("minimumLength"),
        QStringLiteral("maximumLength"), QStringLiteral("aspectRatio"), QStringLiteral("strike"),
        QStringLiteral("dip"), QStringLiteral("strikeSpread"), QStringLiteral("dipSpread")};
    for (const QString& key : numbers)
    {
        if (!parameters.value(key).isDouble() || !std::isfinite(parameters.value(key).toDouble()))
        {
            error = QObject::tr("A required DFN parameter is missing or not finite: %1.").arg(key);
            return false;
        }
    }
    const double count = parameters.value(QStringLiteral("count")).toDouble();
    const double seed = parameters.value(QStringLiteral("seed")).toDouble();
    const double first = parameters.value(QStringLiteral("firstLayer")).toDouble();
    const double last = parameters.value(QStringLiteral("lastLayer")).toDouble();
    const double minimum = parameters.value(QStringLiteral("minimumLength")).toDouble();
    const double maximum = parameters.value(QStringLiteral("maximumLength")).toDouble();
    const double dip = parameters.value(QStringLiteral("dip")).toDouble();
    if (count < 1 || count > 10000 || count != std::floor(count)
        || seed < 0 || seed > 2147483647 || seed != std::floor(seed)
        || first < 1 || first != std::floor(first) || last < first || last > grid->nz() || last != std::floor(last)
        || minimum <= 0 || maximum < minimum || maximum > 1.0e9
        || parameters.value(QStringLiteral("aspectRatio")).toDouble() < 1.0
        || parameters.value(QStringLiteral("aspectRatio")).toDouble() > 1000.0
        || dip < 0 || dip > 90
        || std::abs(parameters.value(QStringLiteral("strike")).toDouble()) > 360
        || parameters.value(QStringLiteral("strikeSpread")).toDouble() < 0
        || parameters.value(QStringLiteral("strikeSpread")).toDouble() > 180
        || parameters.value(QStringLiteral("dipSpread")).toDouble() < 0
        || parameters.value(QStringLiteral("dipSpread")).toDouble() > 90)
    {
        error = QObject::tr("The DFN count, layers, lengths, angles or random seed are outside their valid ranges.");
        return false;
    }
    return true;
}

bool IDOSDfnGenerator::cellCoordinates(const IDOSGridObject* grid, int cell, QVector<double>& corners)
{
    const int i = cell % grid->nx();
    const int j = (cell / grid->nx()) % grid->ny();
    const int offsetI[4] = {0, 1, 1, 0};
    const int offsetJ[4] = {0, 0, 1, 1};
    corners.resize(24);
    for (int corner = 0; corner < 8; ++corner)
    {
        const int pillar = ((i + offsetI[corner % 4]) + (j + offsetJ[corner % 4]) * (grid->nx() + 1)) * 6;
        const double depth = grid->zcorn().at(cell * 8 + corner);
        const double top = grid->coord().at(pillar + 2);
        const double bottom = grid->coord().at(pillar + 5);
        const double fraction = bottom == top ? 0.0 : (depth - top) / (bottom - top);
        for (int axis = 0; axis < 3; ++axis)
        {
            const double value = axis == 2 ? depth : grid->coord().at(pillar + axis)
                + fraction * (grid->coord().at(pillar + axis + 3) - grid->coord().at(pillar + axis));
            if (!std::isfinite(value))
            {
                return false;
            }
            corners[corner * 3 + axis] = value;
        }
    }
    return true;
}

bool IDOSDfnGenerator::generate(const IDOSGridObject* grid, const IDOSGridPropertyObject* density,
    const QJsonObject& parameters, QVariantList& fractures, QString& error)
{
    fractures.clear();
    if (!validate(grid, density, parameters, error))
    {
        return false;
    }
    const int first = parameters.value(QStringLiteral("firstLayer")).toInt() - 1;
    const int last = parameters.value(QStringLiteral("lastLayer")).toInt();
    const int plane = grid->nx() * grid->ny();
    QVector<int> eligible;
    QVector<double> weights;
    double minimum[3] = {std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    double maximum[3] = {-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), -std::numeric_limits<double>::max()};
    QVector<double> corners;
    double largestWeight = 0.0;
    for (int cell = first * plane; cell < last * plane; ++cell)
    {
        const double weight = density != nullptr ? density->values().at(cell) : 1.0;
        if ((!grid->actnum().isEmpty() && grid->actnum().at(cell) == 0) || !std::isfinite(weight) || weight <= 0)
        {
            continue;
        }
        if (!cellCoordinates(grid, cell, corners))
        {
            error = QObject::tr("An eligible grid cell contains invalid coordinates.");
            return false;
        }
        eligible.append(cell);
        weights.append(weight);
        largestWeight = std::max(largestWeight, weight);
        for (int corner = 0; corner < 8; ++corner)
        {
            for (int axis = 0; axis < 3; ++axis)
            {
                minimum[axis] = std::min(minimum[axis], corners.at(corner * 3 + axis));
                maximum[axis] = std::max(maximum[axis], corners.at(corner * 3 + axis));
            }
        }
    }
    if (eligible.isEmpty() || maximum[0] <= minimum[0] || maximum[1] <= minimum[1] || maximum[2] <= minimum[2])
    {
        error = QObject::tr("No usable active cells with positive weights and nonzero extent were found in the selected layers.");
        return false;
    }
    for (double& weight : weights)
    {
        weight /= largestWeight;
    }
    std::mt19937 engine(static_cast<unsigned int>(parameters.value(QStringLiteral("seed")).toInt()));
    std::discrete_distribution<int> cellChoice(weights.cbegin(), weights.cend());
    std::uniform_real_distribution<double> lengthChoice(parameters.value(QStringLiteral("minimumLength")).toDouble(),
        parameters.value(QStringLiteral("maximumLength")).toDouble());
    std::normal_distribution<double> noise(0.0, 1.0);
    std::uniform_real_distribution<double> positionChoice(0.05, 0.95);
    const double radians = std::acos(-1.0) / 180.0;
    const QStringList axes = {QStringLiteral("east"), QStringLiteral("north"), QStringLiteral("down")};
    const int count = parameters.value(QStringLiteral("count")).toInt();
    const QVariantMap provenance = parameters.toVariantMap();
    for (int index = 0; index < count; ++index)
    {
        cellCoordinates(grid, eligible.at(cellChoice(engine)), corners);
        const double local[3] = {positionChoice(engine), positionChoice(engine), positionChoice(engine)};
        const int cornerX[4] = {0, 1, 1, 0};
        const int cornerY[4] = {0, 0, 1, 1};
        double center[3] = {0.0, 0.0, 0.0};
        for (int corner = 0; corner < 8; ++corner)
        {
            const double weight = (cornerX[corner % 4] != 0 ? local[0] : 1.0 - local[0])
                * (cornerY[corner % 4] != 0 ? local[1] : 1.0 - local[1])
                * (corner >= 4 ? local[2] : 1.0 - local[2]);
            for (int axis = 0; axis < 3; ++axis)
            {
                center[axis] += weight * (corners.at(corner * 3 + axis) - corners.at(axis));
            }
        }
        for (int axis = 0; axis < 3; ++axis)
        {
            center[axis] += corners.at(axis);
        }
        double strike = parameters.value(QStringLiteral("strike")).toDouble()
            + parameters.value(QStringLiteral("strikeSpread")).toDouble() * noise(engine);
        strike = std::fmod(strike, 360.0);
        if (strike < 0)
        {
            strike += 360.0;
        }
        const double dip = std::clamp(parameters.value(QStringLiteral("dip")).toDouble()
            + parameters.value(QStringLiteral("dipSpread")).toDouble() * noise(engine), 0.0, 90.0);
        const double angle = strike * radians;
        const double inclination = dip * radians;
        const double u[3] = {std::cos(angle), std::sin(angle), 0.0};
        const double v[3] = {-std::cos(inclination) * std::sin(angle),
                             std::cos(inclination) * std::cos(angle), std::sin(inclination)};
        const double requestedLength = lengthChoice(engine);
        const double halfLength = requestedLength * 0.5;
        const double halfWidth = halfLength / parameters.value(QStringLiteral("aspectRatio")).toDouble();
        double fit = 1.0;
        for (int axis = 0; axis < 3; ++axis)
        {
            const double extent = std::abs(u[axis] * halfLength) + std::abs(v[axis] * halfWidth);
            if (extent > 0)
            {
                fit = std::min(fit, 0.98 * std::min(center[axis] - minimum[axis], maximum[axis] - center[axis]) / extent);
            }
        }
        if (!std::isfinite(fit) || fit <= 0)
        {
            error = QObject::tr("A fracture cannot fit within the selected grid bounds.");
            fractures.clear();
            return false;
        }
        const int signU[4] = {-1, 1, 1, -1};
        const int signV[4] = {-1, -1, 1, 1};
        QVariantList vertices;
        for (int vertex = 0; vertex < 4; ++vertex)
        {
            QVariantMap point;
            for (int axis = 0; axis < 3; ++axis)
            {
                point.insert(axes.at(axis), center[axis] + fit * (signU[vertex] * halfLength * u[axis] + signV[vertex] * halfWidth * v[axis]));
            }
            vertices.append(point);
        }
        QVariantMap record;
        record.insert(QStringLiteral("id"), index + 1);
        record.insert(QStringLiteral("setId"), 1);
        record.insert(QStringLiteral("vertices"), vertices);
        record.insert(QStringLiteral("normal"), QVariantList{u[1] * v[2], -u[0] * v[2], u[0] * v[1] - u[1] * v[0]});
        record.insert(QStringLiteral("properties"), QVariantMap{{QStringLiteral("Strike"), strike},
            {QStringLiteral("Dip"), dip}, {QStringLiteral("Length"), requestedLength * fit},
            {QStringLiteral("Width"), 2.0 * halfWidth * fit}});
        record.insert(QStringLiteral("generation"), provenance);
        fractures.append(record);
    }
    return true;
}
