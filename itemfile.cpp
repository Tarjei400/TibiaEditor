#include "formathandler.h"
#include "tibiahandler.h"
#include "userthread.h"
#include "itemfile.h"

extern TibiaHandler g_tibiaHandler;
extern FormatHandler g_formatHandler;

ItemFile::ItemFile( QObject *parent ) : TibiaFile( parent )
{
    m_begin = 100;
    m_items = 0;
    m_outfits = 0;
    m_effects = 0;
    m_projectiles = 0;
    m_loaded = false;
    m_datFormat = NULL;
    m_thread = new ItemThread( this );
}

ItemFile::~ItemFile( void )
{
    qDeleteAll( m_itemList );
    m_itemList.clear();
    qDeleteAll( m_outfitList );
    m_outfitList.clear();
    qDeleteAll( m_effectList );
    m_effectList.clear();
    qDeleteAll( m_projectileList );
    m_projectileList.clear();
    delete m_thread;
}

void ItemFile::unload( void )
{
    qDeleteAll( m_itemList );
    m_itemList.clear();
    qDeleteAll( m_outfitList );
    m_outfitList.clear();
    qDeleteAll( m_effectList );
    m_effectList.clear();
    qDeleteAll( m_projectileList );
    m_projectileList.clear();

    TibiaFile::unload();
    m_begin = 100;
    m_items = 0;
    m_outfits = 0;
    m_effects = 0;
    m_projectiles = 0;
    m_loaded = false;
    m_datFormat = NULL;
}

qint32 ItemFile::getItemId( TibiaItem *item ) const
{
    qint32 index = -1;
    if( ( index = m_itemList.indexOf( item ) ) != -1 )
        return m_begin + index;
    if( ( index = m_outfitList.indexOf( item ) ) != -1 )
        return getOutfitsBegin() - 1 + index;
    if( ( index = m_effectList.indexOf( item ) ) != -1 )
        return getEffectsBegin() - 1 + index;
    if( ( index = m_projectileList.indexOf( item ) ) != -1 )
        return getProjectilesBegin() - 1 + index;

    return index;
}

qint32 ItemFile::getItemType( TibiaItem *item ) const
{
    qint32 index = -1;
    if( ( index = m_itemList.indexOf( item ) ) != -1 )
        return m_begin + index;
    if( ( index = m_outfitList.indexOf( item ) ) != -1 )
        return index + 1; // Looktype 0 reserved
    if( ( index = m_effectList.indexOf( item ) ) != -1 )
        return index;
    if( ( index = m_projectileList.indexOf( item ) ) != -1 )
        return index;

    return index;
}

TibiaItem *ItemFile::getItemById( quint32 itemId ) const
{
    if( itemId <= getItemCount() )
        return m_itemList.value( itemId - m_begin );
    if( itemId >= getOutfitsBegin() && itemId < getEffectsBegin() )
        return m_outfitList.value( itemId - getOutfitsBegin() );
    if( itemId >= getEffectsBegin() && itemId < getProjectilesBegin() )
        return m_effectList.value( itemId - getEffectsBegin() );
    if( itemId >= getProjectilesBegin() && itemId < getLast() )
        return m_projectileList.value( itemId - getProjectilesBegin() );
    else
        return NULL; // Out of range
}

TibiaItem *ItemFile::getItemByIndex( quint8 parent, qint32 index ) const
{
    switch( parent ) {
    case ITEM_PARENT_ITEMS:
        return m_itemList.value( index );
        break;
    case ITEM_PARENT_OUTFITS:
        return m_outfitList.value( index );
        break;
    case ITEM_PARENT_EFFECTS:
        return m_effectList.value( index );
        break;
    case ITEM_PARENT_PROJECTILES:
        return m_projectileList.value( index );
        break;
    default:
        return NULL;
        break;
    }

    return NULL;
}

qint32 ItemFile::getItemIndex( TibiaItem *item ) const
{
    qint32 index = -1;
    if( ( index = m_itemList.indexOf( item ) ) != -1 )
        return index;
    if( ( index = m_outfitList.indexOf( item ) ) != -1 )
        return index;
    if( ( index = m_effectList.indexOf( item ) ) != -1 )
        return index;
    if( ( index = m_projectileList.indexOf( item ) ) != -1 )
        return index;

    return index;
}

bool ItemFile::createNew( void )
{
    if( !m_loaded ) {
        m_items = 100;
        m_outfits = 1;
        m_effects = 1;
        m_projectiles = 1;
        insert( ITEM_PARENT_ITEMS, -1, TibiaItem::createItem( ITEM_TYPE_ITEM ) );
        insert( ITEM_PARENT_OUTFITS, -1, TibiaItem::createItem( ITEM_TYPE_OUTFIT ) );
        insert( ITEM_PARENT_EFFECTS, -1, TibiaItem::createItem( ITEM_TYPE_EFFECT ) );
        insert( ITEM_PARENT_PROJECTILES, -1, TibiaItem::createItem( ITEM_TYPE_PROJECTILE ) );
        m_loaded = true;
        return true;
    }

    return false;
}

