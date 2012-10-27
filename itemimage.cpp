#include <QtGui/QPainter>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QApplication>

#include "itemimage.h"
#include "tibiahandler.h"

ItemImage::ItemImage( QWidget *parent ) : QFrame( parent )
{
    m_shownumbers = false;
    m_droptype = QString();
    m_image = NULL;
    m_acceptFiles = false;
    setAcceptDrops( true );

    setImage( QImage() );
}

ItemImage::ItemImage( QWidget *parent, const QImage& img ) : QFrame( parent )
{
    m_shownumbers = false;
    m_droptype = QString();
    m_image = NULL;
    m_acceptFiles = false;
    setAcceptDrops( true );

    setImage( img );
}

ItemImage::ItemImage( const QImage& img ) : QFrame( NULL )
{
    m_shownumbers = false;
    m_droptype = QString();
    m_image = NULL;
    m_acceptFiles = false;
    setAcceptDrops( true );

    setImage( img );
}

ItemImage::~ItemImage()
{
    if( m_image ) {
        delete m_image;
        m_image = NULL;
    }
}

void ItemImage::setMimeDropType( const QString& type )
{
    QMutexLocker locker( &mutex );
    m_droptype = type;
}

const QString& ItemImage::getMimeDropType( void ) const
{
    QMutexLocker locker( &mutex );
    return m_droptype;
}

void ItemImage::setColor( const QColor& color )
{
    QMutexLocker locker( &mutex );
    m_color = color;
    update();
}

void ItemImage::showNumbers( bool show )
{
    QMutexLocker locker( &mutex );
    m_shownumbers = show;
    update();
}

void ItemImage::setAcceptFiles( bool accept )
{
    m_acceptFiles = accept;
    update();
}

void ItemImage::clearContents( void )
{
    clearRectangles();

    if( m_image ) {
        delete m_image;
        m_image = NULL;
    }

    update();
}

void ItemImage::clearRectangles( void )
{
    m_highlighted = QRect();
    for( QMap<QPoint *, QRect>::iterator it = m_pieces.begin(); it != m_pieces.end(); it++ )
        delete it.key();

    m_pieces.clear();
    m_indexes.clear();
}

void ItemImage::setImage( const QImage& img )
{
    QMutexLocker locker( &mutex );

    clearContents();
    m_image = new QImage( img );
    if( m_image->width() < 32 && m_image->height() < 32 ) {
        setMinimumSize( QSize( 32, 32 ) );
        setMaximumSize( QSize( 32, 32 ) );
    } else if( m_image->width() < 32 && m_image->height() >= 32 ) {
        setMinimumSize( QSize( 32, m_image->size().height() ) );
        setMaximumSize( QSize( 32, m_image->size().height() ) );
    } else if( m_image->width() >= 32 && m_image->height() < 32 ) {
        setMinimumSize( QSize( m_image->size().width(), 32 ) );
        setMaximumSize( QSize( m_image->size().width(), 32 ) );
    } else {
        setMinimumSize( m_image->size() );
        setMaximumSize( m_image->size() );
    }

    emit imageChanged( img );

    updatePieces();
    update();
}

const QImage ItemImage::getImage( void )
{
    if( m_image )
        return *m_image;

    return QImage();
}

void ItemImage::clear( void )
{
    clearContents();
    setMinimumSize( QSize( 32, 32 ) );
    setMaximumSize( QSize( 32, 32 ) );
}

void ItemImage::paintEvent( QPaintEvent *event )
{
    QFrame::paintEvent( event );

    QPainter painter( this );
    painter.setRenderHint( QPainter::HighQualityAntialiasing );
    if( m_image && !m_shownumbers )
        painter.drawImage( this->rect(), *m_image );
    if( m_shownumbers && !m_pieces.isEmpty() ) {
        QFont font( ItemImage::font() );
        font.setBold( true );
        font.setPointSize( 10 );
        painter.setFont( font );
        for( QVector<QRect>::const_iterator it = m_indexes.begin(); it != m_indexes.end(); it++ )
            painter.drawText( *it, Qt::AlignCenter, QString::number( m_indexes.indexOf( *it ) ) );
    }
    if( m_highlighted.isValid() ) {
        if( m_color.isValid() )
            drawGradientRectangle( painter, m_highlighted, m_color );
        else
            drawGradientRectangle( painter, m_highlighted, QApplication::palette().color( QPalette::Highlight ) );
    }
}

