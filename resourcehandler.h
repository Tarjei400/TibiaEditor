#ifndef RESOURCEHANDLER_H
#define RESOURCEHANDLER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QHash>

#include "tibiaresource.h"
#include "tibiaitem.h"

typedef QList<SharedResource> ResourceList;

#define RESOURCE_TYPE_SPRITE 0
#define RESOURCE_TYPE_PICTURE 1

class ResourceHandler : public QObject
{
    Q_OBJECT

public:
    ResourceHandler( void );
    ~ResourceHandler( void );

    SharedResource addResource( quint8 resourceType, quint32 identifier, quint16 frame, QFile *file );
    SharedResource addResource( quint32 resourceType, SharedResource& resource );

    void removeResourcesByFile( quint8 resourceType, QFile *file, bool local = true, bool foreign = true );

    void cleanupLocalResources( quint8 );
    SharedResource createLocalResource( quint8 resourceType, quint32 identifier, const TibiaSprite& tibiaSprite );
    //SharedResource createLocalResource( quint8 resourceType, quint32 identifier, const TibiaSprite& tibiaSprite, SharedResource& oldResource );
    SharedResource loadLocalResource( quint8 resourceType, quint32 identifier, bool create );
    void reinitializeLocalResources( quint8 );
    //quint32 importResources( quint32 spriteStart, LocalMap& localMap, ResourceHash& resourceHash, bool filter = true, bool create = false );
    //void importItemListResources( quint32 spriteStart, ItemList& items, QList<LocalMap>& localMapList, ResourceList& additionList, bool filter );
    //bool makeResourceLocal( quint8 resourceType, quint32 identifier, SharedResource resource, SharedResource& oldResource, bool create = true );


    const quint32 getDisplayIdentifier( SharedResource& resource ) const;

    void swapResources( SharedResource& fromResource, SharedResource& toResource );

    /*void moveToLocal( quint8 resourceType, quint32 identifierStart, ResourceList& resources );
    void moveToForeign( quint8 resourceType, ResourceList& resources );
    void removeFromLocal( quint8 resourceType, ResourceList& resources );*/

    void addLocalResources( quint8 resourceType, quint32 identifierStart, ResourceList& resources );
    void removeLocalResources( quint8 resourceType, ResourceList& resources );
    void addForeignResources( quint8 resourceType, ResourceList& resources );
    void removeForeignResources( quint8 resourceType, ResourceList& resources );

    SharedResource getResourceByPointer( TibiaResource *resource );
    SharedResource getResourceByData( quint8 resourceType, quint32 identifier, quint16 frame, const QString& fileName );
    TibiaSprite getSpriteByResource( SharedResource resource );

    ResourceList& getForeignSprites( void );
    ResourceList& getForeignPictures( void );
    ResourceHash& getLocalSprites( void );
    ResourceHash& getLocalPictures( void );

signals:
    void resourcesRemoved( quint8, quint32 );
    void resourcesAdded( quint8, ResourceList& resources );
    void resourceAdded( quint8, SharedResource&, qint32 );
    void resourceUpdated( quint8, qint32, SharedResource& );

private:
    ResourceList m_foreignSprites;
    ResourceList m_foreignPictures;

    ResourceHash m_localSprites;
    ResourceHash m_localPictures;

    friend class CommandImportSprites;
    friend class CommandSwapSprite;
};

#endif // RESOURCEHANDLER_H
