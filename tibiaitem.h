#ifndef TIBIAITEM_H
#define TIBIAITEM_H

#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QSharedPointer>

#include <QtCore/QDataStream>

#include "tibiamodule.h"
#include "tibiasprite.h"
#include "tibiaresource.h"

#define INVALID_HEADER      0xFF

#define ITEM_TYPE_ITEM          0xFF
#define ITEM_TYPE_OUTFIT        0xFE
#define ITEM_TYPE_EFFECT        0xFD
#define ITEM_TYPE_PROJECTILE    0xFC
#define ITEM_TYPE_NONE          0x00

#define ITEM_PARENT_INTERNAL 0
#define ITEM_PARENT_ITEMS 1
#define ITEM_PARENT_OUTFITS 2
#define ITEM_PARENT_EFFECTS 3
#define ITEM_PARENT_PROJECTILES 4
#define ITEM_PARENT_LIBRARY 5
#define ITEM_PARENT_EXTERNAL 6

typedef QList<quint8> HeaderList;
typedef QMap<quint32,quint32> LocalMap;

static const char *item_type_strings[] = {
    QT_TR_NOOP( "Item" ),
    QT_TR_NOOP( "Outfit" ),
    QT_TR_NOOP( "Effect" ),
    QT_TR_NOOP( "Projectile" )
};

static const char *item_parent_strings[] = {
    QT_TR_NOOP( "Internal" ),
    QT_TR_NOOP( "Items" ),
    QT_TR_NOOP( "Outfits" ),
    QT_TR_NOOP( "Effects" ),
    QT_TR_NOOP( "Projectiles" ),
    QT_TR_NOOP( "Library" ),
    QT_TR_NOOP( "External" )
};

struct ItemProperty {
    quint8 header;
    QVector<quint16> childU16;
    QVector<quint8> childU8;

    ItemProperty( quint8 _header ) {
        header = _header;
    };
    ItemProperty( void ) {
        header = 0xFF;
    };
    ~ItemProperty( void ) {
        childU8.clear();
        childU16.clear();
    };

    bool isValid( void ) const {
        return header != 0xFF;
    };

    bool operator<( const ItemProperty& other ) const {
        return header < other.header;
    }
    bool operator==( const ItemProperty& other ) const {
        return header == other.header;
    }
};

typedef QList<ItemProperty> PropertyList;

class ItemData
{
public:
    ItemData() {
        reset();
    };
    ItemData( const ItemData& other ) {
        copy( other );
    };
    ~ItemData() {
        properties.clear();
        localMap.clear();
        resourceHash.clear();
    };

    void copy( const ItemData& other ) {
        parent          = other.parent;
        type            = other.type;
        width           = other.width;
        height          = other.height;
        cropsize        = other.cropsize;
        blendframes     = other.blendframes;
        xdiv            = other.xdiv;
        ydiv            = other.ydiv;
        zdiv            = other.zdiv;
        animcount       = other.animcount;
        properties      = other.properties;
        localMap        = other.localMap;
        resourceHash    = other.resourceHash;
    }

    bool isValid( void ) const;
    bool isNull( void ) const;

    void reset( void ) {
        parent      = ITEM_PARENT_INTERNAL;
        type        = ITEM_TYPE_NONE;
        width       = 1;
        height      = 1;
        cropsize    = 0;
        blendframes = 1;
        xdiv        = 1;
        ydiv        = 1;
        zdiv        = 1;
        animcount   = 1;
        properties.clear();
        localMap.clear();
        resourceHash.clear();
    }

    bool addProperty( const ItemProperty& itemProperty );
    bool removeProperty( const ItemProperty& itemProperty );

    void setSpriteResource( const quint32 frame, SharedResource& src );
    SharedResource getSpriteResource( const quint32 frame ) const;

    quint32 getSpriteCount( bool zfactor = true ) const;

    void setLocalSprite( const quint32 frame, const quint32 spriteId );
    quint32 getLocalSprite( const quint32 frame ) const;

    PropertyList getProperties( void ) const {
        return properties;
    };
    void setProperties( const PropertyList& _properties ) {
        properties.clear();
        properties = _properties;
    };

    ResourceHash getResources( void ) const {
        return resourceHash;
    };
    void setResources( const ResourceHash& _resources ) {
        resourceHash.clear();
        resourceHash = _resources;
    };

    LocalMap getLocalResources( void ) const {
        return localMap;
    };
    void setLocalResources( const LocalMap& _localMap ) {
        localMap.clear();
        localMap = _localMap;
    };

    PropertyList::iterator getPropertiesBegin() {
        return properties.begin();
    };
    PropertyList::iterator getPropertiesEnd() {
        return properties.end();
    };

    PropertyList::const_iterator getPropertiesBegin() const {
        return properties.begin();
    };
    PropertyList::const_iterator getPropertiesEnd() const {
        return properties.end();
    };

    quint8 parent;
    quint8 type;
    quint8 width;
    quint8 height;
    quint8 cropsize;
    quint8 blendframes;
    quint8 xdiv;
    quint8 ydiv;
    quint8 zdiv;
    quint8 animcount;

    PropertyList properties;
    LocalMap localMap;
    ResourceHash resourceHash;
};

class ItemFile;

