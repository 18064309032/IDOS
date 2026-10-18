#ifndef IDOS_FLUID_ROCK_TABLE_OBJECT_H
#define IDOS_FLUID_ROCK_TABLE_OBJECT_H

#include <QString>
#include <QVariantMap>

#include "idosdataobject.h"

class CORE_EXPORT IDOSFluidRockTableObject : public IDOSDataObject
{
    Q_OBJECT

  public:
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    enum class Category
    {
        Fluid,
        RockPhysics
    };

    explicit IDOSFluidRockTableObject(Category category = Category::Fluid, QObject* parent = nullptr);
    ~IDOSFluidRockTableObject() override;

    QString typeId() const override;
    bool canClone() const override;
    IDOSDataObject* clone() const override;
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