bool ItemFile::load( const QString& name )
{
     setFileName( name );

    if( !TibiaFile::exists() ) {
        emit documentError( name, QObject::tr( "File does not exist." ), -1 );
        return false;
    }

    if( !TibiaFile::open( QIODevice::ReadOnly ) ) {
        emit parseError( QObject::tr( "Open Error" ), TibiaFile::error() );
        return false;
    }
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::red ), tr( "File opened lets read first bytes." ) );
    TibiaFile::seek( 0 );

    QDataStream in( this );
    in.setByteOrder( QDataStream::LittleEndian );
    in >> m_signature;
    in >> m_items;
    in >> m_outfits;
    in >> m_effects;
    in >> m_projectiles;
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::red ), tr( "First bytes read." ) );
    quint32 count = 0;
    emit parseError( QObject::tr( "So far its fine", "" ), TibiaFile::error());
    bool success = false;
    success = loadItemSection( in, m_begin, m_items+1, m_itemList, ITEM_TYPE_ITEM, ITEM_PARENT_ITEMS, count ); // min <= max
    if( success )
        success = loadItemSection( in, 0, m_outfits, m_outfitList, ITEM_TYPE_OUTFIT, ITEM_PARENT_OUTFITS, count ); // min < max
    if( success )
        success = loadItemSection( in, 0, m_effects, m_effectList, ITEM_TYPE_EFFECT, ITEM_PARENT_EFFECTS, count ); // min < max
    if( success )
        success = loadItemSection( in, 0, m_projectiles, m_projectileList, ITEM_TYPE_PROJECTILE, ITEM_PARENT_PROJECTILES, count ); // min < max

    if( !success ) {
        unload();
        TibiaFile::close();
        return false;
    }


    TibiaFile::close();
    m_loaded = true;
    return true;
}

bool ItemFile::loadItemSection( QDataStream& in, quint32 minimum, quint32 maximum, ItemList& items, quint8 type, quint8 parent, quint32& count )
{
    for( int i = minimum; i < maximum; i++ ) {
        ItemData itemData;
        QString error;
        if(!loadItem( m_datFormat, in, itemData, error )) {
            emit documentError( TibiaFile::fileName(), QObject::tr( "Successfully Loaded: %n item(s)\nFailed on item %2\n%3", "", count ).arg( count + m_begin ).arg( error ), -1 );
            return false;
        }

        TibiaItem *tibiaItem = new TibiaItem( itemData );
        tibiaItem->setType( type );
        tibiaItem->setParentType( parent );
        items.push_back( tibiaItem );
        count++;

    }
    return true;
}

bool ItemFile::loadItem( DatFormat *datFormat, QDataStream& in, ItemData& itemData, QString& error, bool oldFormat /*=false*/ )
{
    if(!loadItemProperties( datFormat, in, itemData, error ))
        return false;

    in >> itemData.width;
    in >> itemData.height;

    if( itemData.width > 1 || itemData.height > 1 )
        in >> itemData.cropsize;

    in >> itemData.blendframes;
    in >> itemData.xdiv;
    in >> itemData.ydiv;

    if( datFormat->hasZFactor() ) {
        in >> itemData.zdiv;
    }

    in >> itemData.animcount;

    for( quint32 i = 0; i < itemData.getSpriteCount( datFormat->hasZFactor() ); ++i ) {
        quint32 spriteId;
        quint16 spriteId16;
         (oldFormat ? in >>spriteId16 : in >>spriteId);
        itemData.setLocalSprite( i, (oldFormat ? spriteId16 : spriteId) );
    }

    return true;
}

