#include "tibiaitem.h"
#include "tibiafile.h"
#include "itemfile.h"

bool ItemData::addProperty( const ItemProperty& itemProperty )
{
    if( properties.contains( itemProperty ) )
        return false;

    properties.push_back( itemProperty );
    return true;
}

bool ItemData::removeProperty( const ItemProperty& itemProperty )
{
    if( !properties.contains( itemProperty ) )
        return false;

    return properties.removeOne( itemProperty );
}

void ItemData::setSpriteResource( const quint32 frame, SharedResource& src )
{
    resourceHash.insert( frame, src );
}

void ItemData::setLocalSprite( const quint32 frame, const quint32 spriteId )
{
    localMap.insert( frame, spriteId );
}

SharedResource ItemData::getSpriteResource( const quint32 frame ) const
{
    return resourceHash.value( frame );
}

quint32 ItemData::getLocalSprite( const quint32 frame ) const
{
    return localMap.value( frame );
}

bool ItemData::isNull( void ) const
{
    return ( width == 1 && height == 1 && cropsize == 0 && blendframes == 1 && xdiv == 1 && ydiv == 1 && zdiv == 1 && animcount == 1 && properties.isEmpty() && resourceHash.isEmpty() );
}

bool ItemData::isValid( void ) const
{
    return type != ITEM_TYPE_NONE;
}

quint32 TibiaItem::getSpriteFrame( const ItemData& itemData, quint8 divx, quint8 divy, quint8 divz, quint8 frame, quint32 x, quint32 y, quint8 animation )
{
    return ( ( ( ( ( ( ( animation ) * itemData.zdiv + divz ) * itemData.ydiv + divy ) * itemData.xdiv + divx ) * itemData.blendframes + frame ) * itemData.height + y ) * itemData.width + x );
}

TibiaItem::TibiaItem( void ) : TibiaObject()
{
    d = new ItemData;
}

TibiaItem::TibiaItem( const TibiaItem& other ) : TibiaObject()
{
    d = new ItemData( *other.d );
}

TibiaItem::TibiaItem( const ItemData& data ) : TibiaObject()
{
    d = new ItemData( data );
}

TibiaItem *TibiaItem::createItem( quint8 type, quint8 height, quint8 width, quint8 layers, quint8 xpattern, quint8 ypattern, quint8 zpattern, quint8 animations, bool addons )
{
    TibiaItem *item = new TibiaItem;
    item->setType( type );
    item->setHeight( height );
    item->setWidth( width );
    item->setLayers( layers );
    switch( type ) {
    case ITEM_TYPE_ITEM:
        item->setXDiv( xpattern );
        item->setYDiv( ypattern );
        item->setZDiv( zpattern );
        item->setAnimation( animations );
        break;
    case ITEM_TYPE_OUTFIT: {
        item->setXDiv( 4 );
        if( addons )
            item->setYDiv( 3 );
        item->setAnimation( 3 );
    }
    break;
    case ITEM_TYPE_EFFECT:
        item->setAnimation( animations );
        break;
    case ITEM_TYPE_PROJECTILE:
        item->setXDiv( 3 );
        item->setYDiv( 3 );
        break;
    }

    for( quint32 i = 0; i < item->getSpriteCount(); ++i )
        item->setLocalSprite( i, 0 );

    return item;
}

TibiaItem::~TibiaItem( void )
{
    delete d;
}

void TibiaItem::reset( void )
{
    d->reset();
}

QString TibiaItem::fullName( ItemFile *itemFile, TibiaItem *tibiaItem )
{
    if( !tibiaItem )
        return QObject::tr( item_type_strings[0] );

    QString name;
    switch( tibiaItem->getParentType() ) {
    case ITEM_PARENT_LIBRARY:
        name = QObject::tr( "Library %1" ).arg( TibiaItem::typeName( tibiaItem->getType() ) );
        break;
    case ITEM_PARENT_EXTERNAL:
        name = QObject::tr( "External %1" ).arg( TibiaItem::typeName( tibiaItem->getType() ) );
        break;
    case ITEM_PARENT_ITEMS:
    case ITEM_PARENT_OUTFITS:
    case ITEM_PARENT_EFFECTS:
    case ITEM_PARENT_PROJECTILES:
    default: {
        name = QObject::tr( "%1" ).arg( TibiaItem::typeName( tibiaItem->getType() ) );

        if( itemFile ) {
            qint32 itemId = itemFile->getItemType( tibiaItem );
            if( itemId != -1 )
                name += QObject::tr( " %2" ).arg( itemId );
        }
    }
    break;
    }

    return name;
}

QString TibiaItem::typeName( quint8 type )
{
    quint8 number = 0;
    switch( type ) {
    case ITEM_TYPE_ITEM:
        number = 0;
        break;
    case ITEM_TYPE_OUTFIT:
        number = 1;
        break;
    case ITEM_TYPE_EFFECT:
        number = 2;
        break;
    case ITEM_TYPE_PROJECTILE:
        number = 3;
        break;
    default:
        number = 0;
        break;
    }

    return QObject::tr( item_type_strings[number] );
}

QString TibiaItem::parentName( quint8 parent )
{
    if( parent > sizeof( item_parent_strings ) )
        return QObject::tr( "an unknown location" );

    return QObject::tr( item_parent_strings[parent] );
}

bool TibiaItem::hasHeader( quint8 header ) const
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    for( PropertyList::const_iterator it = d->properties.begin(); it != d->properties.end(); it++ ) {
        if( (*it).header == header && header != INVALID_HEADER )
            return true;
    }

    return false;
}

