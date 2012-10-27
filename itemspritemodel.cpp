#include <QtCore/QMimeData>
#include "itemspritemodel.h"
#include "resourcehandler.h"
#include "tibiahandler.h"

extern ResourceHandler g_resourceHandler;
extern TibiaHandler g_tibiaHandler;

ItemSpriteModel::ItemSpriteModel( QObject *parent, ItemData *itemData ) : QAbstractListModel( parent ),
    m_itemData( itemData ),
    m_dropActions( Qt::CopyAction ),
    m_itemFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled )
{

}

void ItemSpriteModel::setDragMimeFormat( const QString& mimeType )
{
    m_dragMimeType = mimeType;
}

void ItemSpriteModel::setDropMimeFormats( const QStringList& mimeTypes )
{
    m_dropMimeTypes = mimeTypes;
}

void ItemSpriteModel::setSupportedDropActions( Qt::DropActions actions )
{
    m_dropActions = actions;
}

void ItemSpriteModel::setItemFlags( Qt::ItemFlags flags )
{
    m_itemFlags = flags;
}

QVariant ItemSpriteModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if( role == Qt::DecorationRole || role == Qt::DisplayRole || role == Qt::UserRole ) {
        SharedResource resource = g_tibiaHandler.getSpriteResource( index.row(), *m_itemData );
        if( resource ) {
            if( role == Qt::DecorationRole ) {
                TibiaSprite sprite = resource->getTibiaSprite();
                if( sprite.image.height() > 128 || sprite.image.width() > 128 )
                    return QVariant( sprite.image.scaled( 128, 128, Qt::KeepAspectRatio ) );

                return QVariant( sprite.image );
            } else if( role == Qt::DisplayRole ) {
                if( resource )
                    return QVariant( g_resourceHandler.getDisplayIdentifier( resource ) );
            } else if( role == Qt::UserRole ) {
                return QVariant::fromValue( resource );
            }
        } else {
            if( role == Qt::DecorationRole ) {
                return QVariant( dummy.image );
            } else if( role == Qt::DisplayRole ) {
                return QVariant( 0 );
            } else if( role == Qt::UserRole ) {
                return QVariant::fromValue( SharedResource() );
            }
        }
    }

    return QVariant();
}

void ItemSpriteModel::invalidate( void )
{
    reset();
}

int ItemSpriteModel::rowCount( const QModelIndex& parent ) const
{
    if( parent.isValid() )
        return 0;

    return m_itemData->getSpriteCount();
}

QMimeData *ItemSpriteModel::mimeData( const QModelIndexList& indexes ) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream( &encodedData, QIODevice::WriteOnly );

    foreach( QModelIndex index, indexes ) {
        if ( index.isValid() ) {
            stream << data( index, Qt::UserRole ).value<SharedResource>();
        }
    }

    mimeData->setData( m_dragMimeType, encodedData );
    return mimeData;
}

// Incoming information
bool ItemSpriteModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
    Q_UNUSED( action );
    Q_UNUSED( column );
    emit decodeMimeDrop( data, row, parent );
    return true;
}

QStringList ItemSpriteModel::mimeTypes( void ) const
{
    return m_dropMimeTypes;
}

Qt::DropActions ItemSpriteModel::supportedDropActions( void ) const
{
    return m_dropActions;
}

Qt::ItemFlags ItemSpriteModel::flags(const QModelIndex&) const
{
    return m_itemFlags;
}
