#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>
#include "resourcehandler.h"
#include "tibiasprite.h"
#include "resourcefile.h"

ResourceHandler::ResourceHandler( void ) : QObject( NULL )
{

}

ResourceHandler::~ResourceHandler( void )
{
    m_localSprites.clear();
    m_localPictures.clear();
    m_foreignSprites.clear();
    m_foreignPictures.clear();
}

SharedResource ResourceHandler::getResourceByPointer( TibiaResource *resource )
{
    QHashIterator<quint32, SharedResource> localIt( m_localSprites );
    while( localIt.hasNext() ) {
        localIt.next();
        if( localIt.value() == resource )
            return localIt.value();
    }

    QHashIterator<quint32, SharedResource> localIt2( m_localPictures );
    while( localIt2.hasNext() ) {
        localIt2.next();
        if( localIt2.value() == resource )
            return localIt2.value();
    }

    QListIterator<SharedResource> foreignIt( m_foreignSprites );
    while( foreignIt.hasNext() ) {
        SharedResource src = foreignIt.next();
        if( src == resource )
            return src;
    }

    QListIterator<SharedResource> foreignIt2( m_foreignPictures );
    while( foreignIt2.hasNext() ) {
        SharedResource src = foreignIt2.next();
        if( src == resource )
            return src;
    }

    return SharedResource();
}

SharedResource ResourceHandler::getResourceByData( quint8 resourceType, quint32 identifier, quint16 frame, const QString& fileName )
{
    ResourceHash *localResources;
    ResourceList *foreignList;

    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        localResources = &m_localSprites;
        foreignList = &m_foreignSprites;
        break;
    case RESOURCE_TYPE_PICTURE:
        localResources = &m_localPictures;
        foreignList = &m_foreignPictures;
        break;
    }

    QHashIterator<quint32, SharedResource> localIt( *localResources );
    while( localIt.hasNext() ) {
        localIt.next();
        if( localIt.value()->getIdentifier() == identifier &&
                localIt.value()->getFrame() == frame &&
                ( ( localIt.value()->getFile() &&
                    localIt.value()->getFile()->fileName() == fileName ) || !localIt.value()->getFile() ) )
            return localIt.value();
    }

    QListIterator<SharedResource> foreignIt( *foreignList );
    while( foreignIt.hasNext() ) {
        SharedResource src = foreignIt.next();
        if( src->getIdentifier() == identifier &&
                src->getFrame() == frame &&
                ( ( src->getFile() &&
                    src->getFile()->fileName() == fileName ) || !src->getFile() ) )
            return src;
    }

    // If filename != empty, attempt to guess file format and load resource

    return SharedResource();
}

SharedResource ResourceHandler::addResource( quint8 resourceType, quint32 identifier, quint16 frame, QFile *file )
{
    ResourceList *foreignResources;
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        foreignResources = &m_foreignSprites;
        break;
    case RESOURCE_TYPE_PICTURE:
        foreignResources = &m_foreignPictures;
        break;
    }

    SharedResource reference(new TibiaResource( resourceType, identifier, frame, file));
    foreignResources->push_back(reference);
    return reference;
}

SharedResource ResourceHandler::addResource( quint32 resourceType, SharedResource& resource )
{
    ResourceList *foreignResources;
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        foreignResources = &m_foreignSprites;
        break;
    case RESOURCE_TYPE_PICTURE:
        foreignResources = &m_foreignPictures;
        break;
    }

    foreignResources->push_back( resource );
    return resource;
}

void ResourceHandler::removeResourcesByFile( quint8 resourceType, QFile *file, bool local, bool foreign )
{
    ResourceHash *localResources;
    ResourceList *foreignList;

    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        localResources = &m_localSprites;
        foreignList = &m_foreignSprites;
        break;
    case RESOURCE_TYPE_PICTURE:
        localResources = &m_localPictures;
        foreignList = &m_foreignPictures;
        break;
    }

    if( local ) {
        QMutableHashIterator<quint32, SharedResource> localIt( *localResources );
        localIt.toBack();
        while( localIt.hasPrevious() ) {
            localIt.previous();
            if( localIt.value()->getFile() == file )
                localIt.remove();
        }
    }
    if( foreign ) {
        QMutableListIterator<SharedResource> foreignIt( *foreignList );
        foreignIt.toBack();
        while( foreignIt.hasPrevious() ) {
            SharedResource previous = foreignIt.previous();
            if( previous->getFile() == file )
                foreignIt.remove();
        }
    }
}

void ResourceHandler::swapResources( SharedResource& fromResource, SharedResource& toResource )
{
    quint8 fromType = fromResource->getType();
    quint32 fromId = fromResource->getIdentifier();
    quint16 fromFrame = fromResource->getFrame();
    QFile *fromFile = fromResource->getFile();

    quint8 toType = toResource->getType();
    quint32 toId = toResource->getIdentifier();
    quint16 toFrame = toResource->getFrame();
    QFile *toFile = toResource->getFile();

    fromResource->resetCache();
    fromResource->setType( toType );
    fromResource->setIdentifier( toId );
    fromResource->setFrame( toFrame );
    fromResource->setFile( toFile, false );

    toResource->resetCache();
    toResource->setType( fromType );
    toResource->setIdentifier( fromId );
    toResource->setFrame( fromFrame );
    toResource->setFile( fromFile, false );
}

