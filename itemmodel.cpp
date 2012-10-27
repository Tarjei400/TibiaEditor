#include <QtCore/QMimeData>
#include "tibiaitem.h"
#include "itemmodel.h"

extern TibiaHandler g_tibiaHandler;

ItemModel::ItemModel( QObject *_parent ) : QAbstractListModel( _parent ),
    m_dropActions( Qt::CopyAction ),
    m_itemFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled )
{

}

void ItemModel::setDragMimeFormat( const QString& mimeType )
{
    m_dragMimeType = mimeType;
}

void ItemModel::setDropMimeFormats( const QStringList& mimeTypes )
{
    m_dropMimeTypes = mimeTypes;
}

void ItemModel::setSupportedDropActions( Qt::DropActions actions )
{
    m_dropActions = actions;
}

void ItemModel::setItemFlags( Qt::ItemFlags flags )
{
    m_itemFlags = flags;
}

void ItemModel::setDrawParameters( const DrawParameters& _dParams )
{
    bool totalUpdate = ( !m_itemList.isEmpty() && m_dParams != _dParams );

    m_dParams = _dParams;

    if( totalUpdate )
        reset();
}

void ItemModel::addItem( qint32 index, TibiaItem *item )
{
    if( index == -1 )
        index = m_itemList.size();

    beginInsertRows( QModelIndex(), index, index );
    m_itemList.insert( index, item );
    endInsertRows();
}

bool ItemModel::removeItem( qint32 index, TibiaItem *item )
{
    if( index == -1 )
        index = m_itemList.indexOf( item );
    if( index == -1 )
        return false;

    beginRemoveRows( QModelIndex(), index, index );
    m_itemList.removeAt( index );
    endRemoveRows();
    return true;
}

void ItemModel::replaceItem( qint32 index, TibiaItem *item )
{
    QModelIndex modelIndex = QAbstractListModel::index( index );
    if( modelIndex.isValid() ) {
        if( m_itemList.value( index ) != NULL ) {
            m_itemList.replace( index, item );
            emit dataChanged( modelIndex, modelIndex );
        }
    }
}

void ItemModel::updateItem( TibiaItem *item )
{
    QModelIndex modelIndex = QAbstractListModel::index( m_itemList.indexOf( item ) );
    if( modelIndex.isValid() )
        emit dataChanged( modelIndex, modelIndex );
}

QModelIndex ItemModel::indexByItem( TibiaItem *item )
{
    return QAbstractListModel::index( m_itemList.indexOf( item ) );
}

QVariant ItemModel::data( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if( role == Qt::DecorationRole || role == Qt::DisplayRole || role == Qt::UserRole ) {
        ItemFile *itemFile = g_tibiaHandler.getItemFile();
        if( itemFile ) {
            TibiaItem *item = m_itemList.value( index.row() );
            if( item ) {
                if( role == Qt::DisplayRole ) {
                    if( !item->getSource().isEmpty() && !item->hasEditorParent() )
                        return QVariant( item->getSource() );

                    return QVariant( itemFile->getItemType( item ) );
                } else if( role == Qt::UserRole )
                    return QVariant::fromValue<TibiaItem *>( item );
                else if( role == Qt::DecorationRole )
                    return QVariant( g_tibiaHandler.drawItem( item, m_dParams ) );
            }
        }
    }

    return QVariant();
}

bool ItemModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if( row >= m_itemList.size() || row + count <= 0 )
        return false;

    int beginRow = qMax( 0, row );
    int endRow = qMin( row + count - 1, m_itemList.size() - 1 );

    beginRemoveRows( parent, beginRow, endRow );
    qint32 subCount = 0;
    while( subCount <= count ) {
        m_itemList.removeAt( beginRow );
        subCount++;
    }
    endRemoveRows();
    return true;
}


int ItemModel::rowCount( const QModelIndex& ) const
{
    return m_itemList.size();
}

void ItemModel::clear( void )
{
    m_itemList.clear();
    reset();
}

void ItemModel::deleteAll( void )
{
    qDeleteAll( m_itemList );
    m_itemList.clear();
    reset();
}

void ItemModel::setItemList( const ItemList& items )
{
    m_itemList = items;
    reset();
}

void ItemModel::addItemList( const ItemList& items )
{
    m_itemList += items;
    reset();
}


void ItemModel::invalidate( void )
{
    reset();
}

QMimeData *ItemModel::mimeData( const QModelIndexList& indexes ) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream( &encodedData, QIODevice::WriteOnly );

    stream << (quint32)indexes.size();
    foreach( QModelIndex index, indexes ) {
        if ( index.isValid() ) {
            stream << (quint64)data( index, Qt::UserRole ).value<TibiaItem *>();
        }
    }

    mimeData->setData( m_dragMimeType, encodedData );
    return mimeData;
}

// Incoming information
bool ItemModel::dropMimeData( const QMimeData *data, Qt::DropAction, int row, int, const QModelIndex& parent )
{
    emit decodeMimeDrop( data, row, parent );
    return true;
}

QStringList ItemModel::mimeTypes( void ) const
{
    return m_dropMimeTypes;
}

Qt::DropActions ItemModel::supportedDropActions( void ) const
{
    return m_dropActions;
}

Qt::ItemFlags ItemModel::flags( const QModelIndex& ) const
{
    return m_itemFlags;
}
