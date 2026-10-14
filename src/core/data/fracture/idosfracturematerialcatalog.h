#ifndef IDOS_FRACTURE_MATERIAL_CATALOG_H
#define IDOS_FRACTURE_MATERIAL_CATALOG_H

#include <QStringList>
#include <QVariantList>

#include "idosdataobject.h"

class CORE_EXPORT IDOSFractureMaterialCatalog : public IDOSDataObject
{
    Q_OBJECT

  public:
    enum class Kind
    {
        Fluid,
        Proppant
    };

    explicit IDOSFractureMaterialCatalog(QObject* parent = nullptr);

    QString typeId() const override;
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

#endif // IDOS_FRACTURE_MATERIAL_CATALOG_H
