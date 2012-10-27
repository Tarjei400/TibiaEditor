#ifndef IMPORTTHREAD_H
#define IMPORTTHREAD_H

#include <QtCore/QMetaType>
#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtCore/QStringList>
#include "userthread.h"
#include "itemfile.h"
#include "resourcehandler.h"

typedef QList<QUrl> QUrlList;

enum ImportMode_t {
    IMPORT_NONE = 0,
    IMPORT_ITEMS_AND_SPRITES,
    IMPORT_ITEMS,
    IMPORT_SPRITES,
    IMPORT_PICTURES
};

class ImportThread : public UserThread
{
    Q_OBJECT

public:
    ImportThread( QObject *parent );
    virtual ~ImportThread( void );

    void setIndex( qint32 index ) {
        m_startIndex = index;
    };
    void setItemList( const ItemList& items ) {
        m_itemList = items;
    };
    void setSpriteList( const ResourceList& resources ) {
        m_spriteList = resources;
    };
    void setFilter( bool filter ) {
        m_filter = filter;
    };
    void setResourceType( quint8 type ) {
        m_resourceType = type;
    };
    void setMode( ImportMode_t mode ) {
        m_mode = mode;
    };
    void setDestination( quint8 dest ) {
        m_destination = dest;
    };

    ImportMode_t getMode( void ) const {
        return m_mode;
    };

    virtual void setup( void );

signals:
    void importItems( quint8, ItemList );
    void importSprites( quint8, quint32, ResourceList );

protected:
    virtual void run( void );

private:
    ImportMode_t m_mode;
    bool m_filter;
    quint8 m_destination;
    quint8 m_resourceType;
    qint32 m_startIndex;
    ItemList m_itemList;
    ResourceList m_spriteList;
};

enum DropMode_t {
    DROP_NONE = 0,
    DROP_ITEMS,
    DROP_SPRITES,
    DROP_PICTURES
};

class DropFileThread : public UserThread
{
    Q_OBJECT

public:
    DropFileThread( QObject *parent );
    virtual ~DropFileThread( void );

    virtual void setup( void );
    void setDropMode( DropMode_t dropMode ) {
        m_dropMode = dropMode;
    };
    void setUrlList( const QUrlList& urlList );
    void setStringList( const QStringList& strList );

signals:
    void dropItems( ItemList );
    void dropSprites( quint8, ResourceList );
    void dropTibiaFile( QFile * );

protected:
    virtual void run( void );

private:
    bool processFile( const QString& filePath, ItemList& items, ResourceList& sprites, ResourceList& pictures );
    void pathSearch( QStringList& validFiles, const QString& path );
    void recursiveSearch( QStringList& validFiles, const QDir& dir );

    DropMode_t m_dropMode;
    QStringList m_stringList;
    QUrlList m_urlList;
};

#endif // IMPORTTHREAD_H
