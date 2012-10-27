#include "chasewidget.h"

#include <QtCore/QPoint>

#include <QtGui/QApplication>
#include <QtGui/QHideEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QShowEvent>

ChaseWidget::ChaseWidget( QWidget *parent, QPixmap pixmap, bool pixmapEnabled ) : QWidget( parent )
    , m_segment( 0 )
    , m_delay( 100 )
    , m_step( 40 )
    , m_timerId( -1 )
    , m_animated( false )
    , m_pixmap( pixmap )
    , m_pixmapEnabled( pixmapEnabled )
{
}

void ChaseWidget::toggleOperation( int id, bool value )
{
    if( value && !m_operations.contains( id ) )
        addOperation( id );
    else if( !value && m_operations.contains( id ) )
        removeOperation( id );
}

void ChaseWidget::addOperation( int id )
{
    if( !m_operations.contains( id ) )
        m_operations.push_back( id );
    if( m_operations.size() != 0 && !m_animated )
        setAnimated( true );
}

void ChaseWidget::removeOperation( int id )
{
    if( !m_operations.contains( id ) )
        return;

    int index = m_operations.indexOf( id );
    if( index != -1 ) {
        m_operations.remove( index );
    }

    if( m_operations.size() == 0 )
        setAnimated( false );
}

void ChaseWidget::setAnimated( bool value )
{
    if ( m_animated == value )
        return;
    m_animated = value;
    if( m_timerId != -1 ) {
        killTimer( m_timerId );
        m_timerId = -1;
    }
    if( m_animated ) {
        m_segment = 0;
        m_timerId = startTimer( m_delay );
    }
    update();
}

void ChaseWidget::paintEvent( QPaintEvent *event )
{
    Q_UNUSED( event );
    QPainter p( this );
    if ( m_pixmapEnabled && !m_pixmap.isNull() ) {
        p.drawPixmap( 0, 0, m_pixmap );
        return;
    }

    const int extent = qMin( width(), height() );
    const int displ = extent / 4;
    const int ext = extent / 4 - 1;

    p.setRenderHint( QPainter::Antialiasing, true );

    if( m_animated )
        p.setPen( Qt::gray );
    else
        p.setPen( QPen( palette().dark().color() ) );

    p.translate( width() / 2, height() / 2 ); // center

    for ( int segment = 0; segment < segmentCount(); ++segment ) {
        p.rotate( QApplication::isRightToLeft() ? m_step : -m_step );
        if(m_animated)
            p.setBrush( colorForSegment( segment ) );
        else
            p.setBrush( palette().background() );
        p.drawEllipse( QRect( displ, -ext / 2, ext, ext ) );
    }
}

QSize ChaseWidget::sizeHint( void ) const
{
    return QSize(32, 32);
}

void ChaseWidget::timerEvent( QTimerEvent *event )
{
    if ( event->timerId() == m_timerId ) {
        ++m_segment;
        update();
    }
    QWidget::timerEvent(event);
}

QColor ChaseWidget::colorForSegment( int seg ) const
{
    int index = ( ( seg + m_segment ) % segmentCount() );
    int comp = qMax( 0, 255 - ( index * ( 255 / segmentCount() ) ) );
    return QColor( comp, comp, comp, 255 );
}

int ChaseWidget::segmentCount( void ) const
{
    return 360 / m_step;
}

void ChaseWidget::setPixmapEnabled( bool enable )
{
    m_pixmapEnabled = enable;
}

