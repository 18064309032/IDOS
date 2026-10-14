#ifndef IDOS_FLUID_ROCK_TABLE_H
#define IDOS_FLUID_ROCK_TABLE_H

#include <QString>
#include <QVariantMap>

#include "idosdataobject.h"

class CORE_EXPORT IDOSFluidRockTable : public IDOSDataObject
{
    Q_OBJECT

  public:
    enum class Category
    {
        Fluid,
        RockPhysics
    };

    explicit IDOSFluidRockTable(Category category = Category::Fluid, QObject* parent = nullptr);
    ~IDOSFluidRockTable() override;

    QString typeId() const override;
    Category category() const;
    QVariantMap tableData() const;
    void setTableData(const QVariantMap& data);
    QString sourceFile() const;
    void setSourceFile(const QString& path);

  private:
    Category m_category;
    QVariantMap m_tableData;
    QString m_sourceFile;
};

#endif
