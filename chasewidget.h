#ifndef CHASEWIDGET_H
#define CHASEWIDGET_H

#include <QtGui/QWidget>

#include <QtCore/QSize>
#include <QtGui/QColor>
#include <QtGui/QPixmap>

QT_BEGIN_NAMESPACE
class QHideEvent;
class QShowEvent;
class QPaintEvent;
class QTimerEvent;
QT_END_NAMESPACE

class ChaseWidget : public QWidget
{
    Q_OBJECT
public:
    ChaseWidget( QWidget *parent = 0, QPixmap pixmap = QPixmap(), bool pixmapEnabled = false );

    void toggleOperation( int, bool );
    void addOperation( int );
    void removeOperation( int );
    void setAnimated( bool value );
    void setPixmapEnabled( bool enable );
    QSize sizeHint( void ) const;

protected:
    void paintEvent( QPaintEvent *event );
    void timerEvent( QTimerEvent *event );

private:
    int segmentCount( void ) const;
    QColor colorForSegment( int segment ) const;

    QVector<int> m_operations;
    int m_segment;
    int m_delay;
    int m_step;
    int m_timerId;
    bool m_animated;
    QPixmap m_pixmap;
    bool m_pixmapEnabled;
};

#endif
