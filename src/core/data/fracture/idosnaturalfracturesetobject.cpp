#include <algorithm>
#include <cmath>

#include <QObject>
#include <QSet>
#include <QStringList>

#include "idosnaturalfracturesetobject.h"

IDOSNaturalFractureSetObject::IDOSNaturalFractureSetObject(QObject* parent)
    :
    IDOSDataObject(parent)
    , m_coordinateScale(1.0)
{
}

QString IDOSNaturalFractureSetObject::typeId() const
{
    return QStringLiteral("idos.naturalFracture.set");
}

QString IDOSNaturalFractureSetObject::sourceFile() const
{
    return m_sourceFile;
}

void IDOSNaturalFractureSetObject::setSourceFile(const QString& sourceFile)
{
    if (m_sourceFile == sourceFile)
    {
        return;
    }
    m_sourceFile = sourceFile;
    emit dataChanged();
}

QString IDOSNaturalFractureSetObject::lengthUnit() const
{
    return m_lengthUnit;
}

void IDOSNaturalFractureSetObject::setLengthUnit(const QString& lengthUnit)
{
    if (m_lengthUnit == lengthUnit)
    {
        return;
    }
    m_lengthUnit = lengthUnit;
    emit dataChanged();
}

double IDOSNaturalFractureSetObject::coordinateScale() const
{
    return m_coordinateScale;
}

void IDOSNaturalFractureSetObject::setCoordinateScale(double coordinateScale)
{
    if (m_coordinateScale == coordinateScale)
    {
        return;
    }
    m_coordinateScale = coordinateScale;
    emit dataChanged();
}

QVariantList IDOSNaturalFractureSetObject::fractures() const
{
    return m_fractures;
}

void IDOSNaturalFractureSetObject::setFractures(const QVariantList& fractures)
{
    if (m_fractures == fractures)
    {
        return;
    }
    m_fractures = fractures;
    emit dataChanged();
}

void IDOSNaturalFractureSetObject::appendGeometryIssue(QVariantList* issues, const QString& code,
                                               const QString& message, bool error) const
{
    QVariantMap issue;
    issue.insert(QStringLiteral("code"), code);
    issue.insert(QStringLiteral("message"), message);
    issue.insert(QStringLiteral("error"), error);
    issues->append(issue);
}

double IDOSNaturalFractureSetObject::projectedOrientation(const QVector<double>& coordinates, int first,
                                                  int second, int third, int xAxis, int yAxis) const
{
    return (coordinates.at(second * 3 + xAxis) - coordinates.at(first * 3 + xAxis))
            * (coordinates.at(third * 3 + yAxis) - coordinates.at(first * 3 + yAxis))
        - (coordinates.at(second * 3 + yAxis) - coordinates.at(first * 3 + yAxis))
            * (coordinates.at(third * 3 + xAxis) - coordinates.at(first * 3 + xAxis));
}

bool IDOSNaturalFractureSetObject::edgesIntersect(const QVector<double>& coordinates, int first, int second,
                                          int xAxis, int yAxis, double lengthTolerance,
                                          double areaTolerance) const
{
    const int count = coordinates.size() / 3;
    const int nextFirst = (first + 1) % count;
    const int nextSecond = (second + 1) % count;
    const double orientations[4] = {
        projectedOrientation(coordinates, first, nextFirst, second, xAxis, yAxis),
        projectedOrientation(coordinates, first, nextFirst, nextSecond, xAxis, yAxis),
        projectedOrientation(coordinates, second, nextSecond, first, xAxis, yAxis),
        projectedOrientation(coordinates, second, nextSecond, nextFirst, xAxis, yAxis)};
    if (((orientations[0] > areaTolerance && orientations[1] < -areaTolerance)
         || (orientations[0] < -areaTolerance && orientations[1] > areaTolerance))
        && ((orientations[2] > areaTolerance && orientations[3] < -areaTolerance)
            || (orientations[2] < -areaTolerance && orientations[3] > areaTolerance)))
    {
        return true;
    }
    const int segmentStarts[4] = {first, first, second, second};
    const int segmentEnds[4] = {nextFirst, nextFirst, nextSecond, nextSecond};
    const int testedPoints[4] = {second, nextSecond, first, nextFirst};
    for (int index = 0; index < 4; ++index)
    {
        if (std::abs(orientations[index]) > areaTolerance)
        {
            continue;
        }
        const double x = coordinates.at(testedPoints[index] * 3 + xAxis);
        const double y = coordinates.at(testedPoints[index] * 3 + yAxis);
        const double ax = coordinates.at(segmentStarts[index] * 3 + xAxis);
        const double ay = coordinates.at(segmentStarts[index] * 3 + yAxis);
        const double bx = coordinates.at(segmentEnds[index] * 3 + xAxis);
        const double by = coordinates.at(segmentEnds[index] * 3 + yAxis);
        if (x >= std::min(ax, bx) - lengthTolerance && x <= std::max(ax, bx) + lengthTolerance
            && y >= std::min(ay, by) - lengthTolerance && y <= std::max(ay, by) + lengthTolerance)
        {
            return true;
        }
    }
    return false;
}