void ResourceHandler::cleanupLocalResources( quint8 resourceType )
{
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        m_localSprites.clear();
        break;

    case RESOURCE_TYPE_PICTURE:
        m_localPictures.clear();
        break;
    }
}

SharedResource ResourceHandler::loadLocalResource( quint8 resourceType, quint32 identifier, bool create )
{
    ResourceHash *localResources;
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        localResources = &m_localSprites;
        break;
    case RESOURCE_TYPE_PICTURE:
        localResources = &m_localPictures;
        break;
    }

    if( localResources->contains( identifier ) )
        return localResources->value( identifier );

    if( create ) {
        SharedResource reference = SharedResource( new TibiaResource( resourceType, identifier ) );
        localResources->insert( identifier, reference );
    }

    return localResources->value( identifier );
}

TibiaSprite ResourceHandler::getSpriteByResource( SharedResource resource )
{
    if( resource )
        return resource->getTibiaSprite();

    return dummy;
}

/*void ResourceHandler::importItemListResources( quint32 spriteStart, ItemList& items, QList<LocalMap>& localMapList, ResourceList& additionList, bool filter )
{
    QList<QList<quint32>> duplicateList;
    QList<ResourceHash> resourceHashList;

    for( ItemList::iterator it = items.begin(); it != items.end(); it++ )
    {
        localMapList.push_back( (*it)->getLocalResources() );
        resourceHashList.push_back( (*it)->getResources() );
        duplicateList.push_back( QList<quint32>() );
    }

    if( filter )
    {
        // Gather all sprites
        SpriteHash localHash;
        QHashIterator<quint32, SharedResource> localIt( m_localSprites );
        while( localIt.hasNext() )
        {
            localIt.next();
            TibiaSprite localSprite = localIt.value()->getTibiaSprite();
            if( localSprite.isDummy() )
                continue;

            localHash.insert( localIt.key(), localSprite );
        }

        // Gather list of lists worth of item sprites
        QList<SpriteHash> itemHashList;
        QListIterator<ResourceHash> hashList( resourceHashList );
        while( hashList.hasNext() )
        {
            SpriteHash listHash;
            QHashIterator<quint32, SharedResource> listIt( hashList.next() );
            while( listIt.hasNext() )
            {
                listIt.next();
                TibiaSprite listSprite = listIt.value()->getTibiaSprite();
                if( listSprite.isDummy() )
                    continue;

                listHash.insert( listIt.key(), listSprite );
            }

            itemHashList.push_back( listHash );
        }


        // Iterate main sprite list
        QHashIterator<quint32, TibiaSprite> spriteListIt( localHash );
        while( spriteListIt.hasNext() )
        {
            spriteListIt.next();

            quint32 localMapIndex = 0;
            QList<quint32> dupeList;
            // Iterate list of sprite lists
            QListIterator<SpriteHash> itemListIt( itemHashList );
            while( itemListIt.hasNext() )
            {
                // Iterate sprite list
                QHashIterator<quint32, TibiaSprite> itemIt( itemListIt.next() );
                while( itemIt.hasNext() )
                {
                    itemIt.next();

                    // Compare sprite with main sprite, push into duplicates if exists, write our new ID to the localMap of this specific index
                    if( spriteListIt.value().image == itemIt.value().image )
                    {
                        localMapList[localMapIndex].insert( itemIt.key(), spriteListIt.key() );
                        duplicateList[localMapIndex].push_back( itemIt.key() );
                    }
                }

                localMapIndex++;
            }
        }

        localHash.clear();
        itemHashList.clear();
    }

    quint32 resourceHashIndex = 0;

    // Iterate list of duplicate lists
    QListIterator<QList<quint32>> dupeListIt( duplicateList );
    while( dupeListIt.hasNext() )
    {
        // Iterate duplicate list
        QListIterator<quint32> dupeIt( dupeListIt.next() );
        while( dupeIt.hasNext() ){
            resourceHashList[resourceHashIndex].remove( dupeIt.next() ); // Remove duplicate from specific hashlist index
        }

        resourceHashIndex++;
    }

    duplicateList.clear();

    quint32 addition = 0;
    resourceHashIndex = 0;

    blockSignals( true ); // Block update signals

    // Iterate list of resource lists
    QListIterator<ResourceHash> addItList( resourceHashList );
    while( addItList.hasNext() )
    {
        // Iterate resource list
        QHashIterator<quint32, SharedResource> addIt( addItList.next() );
        while( addIt.hasNext() )
        {
            addIt.next();
            localMapList[resourceHashIndex].insert( addIt.key(), spriteStart + addition ); // Add the new id of the newly local resource
            additionList.push_back( addIt.value() ); // Add the resource to the total addition list
            addition++;
        }

        resourceHashIndex++;
    }
    blockSignals( false ); // Unblock update signals
    resourceHashList.clear();
}*/

