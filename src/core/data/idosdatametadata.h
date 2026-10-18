#ifndef IDOS_DATA_METADATA_H
#define IDOS_DATA_METADATA_H

#include <QList>
#include <QString>
#include <QVariant>

#include "idos_core.h"

class IDOSDataObject;
class QDataStream;

class CORE_EXPORT IDOSDataMetadata
{
  public:
    IDOSDataMetadata();
    virtual ~IDOSDataMetadata();

    virtual QString typeId() const = 0;
    virtual QString displayName() const = 0;
    virtual IDOSDataObject* create() const = 0;
    virtual bool writeProjectData(qint64 group, const IDOSDataObject* object, QString& error) const;
    virtual IDOSDataObject* readProjectData(qint64 group, QString& error) const;
    static bool writeValue(qint64 parent, const QByteArray& name, const QVariant& value, QString& error);
    static bool readValue(qint64 parent, const QByteArray& name, QVariant& value, QString& error);

  private:
    static bool writePacked(qint64 parent, const QByteArray& name, const QVariant& value, QString& error);
    static bool readPacked(qint64 group, QVariant& value, int depth, quint64& nodes, QString& error);
    static bool encodePacked(QDataStream& stream, const QVariant& value, QList<QVariant>& arrays, int depth, QString& error);
    static bool decodePacked(QDataStream& stream, qint64 group, QVariant& value, int depth, int& arrayIndex, quint64& nodes, QString& error);
    static bool isArray(const QVariant& value);
    static bool isScalar(const QVariant& value);
    static QVariantMap tableColumns(const QVariantList& rows);
    static int columnSize(const QVariant& column);
    static QVariant columnValue(const QVariant& column, int index);
    static bool writeNode(qint64 parent, const QByteArray& name, const QVariant& value, int depth, QString& error);
    static bool readNode(qint64 parent, const QByteArray& name, QVariant& value, int depth, quint64& nodes, QString& error);
    static bool writeArray(qint64 group, qint64 fileType, qint64 memoryType, const void* data, quint64 count);
    static bool readArray(qint64 group, qint64 fileType, qint64 memoryType, void* data, quint64 count);
    static qint64 arraySize(qint64 group);

};

#endif