bool ItemFile::loadItemProperties( DatFormat *datFormat, QDataStream& in, ItemData& itemData, QString& error )
{
    quint8 optionByte = 0;
    quint8 previousByte = 0;

    QIODevice *device = in.device();
    if( !device )
        return false;

    PropertyList properties;
    in >> optionByte;
    while( optionByte != 0xFF ) {
        ItemProperty itemProperty;
        DatProperty *datProperty = datFormat->getPropertyByHeader( optionByte );
        if( !datProperty || datProperty->getHeader() == 0xFF ) { // Invalid header
            error = QObject::tr( "Unknown option byte: 0x%1\nPrevious option byte: 0x%2\nAddress: 0x%3\n\nAborting..." ).arg( optionByte, 0, 16 ).arg( previousByte, 0, 16 ).arg( device->pos(), 0, 16 );
            return false;
        }
        BaseProperty *baseProperty = g_formatHandler.getBaseProperty( datProperty->getBase() );
        if( !baseProperty ) {
            error = QObject::tr( "Unknown option byte: 0x%1\nPrevious option byte: 0x%2\nAddress: 0x%3\n\nAborting..." ).arg( optionByte, 0, 16 ).arg( previousByte, 0, 16 ).arg( device->pos(), 0, 16 );
            return false;
        } else { // Header is valid
            itemProperty.header = baseProperty->getHeader();
            for( DatValueHash::const_iterator it = baseProperty->getValuesBegin(); it != baseProperty->getValuesEnd(); it++ ) { // Search loader according to header sizes
                switch( it.value()->getSize() ) {
                case sizeof( quint8 ): {
                    quint8 buffer;
                    in >> buffer;
                    itemProperty.childU8.push_back( buffer );
                }
                break;
                case sizeof( quint16 ): {
                    quint16 buffer;
                    in >> buffer;
                    itemProperty.childU16.push_back( buffer );
                }
                break;
                }
            }
        }

        properties.push_back( itemProperty ); // Convert our options to the global format

        previousByte = optionByte;
        in >> optionByte;
    }

    itemData.setProperties( properties );
    return true;
}

/*bool ItemFile::save( const QString& name )
{
    setFileName( name );

    if( !TibiaFile::open( QIODevice::WriteOnly ) ){
        emit parseError( QObject::tr( "Open Error" ), TibiaFile::error() );
        return false;
    }

    TibiaFile::seek( 0 );

    QDataStream out( this );
    out.setByteOrder( QDataStream::LittleEndian );
    out << m_signature;
    out << m_items;
    out << m_outfits;
    out << m_effects;
    out << m_projectiles;

    saveItemSection( out, m_itemList );
    saveItemSection( out, m_outfitList );
    saveItemSection( out, m_effectList );
    saveItemSection( out, m_projectileList );

    TibiaFile::close();
    //m_thread->setup();
    //m_thread->setName( name );
    //m_thread->execute();
    return true;
}*/

bool ItemFile::saveItemSection( QDataStream& out, ItemList& items )
{
    foreach( TibiaItem* tibiaItem, items ) {
        if( !saveItem( m_datFormat, out, tibiaItem->getItemData() ) )
            return false;
    }

    return true;
}

bool ItemFile::saveItem( DatFormat *datFormat, QDataStream& out, const ItemData& itemData )
{
    if( !saveItemProperties( datFormat, out, itemData ) )
        return false;

    out << itemData.width;
    out << itemData.height;
    if( itemData.width > 1 || itemData.height > 1 )
        out << itemData.cropsize;

    out << itemData.blendframes;
    out << itemData.xdiv;
    out << itemData.ydiv;

    if( datFormat->hasZFactor() )
        out << itemData.zdiv;

    out << itemData.animcount;

    for( quint32 i = 0; i < itemData.getSpriteCount( datFormat->hasZFactor() ); ++i )
        out << itemData.getLocalSprite( i );

    return true;
}

bool ItemFile::saveItemProperties( DatFormat *datFormat, QDataStream& out, const ItemData& itemData )
{
    for( PropertyList::const_iterator it = itemData.getPropertiesBegin(); it != itemData.getPropertiesEnd(); it++ ) {
        DatProperty *datProperty = datFormat->getPropertyByBase( (*it).header );
        if( !datProperty )
            continue;

        BaseProperty *baseProperty = g_formatHandler.getBaseProperty( datProperty->getBase() );
        if( !baseProperty || datProperty->getHeader() == 0xFF || baseProperty->getBase() == 0xFF )
            continue;

        out << datProperty->getHeader();

        if( baseProperty->getValuesSize() == (*it).childU8.size() || baseProperty->getValuesSize() == (*it).childU16.size() ) {
            quint8 count = 0;
            for( DatValueHash::const_iterator val = baseProperty->getValuesBegin(); val !=  baseProperty->getValuesEnd(); val++ ) {
                switch( val.value()->getSize() ) {
                case sizeof( quint8 ):
                    out << (*it).childU8.value( count );
                    break;
                case sizeof( quint16 ):
                    out << (*it).childU16.value( count );
                    break;
                }

                count++;
            }
        } else {
            QIODevice *device = out.device();
            if( device ) {
                device->seek( device->pos() - sizeof( quint8 ) ); // Jump back to the byte option we just wrote prevent corruption
                continue;
            }
        }
    }

    out << ( quint8 )0xFF;
    return true;
}