class TibiaItem : public TibiaObject
{
public:
    TibiaItem( void );
    TibiaItem( const TibiaItem& other );
    TibiaItem( const ItemData& data );
    virtual ~TibiaItem( void );
    static TibiaItem *createItem( quint8 type, quint8 height = 1, quint8 width = 1, quint8 layers = 1, quint8 xpattern = 1, quint8 ypattern = 1, quint8 zpattern = 1, quint8 animations = 1, bool addons = false );
    static quint32 getSpriteFrame( const ItemData& itemData, quint8 divx, quint8 divy, quint8 divz, quint8 frame, quint32 x, quint32 y, quint8 animation );

    bool operator==( const TibiaItem& other ) const {
        return ( d->type == other.d->type &&
                 d->width == other.d->width &&
                 d->height == other.d->height &&
                 d->cropsize == other.d->cropsize &&
                 d->blendframes == other.d->blendframes &&
                 d->xdiv == other.d->xdiv &&
                 d->ydiv == other.d->ydiv &&
                 d->zdiv == other.d->zdiv &&
                 d->animcount == other.d->animcount &&
                 d->properties == other.d->properties &&
                 d->localMap == other.d->localMap &&
                 d->resourceHash == other.d->resourceHash );
    }

    virtual void reset( void );

    quint8 getParentType( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->parent;
    };
    quint8 getType( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->type;
    };
    quint8 getWidth( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->width;
    };
    quint8 getHeight( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->height;
    };
    quint8 getCropsize( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->cropsize;
    };
    quint8 getLayers( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->blendframes;
    };
    quint8 getXDiv( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->xdiv;
    };
    quint8 getYDiv( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->ydiv;
    };
    quint8 getZDiv( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->zdiv;
    };
    quint8 getAnimation( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->animcount;
    };

    void setParentType( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->parent = value;
    };
    void setType( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->type = value;
    };
    void setWidth( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->width = value;
    };
    void setHeight( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->height = value;
    };
    void setCropsize( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->cropsize = value;
    };
    void setLayers( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->blendframes = value;
    };
    void setXDiv( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->xdiv = value;
    };
    void setYDiv( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->ydiv = value;
    };
    void setZDiv( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->zdiv = value;
    };
    void setAnimation( quint8 value ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->animcount = value;
    };

    PropertyList getProperties( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->getProperties();
    };
    void setProperties( const PropertyList& _properties ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->setProperties( _properties );
    };

    PropertyList::iterator getPropertiesBegin() {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->getPropertiesBegin();
    };
    PropertyList::iterator getPropertiesEnd() {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->getPropertiesEnd();
    };

    PropertyList::const_iterator getPropertiesBegin() const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->getPropertiesBegin();
    };
    PropertyList::const_iterator getPropertiesEnd() const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->getPropertiesEnd();
    };

    bool hasHeader( quint8 header ) const;
    ItemProperty getProperty( quint8 header ) const;
    bool contains( const HeaderList& headerList ) const;

    bool contains( const PropertyList& propList ) const;
    bool hasProperty( const ItemProperty& itemProperty ) const;

    quint32 getSpriteCount( bool zfactor = true ) const;
    static QString typeName( quint8 type );
    static QString parentName( quint8 parent );
    static QString fullName( ItemFile *itemFile, TibiaItem *tibiaItem );

    ResourceHash getResources( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->getResources();
    };
    void setResources( const ResourceHash& _resources ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->setResources( _resources );
    };

    LocalMap getLocalResources( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->getLocalResources();
    };
    void setLocalResources( const LocalMap& _localMap ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->setLocalResources( _localMap );
    };

    void setLocalSprite( const quint32 frame, quint32 spriteId ) {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        d->setLocalSprite( frame, spriteId );
    }
    quint32 getLocalSprite( const quint32 frame ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->getLocalSprite( frame );
    }

    void setSpriteResource( const quint32 frame, SharedResource& src );
    SharedResource getSpriteResource( const quint32 frame ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->getSpriteResource( frame );
    };
    bool referencedSprite( const quint32 frame ) const;
    bool hasResources( void ) const {
        QMutex mutex;
        QMutexLocker locker( &mutex );
        return d->resourceHash.size() > 0;
    };

    bool isValid( void ) const {
        return d->isValid();
    };
    bool isNull( void ) const {
        return d->isNull();
    };
    bool hasEditorParent( void ) const {
        return ( d->parent == ITEM_PARENT_ITEMS || d->parent == ITEM_PARENT_OUTFITS || d->parent == ITEM_PARENT_EFFECTS || d->parent == ITEM_PARENT_PROJECTILES );
    };

    const ItemData& getItemData( void ) const {
        return *d;
    };

private:
    //QSharedDataPointer<ItemData> d;
    ItemData *d;

    friend class ItemAttributes;
    friend class ItemFile;
    friend class TibiaFile;
    friend class ExternalFile;
    friend class QDataStream;
};

typedef QList<TibiaItem *> ItemList;

QDataStream& operator<< ( QDataStream&, const ItemData& );
QDataStream& operator>> ( QDataStream&, ItemData& );
QDataStream& operator<< ( QDataStream&, const TibiaItem& );
QDataStream& operator>> ( QDataStream&, TibiaItem& );
QDataStream& operator<< ( QDataStream&, const ItemProperty& );
QDataStream& operator>> ( QDataStream&, ItemProperty& );

#endif // TIBIAITEM_H
