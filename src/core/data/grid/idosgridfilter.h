#ifndef IDOS_GRID_FILTER_H
#define IDOS_GRID_FILTER_H

#include <QMap>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

#include "idosdataobject.h"

class IDOSGridProperty;

class CORE_EXPORT IDOSGridFilter : public IDOSDataObject
{
    Q_OBJECT
  public:
    bool validateProjectReferences(const IDOSProject* project, QString& error) const override;
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    explicit IDOSGridFilter(QObject* parent = nullptr);
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
                                    const QMap<QString, IDOSGridProperty*>& properties);
    static bool matches(const QVariantMap& criteria, int i, int j, int k, int globalIndex,
                        const QMap<QString, IDOSGridProperty*>& properties);
  private:
    static bool propertyMatches(int comparison, double value, double minimum, double maximum,
                                double epsilon, bool includeMinimum, bool includeMaximum, bool includeInvalid);
    QString m_gridId;
    QVariantMap m_criteria;
};

#endif
