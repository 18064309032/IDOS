#include <climits>

#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QVector>

#include <vtk_hdf5.h>

#include "idosgridpropertyobject.h"

#include "idosgridpropertymetadata.h"

QString IDOSGridPropertyMetadata::typeId() const
{
    return QStringLiteral("idos.gridproperty");
}

QString IDOSGridPropertyMetadata::displayName() const
{
    return QObject::tr("Grid Property");
}

IDOSDataObject* IDOSGridPropertyMetadata::create() const
{
    return new IDOSGridPropertyObject();
}

bool IDOSGridPropertyMetadata::writeProjectData(qint64 group, const IDOSDataObject* object, QString& error) const
{
    const IDOSGridPropertyObject* property = qobject_cast<const IDOSGridPropertyObject*>(object);
    if (property == nullptr || property->typeId() != typeId())
    {
        error = QObject::tr("This object type does not support project storage: %1.").arg(typeId());
        return false;
    }
    QVariantMap data;
    data.insert(QStringLiteral("keyword"), property->keyword());
    data.insert(QStringLiteral("kind"), static_cast<int>(property->kind()));
    data.insert(QStringLiteral("gridId"), property->gridId());
    data.insert(QStringLiteral("nx"), property->numX());
    data.insert(QStringLiteral("ny"), property->numY());
    data.insert(QStringLiteral("nz"), property->numZ());
    data.insert(QStringLiteral("valueUnits"), static_cast<int>(property->valueUnits()));
    data.insert(QStringLiteral("values"), QVariant::fromValue(QVector<double>()));
    data.insert(QStringLiteral("reportStepValues"), QVariantMap());
    data.insert(QStringLiteral("currentReportStep"), property->currentReportStep());
    if (!IDOSDataMetadata::writeValue(group, QByteArray("Data"), QVariant::fromValue(data), error))
    {
        return false;
    }
    const hid_t arrays = H5Gcreate2(group, "GridPropertyArrays", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (arrays < 0)
    {
        error = QObject::tr("Cannot write a project data block.");
        return false;
    }
    bool success = (property->baseValues().isEmpty() || writeDoubleVector(arrays, "Values", property->baseValues())) &&
        writeReportSteps(arrays, property);
    success = H5Gclose(arrays) >= 0 && success;
    if (!success)
    {
        error = QObject::tr("Cannot write a project data block.");
    }
    return success;
}

IDOSDataObject* IDOSGridPropertyMetadata::readProjectData(qint64 group, QString& error) const
{
    htri_t arraysExist = -1;
    H5E_BEGIN_TRY
    {
        arraysExist = H5Lexists(group, "GridPropertyArrays", H5P_DEFAULT);
    }
    H5E_END_TRY
    if (arraysExist == 0)
    {
        return IDOSDataMetadata::readProjectData(group, error);
    }
    H5L_info_t linkInfo;
    hid_t arrays = -1;
    H5E_BEGIN_TRY
    {
        if (arraysExist > 0 && H5Lget_info(group, "GridPropertyArrays", &linkInfo, H5P_DEFAULT) >= 0 &&
            linkInfo.type == H5L_TYPE_HARD)
        {
            arrays = H5Gopen2(group, "GridPropertyArrays", H5P_DEFAULT);
        }
    }
    H5E_END_TRY
    QVariant value;
    bool success = arrays >= 0 && IDOSDataMetadata::readValue(group, QByteArray("Data"), value, error) &&
        value.type() == QVariant::Map;
    IDOSGridPropertyObject* property = success ? new IDOSGridPropertyObject() : nullptr;
    success = success && property->restoreProjectData(value.toMap(), error);
    QVector<double> values;
    qint64 expectedValueCount = 0;
    if (success)
    {
        expectedValueCount = static_cast<qint64>(property->numX()) * property->numY() * property->numZ();
    }
    htri_t valuesExist = -1;
    H5E_BEGIN_TRY
    {
        valuesExist = arrays >= 0 ? H5Lexists(arrays, "Values", H5P_DEFAULT) : -1;
    }
    H5E_END_TRY
    success = success && expectedValueCount >= 0 && expectedValueCount <= INT_MAX && valuesExist >= 0;
    if (success && valuesExist > 0)
    {
        success = readDoubleVector(arrays, "Values", values) && property->setValues(values);
    }
    else if (success && property->kind() == IDOSGridPropertyObject::Kind::Static)
    {
        success = expectedValueCount == 0 && property->setValues(values);
    }
    success = success && readReportSteps(arrays, property, error);
    if (arrays >= 0)
    {
        success = H5Gclose(arrays) >= 0 && success;
    }
    if (!success)
    {
        delete property;
        if (error.isEmpty())
        {
            error = QObject::tr("A project data block has an invalid type, size or value.");
        }
        return nullptr;
    }
    return property;
}

bool IDOSGridPropertyMetadata::writeDoubleVector(qint64 group, const char* name, const QVector<double>& values)
{
    const hsize_t length = static_cast<hsize_t>(values.size());
    const hid_t space = H5Screate_simple(1, &length, nullptr);
    const hid_t properties = H5Pcreate(H5P_DATASET_CREATE);
    bool success = space >= 0 && properties >= 0;
    if (success && length >= 1024)
    {
        const hsize_t chunk = length < 65536 ? length : 65536;
        success = H5Pset_chunk(properties, 1, &chunk) >= 0 && H5Pset_deflate(properties, 1) >= 0 &&
            H5Pset_fletcher32(properties) >= 0;
    }
    const hid_t dataset = success ? H5Dcreate2(group, name, H5T_IEEE_F64LE, space,
        H5P_DEFAULT, properties, H5P_DEFAULT) : -1;
    success = dataset >= 0;
    if (success && length > 0)
    {
        success = H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, values.constData()) >= 0;
    }
    if (dataset >= 0)
    {
        success = H5Dclose(dataset) >= 0 && success;
    }
    if (properties >= 0)
    {
        H5Pclose(properties);
    }
    if (space >= 0)
    {
        H5Sclose(space);
    }
    return success;
}

