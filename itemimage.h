#ifndef ITEMIMAGE_H
#define ITEMIMAGE_H

#include <QtGui/QImage>
#include <QtGui/QFrame>
#include <QtCore/QMutexLocker>
#include <QtCore/QMap>
#include <QtCore/QUrl>

#include "tibiaresource.h"

class QDropEvent;
class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QPaintEvent;
class QDropEvent;

class ItemImage : public QFrame
{
    Q_OBJECT

public:
    ItemImage( QWidget *parent );
    ItemImage( QWidget *parent, const QImage& img );
    ItemImage( const QImage& img = QImage() );
    ~ItemImage( void );

    void setMimeDropType( const QString& type );
    const QString& getMimeDropType( void ) const;
    void setColor( const QColor& color );

    const QImage *image( void ) const;
    const QImage getImage( void );

    void clear( void );

signals:
    void imageChanged( const QImage& );
    void droppedSprite( SharedResource&, quint8, quint8 ); // Sprite resource dropped | #Rect dropped onto
    void droppedFile( const QString& );

public slots:
    void setImage( const QImage& );
    void showNumbers( bool );
    void setAcceptFiles( bool );

private:
    void drawGradientRectangle( QPainter& painter, QRect& rect, QColor baseColor );
    void updatePieces( void );
    void clearRectangles( void );
    void clearContents( void );
    mutable QMutex mutex;

protected:
    virtual void dragEnterEvent( QDragEnterEvent * );
    virtual void dragLeaveEvent( QDragLeaveEvent * );
    virtual void dragMoveEvent( QDragMoveEvent * );
    virtual void dropEvent( QDropEvent * );
    virtual void paintEvent( QPaintEvent * );

    QFont m_font;
    QColor m_color;
    QRect m_highlighted;
    QMap<QPoint *, QRect> m_pieces;
    QVector<QRect> m_indexes;
    QString m_droptype;
    bool m_shownumbers;
    bool m_acceptFiles;
    QImage *m_image;
};

#endif // ITEMIMAGE_H
