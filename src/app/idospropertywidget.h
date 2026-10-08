#ifndef IDOS_PROPERTY_WIDGET_H
#define IDOS_PROPERTY_WIDGET_H

#include <QPointer>
#include <QString>
#include <QWidget>

#include "idos_app.h"
#include "idoswellhead.h"
#include "idoswelllogchannel.h"
#include "idoswelllogset.h"
#include "idoswellpath.h"

class IDOSWell;
class IDOSWellLogSet;
class IDOSWellPath;
class QFormLayout;

/**
 * @brief Displays properties for the current selection.
 *
 * The first implementation supports wells and well data parts. The widget
 * observes the selected QObject and refreshes when its business data changes.
 */
class APP_EXPORT IDOSPropertyWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSPropertyWidget(QWidget* parent = nullptr);

    void setWell(IDOSWell* well);
    void setWellPart(const QString& partKey, const QString& itemKey);
    void clear();

  private slots:
    void onWellDestroyed();
    void onWellHeadChanged(const IDOSWellHead& head);
    void onWellPathChanged(const IDOSWellPath& path);
    void onWellLogsChanged(const IDOSWellLogSet& logs);
    void onWellChanged();

  private:
    void addProperty(const QString& name, const QString& value);
    void clearProperties();
    void refresh();
    void showWellProperties();
    void showTrajectoryProperties();
    void showLogProperties();
    QString wellTypeText() const;
    QString depthRangeText() const;
    QString channelDepthRangeText(const IDOSWellLogChannel& channel) const;

    QPointer<IDOSWell> m_well;
    QFormLayout* m_formLayout;
    QString m_partKey;
    QString m_itemKey;
};

#endif // IDOS_PROPERTY_WIDGET_H
