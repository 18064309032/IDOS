#ifndef IDOS_MICROSEISMIC_SET_OBJECT_H
#define IDOS_MICROSEISMIC_SET_OBJECT_H

#include <QStringList>
#include <QVariantList>
#include <QVariantMap>

#include "idosdataobject.h"

class CORE_EXPORT IDOSMicroseismicSetObject : public IDOSDataObject
{
    Q_OBJECT

  public:
    QVariantMap projectData() const override;
    bool restoreProjectData(const QVariantMap& data, QString& error) override;

    explicit IDOSMicroseismicSetObject(QObject* parent = nullptr);

    QString typeId() const override;
    bool canClone() const override;
    IDOSDataObject* clone() const override;
    QString sourceFile() const;
    void setSourceFile(const QString& sourceFile);
    QVariantList events() const;
    void setEvents(const QVariantList& events);

  private:
    QString m_sourceFile;
    QVariantList m_events;
};

#endif // IDOS_MICROSEISMIC_SET_OBJECT_H
