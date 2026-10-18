#ifndef IDOS_GRID_FILTER_OBJECT_H
#define IDOS_GRID_FILTER_OBJECT_H

#include <QMap>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

#include "idosdataobject.h"

class IDOSGridPropertyObject;

class CORE_EXPORT IDOSGridFilterObject : public IDOSDataObject
{
    Q_OBJECT
  public:
    bool validateProjectReferences(const IDOSProject* project, QString& error) const override;
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    explicit IDOSGridFilterObject(QObject* parent = nullptr);
    QString typeId() const override;
    bool canClone() const override;
    IDOSDataObject* clone() const override;
    QString containerId() const override;
    QString gridId() const;
    void setGridId(const QString& id);
    QVariantMap criteria() const;
    bool setCriteria(const QVariantMap& criteria);
    static bool validateCriteria(const QVariantMap& criteria, QString& error);
    static QStringList propertyIds(const QVariantMap& criteria);
    static QVector<quint8> selection(const QVariantMap& criteria, int nx, int ny, int nz,
                                    const QMap<QString, IDOSGridPropertyObject*>& properties);
    static bool matches(const QVariantMap& criteria, int i, int j, int k, int globalIndex,
                        const QMap<QString, IDOSGridPropertyObject*>& properties);
  private:
    static bool propertyMatches(int comparison, double value, double minimum, double maximum,
                                double epsilon, bool includeMinimum, bool includeMaximum, bool includeInvalid);
    QString m_gridId;
    QVariantMap m_criteria;
};

#endif