void ItemImage::drawGradientRectangle( QPainter& painter, QRect& rect, QColor baseColor )
{
    QColor baseLighter = baseColor.lighter( 175 );
    QColor gradientStart = QColor( baseLighter.red(), baseLighter.green(), baseLighter.blue(), 150 );
    QColor gradientEnd = QColor( baseColor.red(), baseColor.green(), baseColor.blue(), 150 );
    QLinearGradient linearGrad( QPointF( 0, 0 ), QPointF( 0, 1 ) );
    linearGrad.setColorAt( 0, gradientStart );
    linearGrad.setColorAt( 1, gradientEnd );
    linearGrad.setCoordinateMode( QGradient::ObjectBoundingMode );
    painter.setBrush( linearGrad );
    painter.setPen( QPen( baseColor ) );
    painter.drawRoundedRect( rect, 2.7, 2.7 );
    /*painter.drawRect( rect );
    painter.setBrush( QColor( 0x00, 0x00, 0xFF, 100 ) );
    painter.setPen( QPen( QColor(0x00, 0x00, 0xFF ) ) );
    painter.drawRect( rect );*/
}

void ItemImage::updatePieces( void )
{
    clearRectangles();

    if( m_image ) {
        quint8 width = m_image->rect().width() / 32;
        quint8 height = m_image->rect().height() / 32;

        if( width > 0 && height > 0 ) {
            qint8 real_x = 0;
            qint8 real_y = 0;
            for( qint8 offset_y = height - 1; offset_y >= 0; offset_y-- ) { // Reverse image chunking to produce proper points
                real_x = 0;
                for( qint8 offset_x = width - 1; offset_x >= 0; offset_x-- ) {
                    QPoint *coordinate = new QPoint( real_x, real_y );
                    m_pieces.insert( coordinate, QRect( m_image->rect().topLeft().x() + offset_x * 32, m_image->rect().topLeft().y() + offset_y * 32, 32, 32 ).adjusted( 0, 0, -1, -1 ) );
                    real_x++;
                }
                real_y++;
            }

            for( qint8 fy = 0; fy < height; fy++ ) {
                for( qint8 fx = 0; fx < width; fx++ ) { // Forward image chunking to update numbers
                    QRect rect( m_image->rect().topLeft().x() + fx * 32, m_image->rect().topLeft().y() + fy * 32, 32, 32 );
                    m_indexes.push_back( rect );
                }
            }
        }
    }
}

void ItemImage::dragEnterEvent( QDragEnterEvent *event )
{
    if ( !m_droptype.isEmpty() && event->mimeData()->hasFormat( m_droptype ) )
        event->accept();
    else if ( event->mimeData()->hasFormat( "text/uri-list" ) && m_acceptFiles )
        event->accept();
    else
        event->ignore();
}

void ItemImage::dragLeaveEvent( QDragLeaveEvent *event )
{
    m_highlighted = QRect();
    update();
    event->accept();
}

void ItemImage::dragMoveEvent( QDragMoveEvent *event )
{
    if ( !m_droptype.isEmpty() && event->mimeData()->hasFormat( m_droptype ) ) {
        bool inRect = false;
        QRect rect;
        foreach( rect, m_pieces ) {
            if( inRect = rect.contains( event->pos() ) )
                break;
        }
        if( inRect ) {
            m_highlighted = rect;
            event->setDropAction( Qt::CopyAction );
            event->accept();
        } else {
            m_highlighted = QRect();
            event->ignore();
        }

        update( m_image->rect() );
    }
    if( m_acceptFiles && event->mimeData()->hasFormat( "text/uri-list" ) ) {
        if( this->rect().contains( event->pos() ) ) {
            m_highlighted = this->rect().adjusted( 0, 0, -1, -1 );
            event->setDropAction( Qt::CopyAction );
            event->accept();
            update( m_image->rect() );
        }
    }
}

void ItemImage::dropEvent( QDropEvent *event )
{
    if ( !m_droptype.isEmpty() && event->mimeData()->hasFormat( m_droptype ) ) {
        if( m_highlighted.isValid() ) {
            QByteArray pieceData = event->mimeData()->data( m_droptype );
            QDataStream stream( &pieceData, QIODevice::ReadOnly );
            qint32 size;
            stream >> size;
            if( size > 0 ) {
                SharedResource resource;
                stream >> resource;

                QPoint *point = m_pieces.key( m_highlighted );
                emit droppedSprite( resource, point->x(), point->y() );
            }
        }
    }
    if( m_acceptFiles && event->mimeData()->hasUrls() ) {
        emit droppedFile( event->mimeData()->urls().last().toLocalFile() );
    }

    m_highlighted = QRect();
    update( m_image->rect() );
}
