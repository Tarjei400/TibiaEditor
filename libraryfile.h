#ifndef LIBRARYFILE_H
#define LIBRARYFILE_H

#include <QtCore/QDataStream>

#include "tibiafile.h"
#include "itemfile.h"

typedef QHash<quint32, quint32> AddressHash;
typedef QHash<quint32, ItemData> ItemHash;

class UserThread;

class LibraryFile : public TibiaModule
{
    Q_OBJECT

public:
    LibraryFile( QObject *parent = NULL );
    LibraryFile( DatFormat *format, const ItemList& items, QObject *parent = NULL );
    virtual ~LibraryFile( void );

    virtual bool isLoaded( void ) const {
        return m_loaded;
    };
    virtual void unload( void );
    virtual bool load( const QString& ) {
        return false;
    };
    //virtual bool save( const QString& );
    virtual bool save( const QString& ) {
        return false;
    };
    virtual bool idle( const QString&, bool );
    virtual TibiaSprite getSprite( qint32, qint32 );
    ItemData getItemData( qint32 index );
    void setItemData( qint32 index, const ItemData& item );
    quint32 getCount( void ) const {
        return m_count;
    };
    void setVersion( quint16 version ) {
        m_version = version;
    };
    quint16 getVersion( void ) const {
        return m_version;
    };
    quint32 getItemsSize( void ) const {
        return m_items.size();
    };
    bool mustIdle( void ) const {
        return m_mustIdle;
    };

    UserThread *getThread() const {
        return m_thread;
    };

private:
    UserThread *m_thread;
    DatFormat *m_datFormat;
    bool m_loaded;
    bool m_mustIdle;
    quint16 m_version;
    quint32 m_count;

    ItemHash m_internalItems;
    ItemList m_items;

    AddressHash m_spriteHash;

    mutable QMutex mutex;
};

#endif // EXTERNALFILE_H