bool ItemFile::replace( TibiaItem *item, TibiaItem *replaced )
{
    if( !replaced || !item )
        return false;

    switch( replaced->getParentType() ) {
    case ITEM_PARENT_ITEMS: {
        qint32 index = m_itemList.indexOf( replaced );
        if( index != -1 ) {
            m_itemList.replace( index, item );
            item->setType( replaced->getType() );
            item->setParentType( replaced->getParentType() );
            replaced->setParentType( ITEM_PARENT_INTERNAL );
            emit replaceItem( index, item );
            return true;
        }
    }
    break;
    case ITEM_PARENT_OUTFITS: {
        qint32 index = m_outfitList.indexOf( replaced );
        if( index != -1 ) {
            m_outfitList.replace( index, item );
            item->setType( replaced->getType() );
            item->setParentType( replaced->getParentType() );
            replaced->setParentType( ITEM_PARENT_INTERNAL );
            emit replaceOutfit( index, item );
            return true;
        }
    }
    break;
    case ITEM_PARENT_EFFECTS: {
        qint32 index = m_effectList.indexOf( replaced );
        if( index != -1 ) {
            m_effectList.replace( index, item );
            item->setType( replaced->getType() );
            item->setParentType( replaced->getParentType() );
            replaced->setParentType( ITEM_PARENT_INTERNAL );
            emit replaceEffect( index, item );
            return true;
        }
    }
    break;
    case ITEM_PARENT_PROJECTILES: {
        qint32 index = m_projectileList.indexOf( replaced );
        if( index != -1 ) {
            m_projectileList.replace( index, item );
            item->setType( replaced->getType() );
            item->setParentType( replaced->getParentType() );
            replaced->setParentType( ITEM_PARENT_INTERNAL );
            emit replaceProjectile( index, item );
            return true;
        }
    }
    break;
    }

    return false;
}

bool ItemFile::remove( TibiaItem *item )
{
    if( !item )
        return false;

    switch( item->getParentType() ) {
    case ITEM_PARENT_ITEMS: {
        qint32 index = m_itemList.indexOf( item );
        if( index != -1 ) {
            item->setParentType( ITEM_PARENT_INTERNAL );
            m_itemList.removeAt( index );
            m_items--;
            emit removeItem( index, item );
            return true;
        }
    }
    break;

    case ITEM_PARENT_OUTFITS: {
        qint32 index = m_outfitList.indexOf( item );
        if( index != -1 ) {
            item->setParentType( ITEM_PARENT_INTERNAL );
            m_outfitList.removeAt( index );
            m_outfits--;
            emit removeOutfit( index, item );
            return true;
        }
    }
    break;

    case ITEM_PARENT_EFFECTS: {
        qint32 index = m_effectList.indexOf( item );
        if( index != -1 ) {
            item->setParentType( ITEM_PARENT_INTERNAL );
            m_effectList.removeAt( index );
            m_effects--;
            emit removeEffect( index, item );
            return true;
        }
    }
    break;

    case ITEM_PARENT_PROJECTILES: {
        qint32 index = m_projectileList.indexOf( item );
        if( index != -1 ) {
            item->setParentType( ITEM_PARENT_INTERNAL );
            m_projectileList.removeAt( index );
            m_projectiles--;
            emit removeProjectile( index, item );
            return true;
        }
    }
    break;
    }

    return false;
}

bool ItemFile::insert( quint8 parent, qint32 index, TibiaItem *item )
{
    if( !item )
        return false;

    switch( parent ) {
    case ITEM_PARENT_ITEMS: {
        if( index == -1 )
            index = m_itemList.size();

        item->setParentType( parent );
        item->setType( ITEM_TYPE_ITEM );
        m_itemList.insert( index, item );
        m_items++;
        emit insertItem( index, item );
        return true;
    }
    break;

    case ITEM_PARENT_OUTFITS: {
        if( index == -1 )
            index = m_outfitList.size();

        item->setParentType( parent );
        item->setType( ITEM_TYPE_OUTFIT );
        m_outfitList.insert( index, item );
        m_outfits++;
        emit insertOutfit( index, item );
        return true;
    }
    break;

    case ITEM_PARENT_EFFECTS: {
        if( index == -1 )
            index = m_effectList.size();

        item->setParentType( parent );
        item->setType( ITEM_TYPE_EFFECT );
        m_effectList.insert( index, item );
        m_effects++;
        emit insertEffect( index, item );
        return true;
    }
    break;

    case ITEM_PARENT_PROJECTILES: {
        if( index == -1 )
            index = m_projectileList.size();

        item->setParentType( parent );
        item->setType( ITEM_TYPE_PROJECTILE );
        m_projectileList.insert( index, item );
        m_projectiles++;
        emit insertProjectile( index, item );
        return true;
    }
    break;
    }

    return false;
}