QVariantMap IDOSNaturalFractureSetObject::inspectFracture(int index) const
{
    const QVariantMap fracture = m_fractures.at(index).toMap();
    QVariantMap report;
    QVariantList issues;
    report.insert(QStringLiteral("index"), index);
    report.insert(QStringLiteral("fractureId"), fracture.value(QStringLiteral("id")));
    const QVariantList vertices = fracture.value(QStringLiteral("vertices")).toList();
    QVector<double> coordinates;
    const QStringList axes = {QStringLiteral("east"), QStringLiteral("north"), QStringLiteral("down")};
    bool valid = vertices.size() >= 3;
    if (!valid)
    {
        appendGeometryIssue(&issues, QStringLiteral("vertex_count"), tr("At least three vertices are required."), true);
    }
    for (const QVariant& vertexValue : vertices)
    {
        const QVariantMap vertex = vertexValue.toMap();
        for (const QString& axis : axes)
        {
            bool converted = false;
            const double coordinate = vertex.value(axis).toDouble(&converted);
            if (!converted || !std::isfinite(coordinate))
            {
                valid = false;
            }
            coordinates.append(coordinate);
        }
    }
    if (!valid && vertices.size() >= 3)
    {
        appendGeometryIssue(&issues, QStringLiteral("coordinate"), tr("A vertex coordinate is missing or not finite."), true);
    }
    if (valid)
    {
        double span = 0.0;
        double extents[3] = {0.0, 0.0, 0.0};
        QVariantList center;
        for (int axis = 0; axis < 3; ++axis)
        {
            double minimum = coordinates.at(axis);
            double maximum = minimum;
            for (int vertex = 1; vertex < vertices.size(); ++vertex)
            {
                minimum = std::min(minimum, coordinates.at(vertex * 3 + axis));
                maximum = std::max(maximum, coordinates.at(vertex * 3 + axis));
            }
            extents[axis] = maximum - minimum;
            span = std::max(span, extents[axis]);
            center.append(minimum + (maximum - minimum) * 0.5);
        }
        report.insert(QStringLiteral("center"), center);
        const double lengthTolerance = std::max(1.0e-12, span * 1.0e-9);
        const double areaTolerance = std::max(1.0e-24, span * lengthTolerance);
        bool duplicate = false;
        for (int first = 0; first < vertices.size(); ++first)
        {
            for (int second = first + 1; second < vertices.size(); ++second)
            {
                double distanceSquared = 0.0;
                for (int axis = 0; axis < 3; ++axis)
                {
                    const double difference = coordinates.at(first * 3 + axis) - coordinates.at(second * 3 + axis);
                    distanceSquared += difference * difference;
                }
                if (distanceSquared <= lengthTolerance * lengthTolerance)
                {
                    duplicate = true;
                }
            }
        }
        if (duplicate)
        {
            appendGeometryIssue(&issues, QStringLiteral("duplicate_vertex"), tr("The polygon contains repeated vertices."), true);
            valid = false;
        }
        double normal[3] = {0.0, 0.0, 0.0};
        for (int vertex = 0; vertex < vertices.size(); ++vertex)
        {
            const int next = (vertex + 1) % vertices.size();
            double first[3];
            double second[3];
            for (int axis = 0; axis < 3; ++axis)
            {
                first[axis] = coordinates.at(vertex * 3 + axis) - coordinates.at(axis);
                second[axis] = coordinates.at(next * 3 + axis) - coordinates.at(axis);
            }
            normal[0] += first[1] * second[2] - first[2] * second[1];
            normal[1] += first[2] * second[0] - first[0] * second[2];
            normal[2] += first[0] * second[1] - first[1] * second[0];
        }
        const double normalLength = std::hypot(normal[0], normal[1], normal[2]);
        report.insert(QStringLiteral("area"), normalLength * 0.5);
        const bool hasNormal = std::isfinite(normalLength) && normalLength > areaTolerance;
        if (!hasNormal)
        {
            appendGeometryIssue(&issues, QStringLiteral("degenerate"), tr("The polygon has zero or negligible area."), true);
            valid = false;
        }
        else
        {
            // Planarity is an engineering tolerance, separate from topology checks.
            // Rounded ASCII map coordinates need more than floating-point tolerance.
            const double planeTolerance = std::max(lengthTolerance, span * 1.0e-4);
            double maximumPlaneDistance = 0.0;
            bool planar = true;
            for (int vertex = 0; vertex < vertices.size(); ++vertex)
            {
                double distance = 0.0;
                for (int axis = 0; axis < 3; ++axis)
                {
                    distance += (coordinates.at(vertex * 3 + axis) - coordinates.at(axis)) * normal[axis] / normalLength;
                }
                maximumPlaneDistance = std::max(maximumPlaneDistance, std::abs(distance));
                if (std::abs(distance) > planeTolerance)
                {
                    planar = false;
                }
            }
            report.insert(QStringLiteral("planeTolerance"), planeTolerance);
            report.insert(QStringLiteral("maximumPlaneDistance"), maximumPlaneDistance);
            if (!planar)
            {
                appendGeometryIssue(&issues, QStringLiteral("nonplanar"), tr("The polygon vertices are not coplanar."), true);
                valid = false;
            }
        }
        int omittedAxis = 0;
        for (int axis = 1; axis < 3; ++axis)
        {
            if ((hasNormal && std::abs(normal[axis]) > std::abs(normal[omittedAxis]))
                || (!hasNormal && extents[axis] < extents[omittedAxis]))
            {
                omittedAxis = axis;
            }
        }
        bool intersection = false;
        for (int first = 0; first < vertices.size(); ++first)
        {
            for (int second = first + 1; second < vertices.size(); ++second)
            {
                if (second == first + 1 || (first == 0 && second == vertices.size() - 1))
                {
                    continue;
                }
                if (edgesIntersect(coordinates, first, second, (omittedAxis + 1) % 3,
                                   (omittedAxis + 2) % 3, lengthTolerance, areaTolerance))
                {
                    intersection = true;
                }
            }
        }
        if (intersection)
        {
            appendGeometryIssue(&issues, QStringLiteral("self_intersection"), tr("The polygon edges intersect."), true);
            valid = false;
        }
        const QVariantList suppliedNormal = fracture.value(QStringLiteral("normal")).toList();
        bool normalValid = suppliedNormal.size() == 3;
        double supplied[3] = {0.0, 0.0, 0.0};
        for (int axis = 0; axis < suppliedNormal.size() && axis < 3; ++axis)
        {
            bool converted = false;
            supplied[axis] = suppliedNormal.at(axis).toDouble(&converted);
            normalValid = normalValid && converted && std::isfinite(supplied[axis]);
        }
        const double suppliedLength = std::hypot(supplied[0], supplied[1], supplied[2]);
        if (!normalValid || !std::isfinite(suppliedLength) || suppliedLength < 1.0e-12)
        {
            appendGeometryIssue(&issues, QStringLiteral("normal"), tr("The supplied normal is missing, zero or not finite."), false);
        }
        else if (hasNormal)
        {
            const double alignment = std::abs((normal[0] / normalLength) * (supplied[0] / suppliedLength)
                + (normal[1] / normalLength) * (supplied[1] / suppliedLength)
                + (normal[2] / normalLength) * (supplied[2] / suppliedLength));
            if (alignment < 1.0 - 1.0e-6)
            {
                appendGeometryIssue(&issues, QStringLiteral("normal_alignment"), tr("The supplied normal does not match the polygon plane."), false);
            }
        }
    }
    const QVariantMap properties = fracture.value(QStringLiteral("properties")).toMap();
    for (QVariantMap::const_iterator iterator = properties.constBegin(); iterator != properties.constEnd(); ++iterator)
    {
        bool converted = false;
        const double value = iterator.value().toDouble(&converted);
        const QString key = iterator.key().toLower();
        if (!converted || !std::isfinite(value)
            || ((key == QStringLiteral("aperture") || key == QStringLiteral("permeability")
                 || key == QStringLiteral("compressibility") || key == QStringLiteral("porosity")) && value < 0.0)
            || (key == QStringLiteral("porosity") && value > 1.0))
        {
            appendGeometryIssue(&issues, QStringLiteral("property"), tr("Property %1 has an invalid value.").arg(iterator.key()), true);
        }
    }
    report.insert(QStringLiteral("valid"), valid);
    report.insert(QStringLiteral("coordinates"), QVariant::fromValue(coordinates));
    report.insert(QStringLiteral("issues"), issues);
    return report;
}

