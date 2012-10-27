#ifndef ITEMFILE_H
#define ITEMFILE_H

#include <QtCore/QMutex>
#include <QtCore/QDataStream>
#include "tibiafile.h"
#include "tibiaitem.h"

class DatFormat;
class UserThread;

class ItemFile : public TibiaFile
{
    Q_OBJECT

public:
    ItemFile( QObject *parent );
    virtual ~ItemFile( void );

    virtual bool createNew( void );
    virtual bool isLoaded( void ) const {
        return m_loaded;
    };
    virtual void unload( void );
    virtual bool load( const QString& );
    //virtual bool save( const QString& );
    virtual bool save( const QString& ) {
        return false;
    };
    virtual bool idle( const QString&, bool ) {
        return false;
    };

    virtual ItemFile *getItemFile() {
        return this;
    };
    virtual const ItemFile *getItemFile() const {
        return this;
    };

    void setFormat( DatFormat *format ) {
        m_datFormat = format;
    };
    DatFormat *getFormat( void ) const {
        return m_datFormat;
    };

    bool loadItemSection( QDataStream& in, quint32 minimum, quint32 maximum, ItemList& items, quint8 type, quint8 parent, quint32& count );
    static bool loadItem( DatFormat *datFormat, QDataStream& in, ItemData& itemData, QString& error, bool oldFormat = false );
    static bool loadItemProperties( DatFormat *datFormat, QDataStream& in, ItemData& itemData, QString& error );

    bool saveItemSection( QDataStream& out, ItemList& items );
    static bool saveItem( DatFormat *datFormat, QDataStream& out, const ItemData& itemData );
    static bool saveItemProperties( DatFormat *datFormat, QDataStream& out, const ItemData& itemData );

    qint32 getItemId( TibiaItem *item ) const;
    qint32 getItemIndex( TibiaItem *item ) const;
    qint32 getItemType( TibiaItem *item ) const;
    TibiaItem *getItemById( quint32 itemId ) const;
    TibiaItem *getItemByIndex( quint8 parent, qint32 index ) const;

    void setItemCount( quint16 items ) {
        QMutex mutex;
        mutex.lock();
        m_items = items;
        mutex.unlock();
    };
    void setOutfitCount( quint16 outfits ) {
        QMutex mutex;
        mutex.lock();
        m_outfits = outfits;
        mutex.unlock();
    };
    void setEffectCount( quint16 effects ) {
        QMutex mutex;
        mutex.lock();
        m_effects = effects;
        mutex.unlock();
    };
    void setProjectileCount( quint16 projectiles ) {
        QMutex mutex;
        mutex.lock();
        m_projectiles = projectiles;
        mutex.unlock();
    };

    quint32 getFirst() const {
        return m_begin;
    };
    quint32 getOutfitsBegin() const {
        return m_items+1;
    };
    quint32 getEffectsBegin() const {
        return m_items+m_outfits+1;
    };
    quint32 getProjectilesBegin() const {
        return m_items+m_outfits+m_effects+1;
    };
    quint32 getLast() const {
        return m_items+m_outfits+m_effects+m_projectiles;
    };

    quint16 getItemCount( void ) const {
        return m_items;
    };
    quint16 getOutfitCount( void ) const {
        return m_outfits;
    };
    quint16 getEffectCount( void ) const {
        return m_effects;
    };
    quint16 getProjectileCount( void ) const {
        return m_projectiles;
    };

    ItemList& getItems( void ) {
        return m_itemList;
    };
    ItemList& getOutfits( void ) {
        return m_outfitList;
    };
    ItemList& getEffects( void ) {
        return m_effectList;
    };
    ItemList& getProjectiles( void ) {
        return m_projectileList;
    };

    bool replace( TibiaItem *item, TibiaItem *replaced );
    bool remove( TibiaItem *item );
    bool insert( quint8 parent, qint32 index, TibiaItem *item );

    UserThread *getThread( void ) const {
        return m_thread;
    };

signals:
    void replaceItem( qint32, TibiaItem * );
    void replaceOutfit( qint32, TibiaItem * );
    void replaceEffect( qint32, TibiaItem * );
    void replaceProjectile( qint32, TibiaItem * );

    void removeItem( qint32, TibiaItem * );
    void removeOutfit( qint32, TibiaItem * );
    void removeEffect( qint32, TibiaItem * );
    void removeProjectile( qint32, TibiaItem * );

    void insertItem( qint32, TibiaItem * );
    void insertOutfit( qint32, TibiaItem * );
    void insertEffect( qint32, TibiaItem * );
    void insertProjectile( qint32, TibiaItem * );

private:
    UserThread *m_thread;
    DatFormat *m_datFormat;

    ItemList m_itemList;
    ItemList m_outfitList;
    ItemList m_effectList;
    ItemList m_projectileList;

    quint16 m_items;
    quint16 m_begin;
    quint16 m_outfits;
    quint16 m_effects;
    quint16 m_projectiles;

    bool m_loaded;
};

#endif // ITEMFILE_H
