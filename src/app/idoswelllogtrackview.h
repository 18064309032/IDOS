#ifndef IDOS_WELL_LOG_TRACK_VIEW_H
#define IDOS_WELL_LOG_TRACK_VIEW_H

#include <QList>
#include <QPoint>
#include <QPointer>
#include <QString>
#include <QWidget>

#include "idos_app.h"
#include "idoswelllogchannel.h"

class QColor;
class IDOSWell;
class IDOSWellLogSet;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QToolButton;
class QWheelEvent;

/**
 * @brief Default two-dimensional well log track view.
 *
 * Displays the imported curves of one well with depth increasing downward.
 * The view owns no well data and refreshes when the source log set changes.
 */
class APP_EXPORT IDOSWellLogTrackView : public QWidget
{
    Q_OBJECT

  public:
    explicit IDOSWellLogTrackView(QWidget* parent = nullptr);

    void setWell(IDOSWell* well);
    void setSelectedChannelName(const QString& channelName);

  private slots:
    void onLogsChanged(const IDOSWellLogSet& logs);
    void onResetDepthRange();

  protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

  private:
    QList<IDOSWellLogChannel> displayedChannels() const;
    QColor colorForChannel(const QString& channelName) const;
    bool depthRange(const QList<IDOSWellLogChannel>& channels,
                    double* minimumDepth,
                    double* maximumDepth) const;
    bool valueRange(const IDOSWellLogChannel& channel,
                    double* minimumValue,
                    double* maximumValue) const;
    bool visibleDepthRange(const QList<IDOSWellLogChannel>& channels,
                           double* minimumDepth,
                           double* maximumDepth) const;
    void clampVisibleDepthRange(double dataMinimumDepth, double dataMaximumDepth);
    void resetDepthRange();
    void updateResetButtonGeometry();
    double depthToY(double depth, double minimumDepth, double maximumDepth,
                    double top, double height) const;
    double valueToX(double value, double minimumValue, double maximumValue,
                    double left, double width) const;
    double yToDepth(double y, double minimumDepth, double maximumDepth,
                    double top, double height) const;

    QPointer<IDOSWell> m_well;
    QString m_selectedChannelName;
    QToolButton* m_resetDepthRangeButton;
    QPoint m_dragStartPosition;
    double m_dragStartMinimumDepth;
    double m_dragStartMaximumDepth;
    double m_visibleMinimumDepth;
    double m_visibleMaximumDepth;
    bool m_hasVisibleDepthRange;
    bool m_isDragging;
};

#endif // IDOS_WELL_LOG_TRACK_VIEW_H