QVariantList IDOSNaturalFractureSetObject::geometryReport() const
{
    QVariantList reports;
    QSet<int> ids;
    for (int index = 0; index < m_fractures.size(); ++index)
    {
        QVariantMap report = inspectFracture(index);
        QVariantList issues = report.value(QStringLiteral("issues")).toList();
        bool validId = false;
        const int id = report.value(QStringLiteral("fractureId")).toInt(&validId);
        if (!validId || id <= 0 || ids.contains(id))
        {
            appendGeometryIssue(&issues, QStringLiteral("fracture_id"), tr("The fracture ID is missing, nonpositive or duplicated."), true);
        }
        ids.insert(id);
        report.insert(QStringLiteral("issues"), issues);
        reports.append(report);
    }
    return reports;
}

bool IDOSNaturalFractureSetObject::canClone() const
{
    return true;
}

IDOSDataObject* IDOSNaturalFractureSetObject::clone() const
{
    IDOSNaturalFractureSetObject* copy = new IDOSNaturalFractureSetObject();
    copy->setName(name());
    copy->m_sourceFile = m_sourceFile;
    copy->m_lengthUnit = m_lengthUnit;
    copy->m_coordinateScale = m_coordinateScale;
    copy->m_fractures = m_fractures;
    return copy;
}