bool IDOSGridPropertyMetadata::writeReportSteps(qint64 group, const IDOSGridPropertyObject* object)
{
    const QList<int> steps = object->reportSteps();
    if (steps.isEmpty())
    {
        return true;
    }
    const hsize_t stepCount = static_cast<hsize_t>(steps.size());
    const hid_t stepSpace = H5Screate_simple(1, &stepCount, nullptr);
    const hid_t stepDataset = stepSpace >= 0 ? H5Dcreate2(group, "ReportSteps", H5T_STD_I32LE, stepSpace,
        H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) : -1;
    bool success = stepDataset >= 0;
    if (success && stepCount > 0)
    {
        QVector<int> stepValues;
        stepValues.reserve(steps.size());
        for (int step : steps)
        {
            stepValues.append(step);
        }
        success = H5Dwrite(stepDataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, stepValues.constData()) >= 0;
    }
    if (stepDataset >= 0)
    {
        success = H5Dclose(stepDataset) >= 0 && success;
    }
    if (stepSpace >= 0)
    {
        H5Sclose(stepSpace);
    }
    const QVector<double> firstValues = object->reportStepValues(steps.first());
    const hsize_t valueCount = static_cast<hsize_t>(firstValues.size());
    if (!success || stepCount == 0 || valueCount == 0)
    {
        return success;
    }
    const hsize_t dimensions[2] = {stepCount, valueCount};
    const hid_t valueSpace = H5Screate_simple(2, dimensions, nullptr);
    const hid_t properties = H5Pcreate(H5P_DATASET_CREATE);
    success = valueSpace >= 0 && properties >= 0;
    if (success && valueCount >= 1024)
    {
        const hsize_t chunks[2] = {1, valueCount < 65536 ? valueCount : 65536};
        success = H5Pset_chunk(properties, 2, chunks) >= 0 && H5Pset_deflate(properties, 1) >= 0 &&
            H5Pset_fletcher32(properties) >= 0;
    }
    const hid_t dataset = success ? H5Dcreate2(group, "ReportStepValues", H5T_IEEE_F64LE, valueSpace,
        H5P_DEFAULT, properties, H5P_DEFAULT) : -1;
    success = dataset >= 0;
    if (success)
    {
        const hid_t fileSpace = H5Dget_space(dataset);
        const hid_t memorySpace = H5Screate_simple(1, &valueCount, nullptr);
        success = fileSpace >= 0 && memorySpace >= 0;
        for (hsize_t row = 0; success && row < stepCount; ++row)
        {
            const QVector<double> values = object->reportStepValues(steps.at(static_cast<int>(row)));
            const hsize_t start[2] = {row, 0};
            const hsize_t selection[2] = {1, valueCount};
            success = static_cast<hsize_t>(values.size()) == valueCount &&
                H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, nullptr, selection, nullptr) >= 0 &&
                H5Dwrite(dataset, H5T_NATIVE_DOUBLE, memorySpace, fileSpace, H5P_DEFAULT, values.constData()) >= 0;
        }
        if (memorySpace >= 0)
        {
            H5Sclose(memorySpace);
        }
        if (fileSpace >= 0)
        {
            H5Sclose(fileSpace);
        }
    }
    if (dataset >= 0)
    {
        success = H5Dclose(dataset) >= 0 && success;
    }
    if (properties >= 0)
    {
        H5Pclose(properties);
    }
    if (valueSpace >= 0)
    {
        H5Sclose(valueSpace);
    }
    return success;
}

