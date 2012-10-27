#include <QtGui/QDrag>
#include "spriteview.h"

void SpriteView::startDrag( Qt::DropActions supportedActions )
{
    QModelIndexList indexes = selectedIndexes();
    if( indexes.count() > 0 ) {
        QMimeData *data = model()->mimeData( indexes );
        if( !data)
            return;

        QPixmap pixmap = QPixmap::fromImage( indexes.first().data( Qt::DecorationRole ).value<QImage>() );
        QDrag *drag = new QDrag( this );
        drag->setPixmap( pixmap );
        drag->setMimeData( data );
        drag->setHotSpot( QPoint( pixmap.width() / 2, pixmap.height() / 2) );
        Qt::DropAction defaultDropAction = Qt::IgnoreAction;
        if( QAbstractItemView::defaultDropAction() != Qt::IgnoreAction && ( supportedActions & QAbstractItemView::defaultDropAction() ) )
            defaultDropAction = QAbstractItemView::defaultDropAction();
        else if( supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove )
            defaultDropAction = Qt::CopyAction;
        drag->exec( supportedActions, defaultDropAction );
    }
}
