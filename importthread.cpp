#include <QtCore/QTime>
#include "importthread.h"
#include "externalfile.h"
#include "libraryfile.h"

extern ResourceHandler g_resourceHandler;

ImportThread::ImportThread( QObject *parent ) : UserThread(parent)
{

}

ImportThread::~ImportThread( void )
{
    m_itemList.clear();
    m_spriteList.clear();
}

void ImportThread::setup( void )
{
    UserThread::setup();
    m_startIndex = -1;
    m_mode = IMPORT_NONE;
    m_filter = false;
    m_itemList.clear();
    m_spriteList.clear();
}

void ImportThread::run( void )
{
    QTime time;
    time.start();
    if( m_mode == IMPORT_ITEMS_AND_SPRITES ) { // Item import with sprites
        QList<LocalMap> localMapList;
        QList<QList<quint32> > duplicateList;
        QList<ResourceHash> resourceHashList;

        setLabel( tr( "Calculating sizes..." ) );
        for( ItemList::iterator it = m_itemList.begin(); it != m_itemList.end(); it++ ) {
            localMapList.push_back( (*it)->getLocalResources() );
            resourceHashList.push_back( (*it)->getResources() );
            duplicateList.push_back( QList<quint32>() );
        }

        quint32 totalSprites = 0;
        QListIterator<ResourceHash> hashList( resourceHashList );
        while( hashList.hasNext() ) {
            totalSprites += hashList.next().size();
        }

        if( m_filter ) {
            setLabel( tr( "Building Sprite List..." ) );
            setMinimum( 0 );
            setMaximum( g_resourceHandler.getLocalSprites().size() );
            // Gather all sprites
            int index = 0;
            SpriteHash localHash;
            QHashIterator<quint32, SharedResource> localIt( g_resourceHandler.getLocalSprites() );
            while( localIt.hasNext() ) {
                if( isCanceled() )
                    return;
                localIt.next();
                TibiaSprite localSprite = localIt.value()->getTibiaSprite();
                if( localSprite.isDummy() ) {
                    emit valueChanged( index );
                    index++;
                    continue;
                }

                localHash.insert( localIt.key(), localSprite );
                emit valueChanged( index );
                index++;
            }

            setLabel( tr( "Building Item Sprite List..." ) );
            setMinimum( 0 );
            setMaximum( totalSprites );
            index = 0;
            // Gather list of lists worth of item sprites
            QList<SpriteHash> itemHashList;
            QListIterator<ResourceHash> hashList( resourceHashList );
            while( hashList.hasNext() ) {
                if( isCanceled() )
                    return;
                SpriteHash listHash;
                QHashIterator<quint32, SharedResource> listIt( hashList.next() );
                while( listIt.hasNext() ) {
                    listIt.next();
                    TibiaSprite listSprite = listIt.value()->getTibiaSprite();
                    if( listSprite.isDummy() ) {
                        emit valueChanged( index );
                        index++;
                        continue;
                    }

                    listHash.insert( listIt.key(), listSprite );
                    emit valueChanged( index );
                    index++;
                }

                itemHashList.push_back( listHash );
            }


            setLabel( tr( "Comparing sprites..." ) );
            // Iterate main sprite list
            QHashIterator<quint32, TibiaSprite> spriteListIt( localHash );
            while( spriteListIt.hasNext() ) {
                if( isCanceled() )
                    return;
                spriteListIt.next();

                quint32 localMapIndex = 0;
                QList<quint32> dupeList;
                // Iterate list of sprite lists
                QListIterator<SpriteHash> itemListIt( itemHashList );
                while( itemListIt.hasNext() ) {
                    // Iterate sprite list
                    QHashIterator<quint32, TibiaSprite> itemIt( itemListIt.next() );
                    while( itemIt.hasNext() ) {
                        itemIt.next();

                        // Compare sprite with main sprite, push into duplicates if exists, write our new ID to the localMap of this specific index
                        if( spriteListIt.value().image == itemIt.value().image ) {
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
        QListIterator<QList<quint32> > dupeListIt( duplicateList );
        while( dupeListIt.hasNext() ) {
            if( isCanceled() )
                return;
            // Iterate duplicate list
            QListIterator<quint32> dupeIt( dupeListIt.next() );
            while( dupeIt.hasNext() ) {
                if( isCanceled() )
                    return;
                resourceHashList[resourceHashIndex].remove( dupeIt.next() ); // Remove duplicate from specific hashlist index
            }

            resourceHashIndex++;
        }

        duplicateList.clear();

        quint32 addition = 0;
        resourceHashIndex = 0;

        // Iterate list of resource lists
        QListIterator<ResourceHash> addItList( resourceHashList );
        while( addItList.hasNext() ) {
            if( isCanceled() )
                return;
            // Iterate resource list
            QHashIterator<quint32, SharedResource> addIt( addItList.next() );
            while( addIt.hasNext() ) {
                if( isCanceled() )
                    return;
                addIt.next();
                localMapList[resourceHashIndex].insert( addIt.key(), m_startIndex + addition ); // Add the new id of the newly local resource
                m_spriteList.push_back( addIt.value() ); // Add the resource to the total addition list
                addition++;
            }

            resourceHashIndex++;
        }

        resourceHashList.clear();

        quint32 index = 0;
        foreach( LocalMap localMap, localMapList ) {
            if( isCanceled() )
                return;
            if( TibiaItem *tibiaItem = m_itemList.at( index ) ) {
                tibiaItem->setLocalResources( localMap );
                tibiaItem->setResources( ResourceHash() );
            }

            index++;
        }

        emit importSprites( m_resourceType, m_startIndex, m_spriteList );
        emit importItems( m_destination, m_itemList );
        emit success( time.elapsed() );
    }
    if( m_mode == IMPORT_ITEMS ) { // Purely item importing
        setLabel( tr( "Importing Items..." ) );
        setMinimum( 0 );
        setMaximum( m_itemList.size() );

        emit importItems( m_destination, m_itemList );
        emit success( time.elapsed() );
    }
    if( m_mode == IMPORT_SPRITES || m_mode == IMPORT_PICTURES ) { // Purely sprite importing
        setLabel( tr( "Importing Sprites..." ) );
        setMinimum( 0 );
        setMaximum( m_spriteList.size() );

        emit importSprites( m_resourceType, m_startIndex, m_spriteList );
        emit success( time.elapsed() );
    }
}

DropFileThread::DropFileThread( QObject *parent ) : UserThread(parent)
{

}

DropFileThread::~DropFileThread( void )
{
    m_dropMode = DROP_NONE;
    m_urlList.clear();
    m_stringList.clear();
}

void DropFileThread::setup( void )
{
    UserThread::setup();
    m_dropMode = DROP_NONE;
    m_urlList.clear();
    m_stringList.clear();
}

void DropFileThread::setUrlList(const QUrlList& urlList )
{
    m_urlList = urlList;
}
void DropFileThread::setStringList(const QStringList& strList )
{
    m_stringList = strList;
}

void DropFileThread::pathSearch( QStringList& validFiles, const QString& path )
{
    QFileInfo fileInfo( path );
    if( fileInfo.exists() ) {
        if( fileInfo.isDir() )
            recursiveSearch( validFiles, QDir( path ) );
        else
            validFiles.push_back( path );
    }
}

void DropFileThread::run( void )
{
    QTime time;
    time.start();

    QStringList paths;
    if( !m_urlList.isEmpty() ) {
        setLabel( tr( "Searching Paths..." ) );
        setMinimum( 0 );
        setMaximum( m_urlList.size() );
        qint32 i = 0;
        foreach( QUrl url, m_urlList ) {
            pathSearch( paths, url.toLocalFile() );
            i++;
            emit valueChanged( i );
        }
    }
    if( !m_stringList.isEmpty() ) {
        setLabel( tr( "Searching Paths..." ) );
        setMinimum( 0 );
        setMaximum( m_stringList.size() );
        qint32 i = 0;
        foreach( QString str, m_stringList ) {
            pathSearch( paths, str );
            i++;
            emit valueChanged( i );
        }
    }

    setLabel( tr( "Processing files..." ) );
    setMinimum( 0 );
    setMaximum( paths.size() );

    ItemList items;
    ResourceList sprites;
    ResourceList pictures;

    QTime rate;
    rate.start();
    quint32 count = 0;
    qint32 i;
    for( i = 0; i < paths.size(); i++ ) {
        if( isCanceled() )
            break;

        processFile( paths.at( i ), items, sprites, pictures );

        emit valueChanged( i );

        if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
            emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( m_maximum - i ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
            rate.restart();
            count = 0;
        }

        count++;
    }

    if( !items.isEmpty() )
        emit dropItems( items );
    if( !sprites.isEmpty() )
        emit dropSprites( RESOURCE_TYPE_SPRITE, sprites );
    if( !pictures.isEmpty() )
        emit dropSprites( RESOURCE_TYPE_PICTURE, pictures );

    if( !isCanceled() && i > 0 )
        emit success( time.elapsed() );
}

void DropFileThread::recursiveSearch( QStringList& validFiles, const QDir& dir )
{
    QStringList files = dir.entryList();
    files.removeOne(".");
    files.removeOne("..");
    foreach( QString str, files )
    pathSearch( validFiles, dir.path() + "/" + str );
}

bool DropFileThread::processFile( const QString& filePath, ItemList& items, ResourceList& sprites, ResourceList& pictures )
{
    QFileInfo fileInfo( filePath );
    QString fileType = fileInfo.suffix();

    bool dropSprite = ( m_dropMode == DROP_SPRITES );
    bool dropPicture = ( m_dropMode == DROP_PICTURES );
    bool dropItem = ( m_dropMode == DROP_ITEMS );

    bool isPNG = ( fileType.compare( tr( "png" ), Qt::CaseInsensitive ) == 0 );
    bool isBMP = ( fileType.compare( tr( "bmp" ), Qt::CaseInsensitive ) == 0 );
    bool isIDF = ( fileType.compare( tr( "idf" ), Qt::CaseInsensitive ) == 0 || fileType.compare( tr( "idc" ), Qt::CaseInsensitive ) == 0 );
    bool isIDLF = ( fileType.compare( tr( "idlf" ), Qt::CaseInsensitive ) == 0 );

    if( ( dropSprite || dropPicture ) && !isPNG && !isBMP ) {
        emit documentError( filePath, tr( "Invalid extension, must be PNG or BMP." ), -1 );
        return false;
    }
    if( dropItem && !isIDF && !isIDLF ) {
        emit documentError( filePath, tr( "Invalid extension, must be IDF or IDLF." ), -1 );
        return false;
    }

    if( dropSprite || dropPicture ) {
        if( isPNG || isBMP ) {
            QImage verifiedImage( filePath, fileType.toLatin1() );
            if( verifiedImage.width() % 32 == 0 && verifiedImage.height() % 32 == 0 ) {
                if( verifiedImage.isNull() ) {
                    emit documentError( filePath, tr( "Could not be loaded." ), -1 );
                    return false;
                }

                quint32 x = verifiedImage.width() / 32;
                quint32 y = verifiedImage.height() / 32;

                if( x > 0xFF || y > 0xFF ) {
                    emit documentError( filePath, tr( "Dimensions too large." ), -1 );
                    return false;
                } else if( x < 1 || y < 1 ) {
                    emit documentError( filePath, tr( "Dimensions too small." ), -1 );
                    return false;
                }

                if( dropSprite ) {
                    // Reverse loop
                    quint8 real_x = 0;
                    quint8 real_y = 0;
                    quint8 dx = x - 1;
                    quint8 dy = y - 1;
                    for( real_x = 0; real_x < x; real_x++ ) {
                        dy = y - 1;
                        for( real_y = 0; real_y < y; real_y++ ) {
                            TibiaSprite newSprite( -1 );
                            newSprite.setImage( verifiedImage.copy( real_x * 32, real_y * 32, 32, 32 ) );
                            newSprite.setSource( fileInfo.fileName() );
                            sprites.push_back( g_resourceHandler.createLocalResource( RESOURCE_TYPE_SPRITE, 0xFFFF, newSprite ) );
                            dy--;
                        }

                        dx--;
                    }
                    return true;
                }

                if( dropPicture ) {
                    TibiaSprite newSprite( -1, x, y );
                    newSprite.setImage( verifiedImage );
                    newSprite.setSource( fileInfo.fileName() );
                    pictures.push_back( g_resourceHandler.createLocalResource( RESOURCE_TYPE_PICTURE, 0xFFFF, newSprite ) );
                    return true;
                }
            } else {
                emit documentError( filePath, tr( "Invalid dimensions, must be divisible by 32." ), -1 );
                return false;
            }
        }
    } else if( dropItem ) {
        if( isIDF ) { // Exported Item
            ExternalFile *externalFile = new ExternalFile();
            if( externalFile->idle( filePath, true ) ) {
                ItemData item = externalFile->getItemData();
                if( !item.isNull() ) {
                    TibiaItem *tibiaItem = new TibiaItem( item );
                    tibiaItem->setSource( fileInfo.fileName() );
                    items.push_back( tibiaItem );
                }

                emit dropTibiaFile( externalFile );
                return true;
            }

            emit documentError( filePath, tr( "Unable to load external file." ), -1 );
            delete externalFile;
            return false;
        } else if( isIDLF ) { // Item Library
            LibraryFile *libraryFile = new LibraryFile();
            if( libraryFile->idle( filePath, true ) ) {
                for( quint32 i = 0; i < libraryFile->getCount(); i++ ) {
                    ItemData item = libraryFile->getItemData( i );
                    if( !item.isNull() ) {
                        TibiaItem *tibiaItem = new TibiaItem( item );
                        tibiaItem->setSource( fileInfo.fileName() );
                        items.push_back( tibiaItem );
                    }
                }

                emit dropTibiaFile( libraryFile );
                return true;
            }

            emit documentError( filePath, tr( "Unable to load library." ), -1 );
            delete libraryFile;
            return false;
        }
    }

    return false;
}