bool IDOSGridPropertyMetadata::readDoubleVector(qint64 group, const char* name, QVector<double>& values)
{
    H5L_info_t linkInfo;
    bool linkValid = false;
    H5E_BEGIN_TRY
    {
        linkValid = H5Lget_info(group, name, &linkInfo, H5P_DEFAULT) >= 0 && linkInfo.type == H5L_TYPE_HARD;
    }
    H5E_END_TRY
    if (!linkValid)
    {
        return false;
    }
    const hid_t dataset = H5Dopen2(group, name, H5P_DEFAULT);
    if (dataset < 0)
    {
        return false;
    }
    const hid_t type = H5Dget_type(dataset);
    const hid_t properties = H5Dget_create_plist(dataset);
    const hid_t space = H5Dget_space(dataset);
    hsize_t length = 0;
    bool success = type >= 0 && properties >= 0 && space >= 0 && H5Tequal(type, H5T_IEEE_F64LE) > 0 &&
        H5Pget_external_count(properties) == 0 && H5Pget_layout(properties) != H5D_VIRTUAL &&
        H5Sget_simple_extent_ndims(space) == 1 && H5Sget_simple_extent_dims(space, &length, nullptr) >= 0 &&
        length <= INT_MAX;
    if (success)
    {
        values.resize(static_cast<int>(length));
        if (length > 0)
        {
            success = H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, values.data()) >= 0;
        }
    }
    if (space >= 0)
    {
        H5Sclose(space);
    }
    if (properties >= 0)
    {
        H5Pclose(properties);
    }
    if (type >= 0)
    {
        H5Tclose(type);
    }
    H5Dclose(dataset);
    return success;
}