QVariantMap IDOSNaturalFractureSetObject::projectData() const
{
    QVariantMap data;
    data.insert(QStringLiteral("sourceFile"), QVariant::fromValue(m_sourceFile));
    data.insert(QStringLiteral("lengthUnit"), QVariant::fromValue(m_lengthUnit));
    data.insert(QStringLiteral("coordinateScale"), QVariant::fromValue(m_coordinateScale));
    data.insert(QStringLiteral("fractures"), QVariant::fromValue(m_fractures));
    return data;
}

bool IDOSNaturalFractureSetObject::restoreProjectData(const QVariantMap& data, QString& error)
{
    if (!data.contains(QStringLiteral("sourceFile")) || data.value(QStringLiteral("sourceFile")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("sourceFile"));
        return false;
    }
    if (!data.contains(QStringLiteral("lengthUnit")) || data.value(QStringLiteral("lengthUnit")).userType() != qMetaTypeId<QString>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("lengthUnit"));
        return false;
    }
    if (!data.contains(QStringLiteral("coordinateScale")) || data.value(QStringLiteral("coordinateScale")).userType() != qMetaTypeId<double>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("coordinateScale"));
        return false;
    }
    if (!data.contains(QStringLiteral("fractures")) || data.value(QStringLiteral("fractures")).userType() != qMetaTypeId<QVariantList>())
    {
        error = QObject::tr("A stored object field is missing or has an invalid type: %1.").arg(QStringLiteral("fractures"));
        return false;
    }
    m_sourceFile = data.value(QStringLiteral("sourceFile")).value<QString>();
    m_lengthUnit = data.value(QStringLiteral("lengthUnit")).value<QString>();
    m_coordinateScale = data.value(QStringLiteral("coordinateScale")).value<double>();
    m_fractures = data.value(QStringLiteral("fractures")).value<QVariantList>();
    return true;
}
