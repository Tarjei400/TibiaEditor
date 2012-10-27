#include <QtCore/QMimeData>
#include <QtGui/QPixmapCache>
#include "spritemodel.h"
#include "tibiafile.h"
#include "resourcehandler.h"
#include "tibiahandler.h"
#include "resourcefile.h"

extern ResourceHandler g_resourceHandler;
extern TibiaHandler g_tibiaHandler;

SpriteModel::SpriteModel( QObject *parent, quint8 resourceType, TibiaFile *tibiaFile ) : QAbstractListModel( parent ),
    m_resourceType( resourceType ),
    m_tibiaFile( tibiaFile ),
    m_dropActions( Qt::CopyAction ),
    m_itemFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled )
{

}

SpriteModel::~SpriteModel( void )
{

}

void SpriteModel::setDragMimeFormat( const QString& mimeType )
{
    m_dragMimeType = mimeType;
}

void SpriteModel::setDropMimeFormats( const QStringList& mimeTypes )
{
    m_dropMimeTypes = mimeTypes;
}

void SpriteModel::setSupportedDropActions( Qt::DropActions actions )
{
    m_dropActions = actions;
}

void SpriteModel::setItemFlags( Qt::ItemFlags flags )
{
    m_itemFlags = flags;
}

qint32 SpriteModel::getIndexByResource( SharedResource& obj ) const
{
    return m_resourceList.indexOf( obj );
}

void SpriteModel::addSprites( quint8 resourceType, ResourceList& resources )
{
    if( resourceType != m_resourceType )
        return;

    emit addedSprites( resourceType, resources.size() );

    beginInsertRows( QModelIndex(), m_resourceList.size(), m_resourceList.size() + resources.size() );
    m_resourceList += resources;
    endInsertRows();
}

void SpriteModel::removeSprites( quint8 resourceType, quint32 amount )
{
    if( resourceType != m_resourceType )
        return;

    emit removedSprites( resourceType, amount );

    int row = m_resourceList.size() - amount;
    int count = amount;
    int beginRow = qMax( 0, row );
    int endRow = qMin( row + count - 1, m_resourceList.size() - 1 );

    beginRemoveRows( QModelIndex(), beginRow, endRow );
    for( qint32 i = beginRow; i <= endRow; i++ )
        m_resourceList.removeLast();
    endRemoveRows();
}

void SpriteModel::addSprite( quint8 resourceType, SharedResource& obj, qint32 index )
{
    if( resourceType != m_resourceType )
        return;

    if( index == -1 )
        index = m_resourceList.size();

    emit addedSprites( resourceType, 1 );

    beginInsertRows( QModelIndex(), index, index );
    m_resourceList.insert( index, obj );
    endInsertRows();
}

bool SpriteModel::removeSprite( quint8 resourceType, qint32 index )
{
    if( resourceType != m_resourceType )
        return false;

    if( index == -1 )
        return false;

    emit removedSprites( resourceType, 1 );

    beginRemoveRows( QModelIndex(), index, index );
    m_resourceList.removeAt( index );
    endRemoveRows();
    return true;
}

void SpriteModel::setSprite( quint8 resourceType, qint32 index, SharedResource& obj )
{
    if( resourceType != m_resourceType )
        return;

    QModelIndex modelIndex = QAbstractListModel::index( index );
    if( modelIndex.isValid() ) {
        m_resourceList.replace( index, obj );
        emit dataChanged( modelIndex, modelIndex );
    }
}

QVariant SpriteModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if( role == Qt::DecorationRole || role == Qt::DisplayRole || role == Qt::UserRole ) {
        SharedResource resource = m_resourceList.value( index.row() );
        if( role == Qt::DecorationRole ) {
            TibiaSprite sprite = g_resourceHandler.getSpriteByResource( resource );
            if( sprite.image.height() > 128 || sprite.image.width() > 128 )
                return QVariant( sprite.image.scaled( 128, 128, Qt::KeepAspectRatio ) );

            return QVariant( sprite.image );
        } else if( role == Qt::DisplayRole ) {
            if( resource ) {
                quint32 realId = g_resourceHandler.getDisplayIdentifier( resource );
                if( realId == 0xFFFF ) {
                    if( ResourceFile *resourceFile = qobject_cast<ResourceFile *>( resource->getFile() ) )
                        return QVariant( resourceFile->getSource() );
                }
                return QVariant( realId );
            }
        } else if( role == Qt::UserRole ) {
            return QVariant::fromValue( resource );
        }
    }

    return QVariant();
}

// Item is visible if QRect QTreeView::visualRect ( const QModelIndex & index ) const is valid

int SpriteModel::rowCount( const QModelIndex& parent ) const
{
    if( parent.isValid() )
        return 0;

    return m_resourceList.size();
}
void SpriteModel::clear( void )
{
    m_resourceList.clear();
    reset();
}

void SpriteModel::setResourceList( const ResourceList& sprites )
{
    m_resourceList = sprites;
    reset();
}

QMimeData *SpriteModel::mimeData( const QModelIndexList& indexes ) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream( &encodedData, QIODevice::WriteOnly );

    stream << ( qint32 )indexes.size();
    foreach( QModelIndex index, indexes ) {
        if ( index.isValid() ) {
            stream << data( index, Qt::UserRole ).value<SharedResource>();
        }
    }

    mimeData->setData( m_dragMimeType, encodedData );
    return mimeData;
}

// Incoming information
bool SpriteModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
    Q_UNUSED( action );
    Q_UNUSED( column );
    emit decodeMimeDrop( data, row, parent );
    return true;
}

QStringList SpriteModel::mimeTypes( void ) const
{
    return m_dropMimeTypes;
}

Qt::DropActions SpriteModel::supportedDropActions( void ) const
{
    return m_dropActions;
}

Qt::ItemFlags SpriteModel::flags( const QModelIndex& ) const
{
    return m_itemFlags;
}