bool TibiaItem::hasProperty( const ItemProperty& itemProperty ) const
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    for( PropertyList::const_iterator it = d->properties.begin(); it != d->properties.end(); it++ ) {
        if( (*it).childU16 == itemProperty.childU16 && (*it).childU8 == itemProperty.childU8 )
            return true;
    }

    return false;
}

ItemProperty TibiaItem::getProperty( quint8 header ) const
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    for( PropertyList::const_iterator it = d->properties.begin(); it != d->properties.end(); it++ ) {
        if( (*it).header == header && header != INVALID_HEADER )
            return (*it);
    }

    return ItemProperty();
}

bool TibiaItem::contains( const PropertyList& propList ) const
{
    bool success = false;
    for( PropertyList::const_iterator it = propList.begin(); it != propList.end(); it++ ) { // Cycle list until header not found, abort on fail
        success = hasProperty( *it );
        if( !success )
            return false;
    }

    return success; // Loop should return true if all properties found
}

bool TibiaItem::contains( const HeaderList& headerList ) const
{
    bool success = false;
    for( HeaderList::const_iterator it = headerList.begin(); it != headerList.end(); it++ ) { // Cycle list until header not found, abort on fail
        success = hasHeader( *it );
        if( !success )
            return false;
    }

    return success; // Loop should return true if all properties found
}

quint32 ItemData::getSpriteCount( bool zfactor ) const
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    if( !zfactor )
        return width * height * blendframes * xdiv * ydiv * animcount;

    return width * height * blendframes * xdiv * ydiv * zdiv * animcount;
}

quint32 TibiaItem::getSpriteCount( bool zfactor ) const
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    return d->getSpriteCount( zfactor );
}

void TibiaItem::setSpriteResource( const quint32 frame, SharedResource& src )
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    if( d->resourceHash.value( frame ) )
        d->resourceHash.value( frame )->free();

    d->resourceHash.remove( frame );

    if( src )
        src->use();

    d->resourceHash.insert( frame, src );
}

bool TibiaItem::referencedSprite( const quint32 frame ) const
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    if( d->resourceHash.value( frame ) )
        return true;

    return false;
}

QDataStream& operator<< ( QDataStream& stream, const ItemData& itemData )
{
    stream << itemData.parent;
    stream << itemData.type;
    stream << itemData.width;
    stream << itemData.height;
    stream << itemData.cropsize;
    stream << itemData.blendframes;
    stream << itemData.xdiv;
    stream << itemData.ydiv;
    stream << itemData.zdiv;
    stream << itemData.animcount;
    stream << itemData.properties;
    stream << itemData.localMap;
    stream << itemData.resourceHash;
    return stream;
}

QDataStream& operator>> ( QDataStream& stream, ItemData& itemData )
{
    stream >> itemData.parent;
    stream >> itemData.type;
    stream >> itemData.width;
    stream >> itemData.height;
    stream >> itemData.cropsize;
    stream >> itemData.blendframes;
    stream >> itemData.xdiv;
    stream >> itemData.ydiv;
    stream >> itemData.zdiv;
    stream >> itemData.animcount;
    stream >> itemData.properties;
    stream >> itemData.localMap;
    stream >> itemData.resourceHash;
    return stream;
}

QDataStream& operator<< ( QDataStream& stream, const TibiaItem& tibiaItem )
{
    stream << tibiaItem.getProperties();
    stream << tibiaItem.getParentType();
    stream << tibiaItem.getType();
    stream << tibiaItem.getWidth();
    stream << tibiaItem.getHeight();
    stream << tibiaItem.getCropsize();
    stream << tibiaItem.getLayers();
    stream << tibiaItem.getXDiv();
    stream << tibiaItem.getYDiv();
    stream << tibiaItem.getZDiv();
    stream << tibiaItem.getAnimation();
    stream << tibiaItem.getLocalResources();
    stream << tibiaItem.getResources();
    return stream;
}

QDataStream& operator>> ( QDataStream& stream, TibiaItem& tibiaItem )
{
    PropertyList properties;
    stream >> properties;
    tibiaItem.setProperties( properties );
    quint8 byte;
    stream >> byte;
    tibiaItem.setParentType( byte );
    stream >> byte;
    tibiaItem.setType( byte );
    stream >> byte;
    tibiaItem.setWidth( byte );
    stream >> byte;
    tibiaItem.setHeight( byte );
    stream >> byte;
    tibiaItem.setCropsize( byte );
    stream >> byte;
    tibiaItem.setLayers( byte );
    stream >> byte;
    tibiaItem.setXDiv( byte );
    stream >> byte;
    tibiaItem.setYDiv( byte );
    stream >> byte;
    tibiaItem.setZDiv( byte );
    stream >> byte;
    tibiaItem.setAnimation( byte );
    LocalMap localMap;
    stream >> localMap;
    tibiaItem.setLocalResources( localMap );
    ResourceHash resourceHash;
    stream >> resourceHash;
    tibiaItem.setResources( resourceHash );
    return stream;
}

QDataStream& operator<< ( QDataStream& stream, const ItemProperty& itemProperty )
{
    stream << itemProperty.header;
    stream << itemProperty.childU8;
    stream << itemProperty.childU16;
    return stream;
}

QDataStream& operator>> ( QDataStream& stream, ItemProperty& itemProperty )
{
    stream >> itemProperty.header;
    stream >> itemProperty.childU8;
    stream >> itemProperty.childU16;
    return stream;
}
