#include <climits>
#include <cstdio>

#if defined(Q_OS_WIN) || defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QTemporaryFile>
#include <QUuid>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

#include <vtk_hdf5.h>

#include "idoscommandmanager.h"
#include "providers/idosdatametadata.h"
#include "providers/idosdataregistry.h"

#include "idosproject.h"

IDOSProject::IDOSProject(QObject* parent)
    :
    QObject(parent)
    , m_commandManager(new IDOSCommandManager(this, this))
    , m_updateDepth(0)
    , m_pendingAdded()
    , m_pendingRemoved()
    , m_pendingDataChanged()
    , m_pendingVisibilityChanged()
{
}

IDOSProject::~IDOSProject()
{
}

const IDOSProjectMetadata& IDOSProject::metadata() const
{
    return m_metadata;
}

IDOSCommandManager* IDOSProject::commandManager() const
{
    return m_commandManager;
}

IDOSDataObject* IDOSProject::currentObject() const
{
    return m_currentObject.data();
}

void IDOSProject::setCurrentObject(IDOSDataObject* object)
{
    if (object != nullptr && m_objects.value(object->objectId(), nullptr) != object)
    {
        object = nullptr;
    }
    if (m_currentObject == object)
    {
        return;
    }
    m_currentObject = object;
    emit currentObjectChanged(object);
}

QString IDOSProject::currentCaseId() const
{
    return m_currentCaseId;
}

bool IDOSProject::setCurrentCaseId(const QString& caseId)
{
    if (!caseId.isEmpty() && !m_objects.contains(caseId))
    {
        return false;
    }
    if (m_currentCaseId != caseId)
    {
        m_currentCaseId = caseId;
        emit currentCaseChanged(caseId);
    }
    return true;
}

bool IDOSProject::setMetadata(const IDOSProjectMetadata& metadata)
{
    if (metadata.name().trimmed().isEmpty())
    {
        return false;
    }
    if (metadata.coordinateType() == IDOSProjectMetadata::CoordinateType::Custom && metadata.coordinateReference().trimmed().isEmpty())
    {
        return false;
    }
    m_metadata = metadata;
    m_metadata.setName(metadata.name().trimmed());
    m_metadata.setFieldBlock(metadata.fieldBlock().trimmed());
    m_metadata.setDescription(metadata.description().trimmed());
    m_metadata.setCoordinateReference(metadata.coordinateType() == IDOSProjectMetadata::CoordinateType::Custom
                                      ? metadata.coordinateReference().trimmed()
                                      : QString());
    m_metadata.setVerticalDatum(metadata.verticalDatum().trimmed());
    setModified(true);
    emit metadataChanged();
    return true;
}

// ===== 增删查 =====

void IDOSProject::add(IDOSDataObject* object)
{
    if (object == nullptr)
    {
        return;
    }

    const QString id = object->objectId();

    // 已存在同 id，先移除旧的
    IDOSDataObject* existing = m_objects.value(id, nullptr);
    if (existing != nullptr && existing != object)
    {
        if (m_currentCaseId == id)
        {
            setCurrentCaseId(QString());
        }
        if (m_currentObject == existing)
        {
            setCurrentObject(nullptr);
        }
        emit objectAboutToBeRemoved(id);
        disconnectObject(existing);
        m_objects.remove(id);
        existing->deleteLater();
    }

    object->setParent(this);
    m_objects.insert(id, object);
    connectObject(object);

    notifyAdded(id);
}

IDOSDataObject* IDOSProject::take(const QString& objectId)
{
    IDOSDataObject* object = m_objects.value(objectId, nullptr);
    if (object == nullptr)
    {
        return nullptr;
    }

    if (m_currentObject == object)
    {
        setCurrentObject(nullptr);
    }

    if (m_currentCaseId == objectId)
    {
        setCurrentCaseId(QString());
    }

    emit objectAboutToBeRemoved(objectId);
    disconnectObject(object);
    m_objects.remove(objectId);
    object->setParent(nullptr);

    notifyRemoved(objectId);
    return object;
}

bool IDOSProject::remove(const QString& objectId)
{
    IDOSDataObject* object = take(objectId);
    if (object == nullptr)
    {
        return false;
    }

    object->deleteLater();
    return true;
}

IDOSDataObject* IDOSProject::object(const QString& objectId) const
{
    return m_objects.value(objectId, nullptr);
}

IDOSDataObject* IDOSProject::objectByName(const QString& name) const
{
    for (IDOSDataObject* object : m_objects)
    {
        if (object->name() == name)
        {
            return object;
        }
    }
    return nullptr;
}