const quint32 ResourceHandler::getDisplayIdentifier( SharedResource& resource ) const
{
    // 0xFFFFFFFF - Unknown Identifier
    // 0xFFFF0000 - Source Indicator
    quint32 identifier = 0xFFFFFFFF;
    switch( resource->getType() ) {
    case RESOURCE_TYPE_SPRITE:
        identifier = m_localSprites.key( resource, 0xFFFFFFFF );
        break;
    case RESOURCE_TYPE_PICTURE:
        identifier = m_localPictures.key( resource, 0xFFFFFFFF );
        break;
    }

    if( identifier == 0xFFFFFFFF ) { // Identifier not found in list
        identifier = resource->getIdentifier(); // Given Identifier
        if( identifier == 0xFFFF0000 ) // Given a source
            return identifier;
    }

    return identifier;
}

void ResourceHandler::addLocalResources( quint8 resourceType, quint32 identifierStart, ResourceList& resources )
{
    ResourceHash *localResources;
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        localResources = &m_localSprites;
        break;
    case RESOURCE_TYPE_PICTURE:
        localResources = &m_localPictures;
        break;
    }

    quint32 offset = 0;
    QListIterator<SharedResource> it( resources );
    while( it.hasNext() ) {
        localResources->insert( identifierStart + offset, it.next() );
        offset++;
    }
}

void ResourceHandler::removeLocalResources( quint8 resourceType, ResourceList& resources )
{
    ResourceHash *localResources;
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        localResources = &m_localSprites;
        break;
    case RESOURCE_TYPE_PICTURE:
        localResources = &m_localPictures;
        break;
    }

    QMutableHashIterator<quint32, SharedResource> localIt( *localResources );
    localIt.toBack();
    QListIterator<SharedResource> it( resources );
    while( it.hasNext() ) {
        SharedResource resource = it.next();
        if( localIt.findPrevious( resource ) )
            localIt.remove();
        localIt.toBack();
    }
}

void ResourceHandler::addForeignResources( quint8 resourceType, ResourceList& resources )
{
    ResourceList *foreignList;
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        foreignList = &m_foreignSprites;
        break;
    case RESOURCE_TYPE_PICTURE:
        foreignList = &m_foreignPictures;
        break;
    }

    foreignList->append( resources );
}

void ResourceHandler::removeForeignResources( quint8 resourceType, ResourceList& resources )
{
    ResourceList *foreignList;
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        foreignList = &m_foreignSprites;
        break;
    case RESOURCE_TYPE_PICTURE:
        foreignList = &m_foreignPictures;
        break;
    }

    QMutableListIterator<SharedResource> foreignIt( *foreignList );
    foreignIt.toBack();
    while( foreignIt.hasPrevious() ) {
        SharedResource previous = foreignIt.previous();
        QListIterator<SharedResource> it( resources );
        while( it.hasNext() ) {
            SharedResource resource = it.next();
            if( previous == resource )
                foreignIt.remove();
        }
    }
}

SharedResource ResourceHandler::createLocalResource( quint8 resourceType, quint32 identifier, const TibiaSprite& tibiaSprite )
{
    QDir dir( QCoreApplication::applicationDirPath() );
    if( dir.exists() ) {
        if( !dir.cd( tr( "Resources" ) ) )
            dir.mkdir( tr( "Resources" ) );

        dir.cd( tr( "Resources" ) );
    }

    return SharedResource( new TibiaResource( resourceType, identifier, 0, new ResourceFile( dir, identifier, tibiaSprite ) ) );
}

// Called after SpriteFile/PictureFile save completed, dump all foreign resources aswell
void ResourceHandler::reinitializeLocalResources( quint8 resourceType )
{
    // Unhook all local resources set them to defaults (id, 0, NULL)
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE: {
        QHashIterator<quint32, SharedResource> itM( m_localSprites );
        while( itM.hasNext() ) {
            itM.next();
            itM.value()->reset(); // Unhook files here
        }
    }
    break;

    case RESOURCE_TYPE_PICTURE: {
        QHashIterator<quint32, SharedResource> itM( m_localPictures );
        while( itM.hasNext() ) {
            itM.next();
            itM.value()->reset(); // Unhook files here
        }
    }
    break;
    }
}

ResourceList& ResourceHandler::getForeignSprites( void )
{
    return m_foreignSprites;
}

ResourceList& ResourceHandler::getForeignPictures( void )
{
    return m_foreignPictures;
}

ResourceHash& ResourceHandler::getLocalSprites( void )
{
    return m_localSprites;
}

ResourceHash& ResourceHandler::getLocalPictures( void )
{
    return m_localPictures;
}
