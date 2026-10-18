#ifndef IDOS_FRACTURE_MATERIAL_CATALOG_OBJECT_H
#define IDOS_FRACTURE_MATERIAL_CATALOG_OBJECT_H

#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "idosdataobject.h"

class CORE_EXPORT IDOSFractureMaterialCatalogObject : public IDOSDataObject
{
    Q_OBJECT

  public:
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    enum class Kind
    {
        Fluid,
        Proppant
    };

    explicit IDOSFractureMaterialCatalogObject(QObject* parent = nullptr);

    QString typeId() const override;
    bool canClone() const override;
    IDOSDataObject* clone() const override;
    Kind kind() const;
    void setKind(Kind kind);
    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);
    QStringList columns() const;
    void setColumns(const QStringList& columns);
    QVariantList records() const;
    void setRecords(const QVariantList& records);

  private:
    Kind m_kind;
    QString m_sourceFile;
    QStringList m_columns;
    QVariantList m_records;
};

#endif // IDOS_FRACTURE_MATERIAL_CATALOG_OBJECT_H