QList<IDOSDataObject*> IDOSProject::objects() const
{
    return m_objects.values();
}

// ===== 批量事务 =====

void IDOSProject::beginUpdate()
{
    ++m_updateDepth;
}

void IDOSProject::endUpdate()
{
    if (m_updateDepth <= 0)
    {
        return;
    }
    --m_updateDepth;
    if (m_updateDepth > 0)
    {
        return;
    }

    // 最外层事务闭合：按 removed → added → data changed → visibility changed 顺序发批量信号。
    // 集合在 notify* 中已做归并（同 id 先删后加最终算 added 等），列表天然唯一。
    if (!m_pendingRemoved.isEmpty())
    {
        emit objectsRemoved(m_pendingRemoved.values());
        m_pendingRemoved.clear();
    }
    if (!m_pendingAdded.isEmpty())
    {
        emit objectsAdded(m_pendingAdded.values());
        m_pendingAdded.clear();
    }
    if (!m_pendingDataChanged.isEmpty())
    {
        emit objectsDataChanged(m_pendingDataChanged.values());
        m_pendingDataChanged.clear();
    }
    if (!m_pendingVisibilityChanged.isEmpty())
    {
        emit objectsVisibilityChanged(m_pendingVisibilityChanged.values());
        m_pendingVisibilityChanged.clear();
    }
}

void IDOSProject::notifyAdded(const QString& objectId)
{
    setModified(true);
    if (m_updateDepth > 0)
    {
        // 删后又加（含同 id 替换）：最终状态为 added，清掉其他两类记录
        m_pendingRemoved.remove(objectId);
        m_pendingDataChanged.remove(objectId);
        m_pendingVisibilityChanged.remove(objectId);
        m_pendingAdded.insert(objectId);
        return;
    }
    emit objectAdded(objectId);
}

void IDOSProject::notifyRemoved(const QString& objectId)
{
    setModified(true);
    if (m_updateDepth > 0)
    {
        // 加了又删 / 改了又删：最终状态为 removed
        m_pendingAdded.remove(objectId);
        m_pendingDataChanged.remove(objectId);
        m_pendingVisibilityChanged.remove(objectId);
        m_pendingRemoved.insert(objectId);
        return;
    }
    emit objectRemoved(objectId);
}

void IDOSProject::notifyDataChanged(const QString& objectId)
{
    setModified(true);
    if (m_updateDepth > 0)
    {
        // 新对象在事务内的变化不重复记录（最终 added 已包含它）
        if (!m_pendingAdded.contains(objectId))
        {
            m_pendingDataChanged.insert(objectId);
        }
        return;
    }
    emit objectDataChanged(objectId);
}

// ===== 内部：信号转发 =====

void IDOSProject::connectObject(IDOSDataObject* object)
{
    connect(object, &IDOSDataObject::visibilityChanged, this, &IDOSProject::onObjectVisibilityChanged);
    connect(object, &IDOSDataObject::nameChanged, this, &IDOSProject::onObjectNameChanged);
    connect(object, &IDOSDataObject::dataChanged, this, &IDOSProject::onObjectDataChanged);
}

void IDOSProject::disconnectObject(IDOSDataObject* object)
{
    object->disconnect(this);
}

void IDOSProject::onObjectDataChanged()
{
    IDOSDataObject* obj = qobject_cast<IDOSDataObject*>(sender());
    if (obj != nullptr)
    {
        notifyDataChanged(obj->objectId());
    }
}

void IDOSProject::onObjectVisibilityChanged(bool visible)
{
    setModified(true);
    IDOSDataObject* obj = qobject_cast<IDOSDataObject*>(sender());
    if (obj != nullptr)
    {
        if (m_updateDepth > 0)
        {
            if (!m_pendingAdded.contains(obj->objectId()) &&
                !m_pendingRemoved.contains(obj->objectId()))
            {
                m_pendingVisibilityChanged.insert(obj->objectId());
            }
            return;
        }
        emit objectVisibilityChanged(obj->objectId(), visible);
    }
}

void IDOSProject::onObjectNameChanged(const QString& name)
{
    Q_UNUSED(name);
    IDOSDataObject* obj = qobject_cast<IDOSDataObject*>(sender());
    if (obj != nullptr)
    {
        notifyDataChanged(obj->objectId());
    }
}
QByteArray IDOSProject::storagePath(const QString& path)
{
    const QString nativePath = QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath());