bool IDOSGridPropertyMetadata::readReportSteps(qint64 group, IDOSGridPropertyObject* object, QString& error)
{
    htri_t stepsExist = -1;
    H5E_BEGIN_TRY
    {
        stepsExist = H5Lexists(group, "ReportSteps", H5P_DEFAULT);
    }
    H5E_END_TRY
    if (stepsExist == 0)
    {
        return true;
    }
    if (stepsExist < 0)
    {
        return false;
    }
    H5L_info_t stepsLink;
    bool stepsLinkValid = false;
    H5E_BEGIN_TRY
    {
        stepsLinkValid = H5Lget_info(group, "ReportSteps", &stepsLink, H5P_DEFAULT) >= 0 &&
            stepsLink.type == H5L_TYPE_HARD;
    }
    H5E_END_TRY
    if (!stepsLinkValid)
    {
        return false;
    }
    const hid_t stepsDataset = H5Dopen2(group, "ReportSteps", H5P_DEFAULT);
    if (stepsDataset < 0)
    {
        return false;
    }
    const hid_t type = H5Dget_type(stepsDataset);
    const hid_t stepProperties = H5Dget_create_plist(stepsDataset);
    const hid_t space = H5Dget_space(stepsDataset);
    hsize_t stepCount = 0;
    bool success = type >= 0 && stepProperties >= 0 && space >= 0 && H5Tequal(type, H5T_STD_I32LE) > 0 &&
        H5Pget_external_count(stepProperties) == 0 && H5Pget_layout(stepProperties) != H5D_VIRTUAL &&
        H5Sget_simple_extent_ndims(space) == 1 && H5Sget_simple_extent_dims(space, &stepCount, nullptr) >= 0 &&
        stepCount <= 1000000;
    QVector<int> steps;
    if (success)
    {
        steps.resize(static_cast<int>(stepCount));
        if (stepCount > 0)
        {
            success = H5Dread(stepsDataset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, steps.data()) >= 0;
        }
    }
    if (space >= 0)
    {
        H5Sclose(space);
    }
    if (stepProperties >= 0)
    {
        H5Pclose(stepProperties);
    }
    if (type >= 0)
    {
        H5Tclose(type);
    }
    H5Dclose(stepsDataset);
    const qint64 expectedCount = static_cast<qint64>(object->numX()) * object->numY() * object->numZ();
    if (!success || expectedCount < 0 || expectedCount > INT_MAX)
    {
        return false;
    }
    if (stepCount == 0)
    {
        return true;
    }
    if (expectedCount == 0)
    {
        htri_t matrixExists = -1;
        H5E_BEGIN_TRY
        {
            matrixExists = H5Lexists(group, "ReportStepValues", H5P_DEFAULT);
        }
        H5E_END_TRY
        if (matrixExists != 0)
        {
            return false;
        }
        int previousStep = -1;
        for (int step : steps)
        {
            if (step <= previousStep || !object->setReportStepValues(step, QVector<double>()))
            {
                return false;
            }
            previousStep = step;
        }
        return true;
    }
    H5L_info_t matrixLink;
    bool matrixLinkValid = false;
    H5E_BEGIN_TRY
    {
        matrixLinkValid = H5Lget_info(group, "ReportStepValues", &matrixLink, H5P_DEFAULT) >= 0 &&
            matrixLink.type == H5L_TYPE_HARD;
    }
    H5E_END_TRY
    if (!matrixLinkValid)
    {
        return false;
    }
    const hid_t dataset = H5Dopen2(group, "ReportStepValues", H5P_DEFAULT);
    if (dataset < 0)
    {
        return false;
    }
    const hid_t matrixType = H5Dget_type(dataset);
    const hid_t properties = H5Dget_create_plist(dataset);
    const hid_t fileSpace = H5Dget_space(dataset);
    hsize_t dimensions[2] = {0, 0};
    success = matrixType >= 0 && properties >= 0 && fileSpace >= 0 && H5Tequal(matrixType, H5T_IEEE_F64LE) > 0 &&
        H5Pget_external_count(properties) == 0 && H5Pget_layout(properties) != H5D_VIRTUAL &&
        H5Sget_simple_extent_ndims(fileSpace) == 2 &&
        H5Sget_simple_extent_dims(fileSpace, dimensions, nullptr) >= 0 &&
        dimensions[0] == stepCount && dimensions[1] == static_cast<hsize_t>(expectedCount);
    const hsize_t memoryLength = static_cast<hsize_t>(expectedCount);
    const hid_t memorySpace = success ? H5Screate_simple(1, &memoryLength, nullptr) : -1;
    success = success && memorySpace >= 0;
    int previousStep = -1;
    for (hsize_t row = 0; success && row < stepCount; ++row)
    {
        QVector<double> values(static_cast<int>(expectedCount));
        const hsize_t start[2] = {row, 0};
        const hsize_t selection[2] = {1, memoryLength};
        const int step = steps.at(static_cast<int>(row));
        success = H5Sselect_hyperslab(fileSpace, H5S_SELECT_SET, start, nullptr, selection, nullptr) >= 0 &&
            H5Dread(dataset, H5T_NATIVE_DOUBLE, memorySpace, fileSpace, H5P_DEFAULT, values.data()) >= 0 &&
            step > previousStep && object->setReportStepValues(step, values);
        previousStep = step;
    }
    if (!success && error.isEmpty())
    {
        error = QObject::tr("A stored report step is invalid.");
    }
    if (memorySpace >= 0)
    {
        H5Sclose(memorySpace);
    }
    if (fileSpace >= 0)
    {
        H5Sclose(fileSpace);
    }
    if (properties >= 0)
    {
        H5Pclose(properties);
    }
    if (matrixType >= 0)
    {
        H5Tclose(matrixType);
    }
    H5Dclose(dataset);
    return success;
}