#if defined(Q_OS_WIN)
    const DWORD size = GetShortPathNameW(reinterpret_cast<LPCWSTR>(nativePath.utf16()), nullptr, 0);
    if (size > 0)
    {
        QVector<wchar_t> buffer(static_cast<int>(size));
        const DWORD written = GetShortPathNameW(reinterpret_cast<LPCWSTR>(nativePath.utf16()), buffer.data(), size);
        if (written > 0 && written < size)
        {
            return QString::fromWCharArray(buffer.constData(), static_cast<int>(written)).toLocal8Bit();
        }
    }
#endif
    return nativePath.toLocal8Bit();
}

QString IDOSProject::filePath() const
{
    return m_filePath;
}

bool IDOSProject::isModified() const
{
    return m_modified;
}

void IDOSProject::setModified(bool modified)
{
    if (m_modified != modified)
    {
        m_modified = modified;
        emit modifiedChanged(modified);
    }
}

bool IDOSProject::save(const QString& path, const IDOSDataRegistry* registry, QString& error)
{
    error.clear();
    QElapsedTimer timing;
    timing.start();
    if (path.isEmpty() || registry == nullptr || m_updateDepth > 0)
    {
        error = tr("The project cannot be saved without a destination and data registry, or during an update.");
        return false;
    }
    QVariantList index;
    const QList<IDOSDataObject*> objects = this->objects();
    for (IDOSDataObject* object : objects)
    {
        if (registry->metadata(object->typeId()) == nullptr || !object->validateProjectReferences(this, error))
        {
            if (error.isEmpty())
            {
                error = tr("An object type is not registered: %1.").arg(object->typeId());
            }
            return false;
        }
        QVariantMap entry;
        entry.insert(QStringLiteral("id"), object->objectId());
        entry.insert(QStringLiteral("type"), object->typeId());
        entry.insert(QStringLiteral("name"), object->name());
        entry.insert(QStringLiteral("visible"), object->isVisible());
        index.append(entry);
    }
    QVariantMap header;
    header.insert(QStringLiteral("format"), QStringLiteral("IDOS"));
    header.insert(QStringLiteral("version"), 2);
    header.insert(QStringLiteral("metadata"), m_metadata.projectData());
    header.insert(QStringLiteral("currentCaseId"), m_currentCaseId);
    header.insert(QStringLiteral("currentObjectId"), m_currentObject != nullptr ? m_currentObject->objectId() : QString());
    header.insert(QStringLiteral("objects"), index);
    header.insert(QStringLiteral("presentation"), m_presentationState);
    const qint64 preparedMs = timing.elapsed();
    const QFileInfo destination(path);
    QTemporaryFile temporary(destination.absolutePath() + QStringLiteral("/.idos-XXXXXX.tmp"));
    if (!temporary.open())
    {
        error = tr("Cannot create a temporary project file: %1.").arg(temporary.errorString());
        return false;
    }
    const QString temporaryPath = temporary.fileName();
    temporary.close();
    hid_t file = -1;
    bool memoryFile = false;
    H5E_BEGIN_TRY
    {
        file = H5Fcreate(storagePath(temporaryPath).constData(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    }
    H5E_END_TRY
    if (file < 0)
    {
        const hid_t access = H5Pcreate(H5P_FILE_ACCESS);
        if (access >= 0 && H5Pset_fapl_core(access, 1024 * 1024, false) >= 0)
        {
            const QByteArray name = QByteArray("idos-") + QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
            file = H5Fcreate(name.constData(), H5F_ACC_TRUNC, H5P_DEFAULT, access);
            memoryFile = file >= 0;
        }
        if (access >= 0)
        {
            H5Pclose(access);
        }
        if (file < 0)
        {
            error = tr("Cannot create the HDF5 project file.");
            return false;
        }
    }
    bool success = IDOSDataMetadata::writeValue(file, "IDOS", header, error);
    const hid_t group = success ? H5Gcreate2(file, "Objects", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT) : -1;
    success = success && group >= 0;
    for (int position = 0; success && position < objects.size(); ++position)
    {
        IDOSDataObject* object = objects.at(position);
        const QByteArray name = QByteArray::number(position);
        const hid_t objectGroup = H5Gcreate2(group, name.constData(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        success = objectGroup >= 0;
        if (success)
        {
            success = registry->metadata(object->typeId())->writeProjectData(objectGroup, object, error);
            success = H5Gclose(objectGroup) >= 0 && success;
        }
    }
    if (group >= 0)
    {
        H5Gclose(group);
    }
    success = H5Fflush(file, H5F_SCOPE_GLOBAL) >= 0 && success;
    QByteArray image;
    if (success && memoryFile)
    {
        const ssize_t size = H5Fget_file_image(file, nullptr, 0);
        success = size > 0 && size <= INT_MAX;
        if (success)
        {
            image.resize(static_cast<int>(size));
            success = H5Fget_file_image(file, image.data(), image.size()) == size;
        }
    }
    success = H5Fclose(file) >= 0 && success;
    if (!success)
    {
        if (error.isEmpty())
        {
            error = tr("Cannot finish writing the project file.");
        }
        return false;
    }
    const qint64 writtenMs = timing.elapsed();
    // A completed on-disk HDF5 file is committed by same-directory replacement.
    if (!memoryFile)
    {
        if (!temporary.rename(temporaryPath + QStringLiteral(".commit")) ||
            !replaceStorageFile(temporary.fileName(), destination.absoluteFilePath()))
        {
            error = tr("Cannot replace the project file: %1.").arg(destination.absoluteFilePath());
            return false;
        }
        temporary.setAutoRemove(false);
    }
    else
    {
        QSaveFile output(destination.absoluteFilePath());
        output.setDirectWriteFallback(false);
        if (!output.open(QIODevice::WriteOnly))
        {
            error = tr("Cannot prepare the project file for atomic replacement.");
            return false;
        }
        if (output.write(image) != image.size())
        {
            output.cancelWriting();
            error = tr("Cannot write the complete project file.");
            return false;
        }
        if (!output.commit())
        {
            error = tr("Cannot replace the project file: %1.").arg(output.errorString());
            return false;
        }
    }
    qDebug().nospace() << "IDOS storage save: prepare=" << preparedMs << "ms write="
        << writtenMs - preparedMs << "ms commit=" << timing.elapsed() - writtenMs << "ms total=" << timing.elapsed() << "ms";
    m_filePath = destination.absoluteFilePath();
    m_commandManager->setClean();
    setModified(false);
    return true;
}

IDOSProject* IDOSProject::load(const QString& path, const IDOSDataRegistry* registry, QString& error, QObject* parent)
{
    error.clear();
    QElapsedTimer timing;
    timing.start();
    if (registry == nullptr)
    {
        error = tr("The data registry is unavailable.");
        return nullptr;
    }
    hid_t file = -1;
    QByteArray image;
    H5E_BEGIN_TRY
    {
        file = H5Fopen(storagePath(path).constData(), H5F_ACC_RDONLY, H5P_DEFAULT);
    }
    H5E_END_TRY
    if (file < 0)
    {
        QFile input(path);
        if (input.open(QIODevice::ReadOnly) && input.size() > 0 && input.size() <= INT_MAX)
        {
            image = input.readAll();
            if (image.size() == input.size() && input.error() == QFile::NoError)
            {
                H5E_BEGIN_TRY
                {
                    file = H5LTopen_file_image(image.data(), image.size(),
                        H5LT_FILE_IMAGE_DONT_COPY | H5LT_FILE_IMAGE_DONT_RELEASE);
                }
                H5E_END_TRY
            }
        }
        if (file < 0)
        {
            error = tr("Cannot open this file as an IDOS HDF5 project.");
            return nullptr;
        }
    }
    QVariant value;
    bool success = IDOSDataMetadata::readValue(file, "IDOS", value, error) && value.type() == QVariant::Map;
    const QVariantMap header = value.toMap();
    if (success && (header.value(QStringLiteral("format")).toString() != QStringLiteral("IDOS") ||
        header.value(QStringLiteral("version")).type() != QVariant::Int || (header.value(QStringLiteral("version")).toInt() != 1 && header.value(QStringLiteral("version")).toInt() != 2)))
    {
        error = tr("This project format or version is not supported.");
        success = false;
    }
    IDOSProject* project = new IDOSProject();
    project->beginUpdate();
    IDOSProjectMetadata metadata;
    success = success && header.value(QStringLiteral("objects")).type() == QVariant::List &&
        metadata.restoreProjectData(header.value(QStringLiteral("metadata")).toMap(), error);
    if (success)
    {
        // Empty metadata names are permitted for programmatically created projects.
        project->m_metadata = metadata;
    }
    hid_t group = -1;
    H5L_info_t groupLink;
    H5E_BEGIN_TRY
    {
        if (success && H5Lget_info(file, "Objects", &groupLink, H5P_DEFAULT) >= 0 && groupLink.type == H5L_TYPE_HARD)
        {
            group = H5Gopen2(file, "Objects", H5P_DEFAULT);
        }
    }
    H5E_END_TRY
    success = success && group >= 0;
    const qint64 headerMs = timing.elapsed();
    const QVariantList index = header.value(QStringLiteral("objects")).toList();
    H5G_info_t info;
    success = success && H5Gget_info(group, &info) >= 0 && info.nlinks == static_cast<hsize_t>(index.size());
    for (int position = 0; success && position < index.size(); ++position)
    {
        const QVariantMap entry = index.at(position).toMap();
        const QString id = entry.value(QStringLiteral("id")).toString();
        const QString type = entry.value(QStringLiteral("type")).toString();
        const IDOSDataMetadata* dataMetadata = registry->metadata(type);
        if (id.trimmed().isEmpty() || project->object(id) != nullptr || dataMetadata == nullptr ||
            entry.value(QStringLiteral("name")).type() != QVariant::String || entry.value(QStringLiteral("visible")).type() != QVariant::Bool)
        {
            error = tr("The project contains duplicate identities, invalid object information or an unregistered type: %1.").arg(type);
            success = false;
            break;
        }
        const QByteArray name = QByteArray::number(position);
        hid_t objectGroup = -1;
        H5E_BEGIN_TRY
        {
            if (H5Lget_info(group, name.constData(), &groupLink, H5P_DEFAULT) >= 0 && groupLink.type == H5L_TYPE_HARD)
            {
                objectGroup = H5Gopen2(group, name.constData(), H5P_DEFAULT);
            }
        }
        H5E_END_TRY
        IDOSDataObject* object = objectGroup >= 0 ? dataMetadata->readProjectData(objectGroup, error) : nullptr;
        if (objectGroup >= 0)
        {
            H5Gclose(objectGroup);
        }
        if (object == nullptr)
        {
            success = false;
            break;
        }
        object->setObjectId(id);
        object->setName(entry.value(QStringLiteral("name")).toString());
        object->setVisible(entry.value(QStringLiteral("visible")).toBool());
        project->add(object);
    }
    if (group >= 0)
    {
        H5Gclose(group);
    }
    project->endUpdate();
    success = H5Fclose(file) >= 0 && success;
    const qint64 restoredMs = timing.elapsed();
    if (success)
    {
        for (IDOSDataObject* object : project->objects())
        {
            if (!object->validateProjectReferences(project, error))
            {
                success = false;
                break;
            }
        }
    }
    const QString caseId = header.value(QStringLiteral("currentCaseId")).toString();
    const QString objectId = header.value(QStringLiteral("currentObjectId")).toString();
    if (success && ((!caseId.isEmpty() && project->object(caseId) == nullptr) ||
        (!objectId.isEmpty() && project->object(objectId) == nullptr)))
    {
        error = tr("The project selection refers to a missing object.");
        success = false;
    }
    if (!success)
    {
        if (error.isEmpty())
        {
            error = tr("The project file is incomplete or invalid.");
        }
        delete project;
        return nullptr;
    }
    project->m_presentationState = header.value(QStringLiteral("presentation")).toMap();
    project->m_currentCaseId = caseId;
    project->m_currentObject = project->object(objectId);
    project->m_filePath = QFileInfo(path).absoluteFilePath();
    project->setModified(false);
    project->m_commandManager->setClean();
    project->setParent(parent);
    qDebug().nospace() << "IDOS storage load: header=" << headerMs << "ms restore="
        << restoredMs - headerMs << "ms validate=" << timing.elapsed() - restoredMs << "ms total=" << timing.elapsed() << "ms";
    return project;
}
bool IDOSProject::replaceStorageFile(const QString& temporary, const QString& destination)
{
#if defined(Q_OS_WIN)
    const QString sourcePath = QDir::toNativeSeparators(QFileInfo(temporary).absoluteFilePath());
    const QString destinationPath = QDir::toNativeSeparators(QFileInfo(destination).absoluteFilePath());
    return MoveFileExW(reinterpret_cast<LPCWSTR>(sourcePath.utf16()),
        reinterpret_cast<LPCWSTR>(destinationPath.utf16()), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return std::rename(QFile::encodeName(temporary).constData(), QFile::encodeName(destination).constData()) == 0;
#endif
}

QVariantMap IDOSProject::presentationState() const
{
    return m_presentationState;
}

void IDOSProject::setPresentationState(const QVariantMap& state)
{
    if (m_presentationState != state)
    {
        m_presentationState = state;
        setModified(true);
    }
}

void IDOSProject::markPresentationModified()
{
    setModified(true);
}
