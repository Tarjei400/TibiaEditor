#include <QtGui/QPixmapCache>
#include <QtCore/QMutex>
#include "resourcefile.h"
#include "tibiaresource.h"
#include "tibiasprite.h"
#include "tibiamodule.h"
#include "tibiahandler.h"
#include "resourcehandler.h"

extern TibiaHandler g_tibiaHandler;
extern ResourceHandler g_resourceHandler;

TibiaResource::TibiaResource( quint8 resourceType, quint32 identifier, quint16 frame, QFile *file ) : m_type( resourceType ), m_id( identifier ), m_frame( frame ), m_file( file )
{
    if( qobject_cast<TibiaFile *>( m_file ) )
        use();
}

TibiaResource::~TibiaResource( void )
{
    reset();
}

void TibiaResource::reset( void )
{
    resetCache();
    m_type = 0;
    m_id = 0;
    m_frame = 0;
    setFile( NULL );
}

void TibiaResource::resetCache( void )
{
    QPixmapCache::remove( QString( "%1-%2-%3-%4" ).arg( m_type ).arg( m_id ).arg( m_frame ).arg( ( quint64 )m_file ) );
}

void TibiaResource::setFile( QFile *file, bool load )
{
    if( m_file != file ) {
        if( m_file && load ) {
            if( qobject_cast<ResourceFile *>( m_file ) )
                unhook();
            if( qobject_cast<TibiaFile *>( m_file ) )
                free();
        }
        m_file = file;
        if( file && load ) {
            if( qobject_cast<TibiaFile *>( m_file ) )
                use();
        }
    }
}

void TibiaResource::unhook( void )
{
    if( ResourceFile *resourceFile = qobject_cast<ResourceFile *>( m_file ) ) {
        delete resourceFile;
        m_file = NULL;
    }
}

void TibiaResource::use( void )
{
    if( TibiaFile *tibiaFile = qobject_cast<TibiaFile *>( m_file ) ) {
        if( tibiaFile->getTibiaModule() )
            tibiaFile->getTibiaModule()->use();
    }
}

void TibiaResource::free( void )
{
    if( TibiaFile *tibiaFile = qobject_cast<TibiaFile *>( m_file ) ) {
        if( tibiaFile->getTibiaModule() )
            tibiaFile->getTibiaModule()->free();
    }
}

TibiaSprite TibiaResource::getTibiaSprite( void )
{
    TibiaSprite sprite = dummy;
    QPixmap pixmap;
    if( QPixmapCache::find( QString( "%1-%2-%3-%4" ).arg( m_type ).arg( m_id ).arg( m_frame ).arg( ( quint64 )m_file ), &pixmap ) ) {
        sprite.id = m_id;
        sprite.width = pixmap.width() / 32;
        sprite.height = pixmap.height() / 32;
        sprite.setImage( pixmap.toImage() );
        return sprite;
    }

    if( !m_file ) {
        if( m_type == RESOURCE_TYPE_SPRITE ) {
            SpriteFile *spriteFile = g_tibiaHandler.getSpriteFile();
            if( spriteFile )
                sprite = spriteFile->loadSprite( m_id );
        } else if( m_type == RESOURCE_TYPE_PICTURE ) {
            PictureFile *pictureFile = g_tibiaHandler.getPictureFile();
            if( pictureFile )
                sprite = pictureFile->loadPicture( m_id );
        }
    } else {
        if( TibiaFile *tibiaFile = qobject_cast<TibiaFile *>( m_file ) ) {
            TibiaModule *tibiaModule = tibiaFile->getTibiaModule();
            if( tibiaModule )
                sprite = tibiaModule->getSprite( m_id, m_frame );
        } else if( ResourceFile *resourceFile = qobject_cast<ResourceFile *>( m_file ) ) {
            sprite = resourceFile->getSprite();
        }
    }

    QPixmapCache::insert( QString( "%1-%2-%3-%4" ).arg( m_type ).arg( m_id ).arg( m_frame ).arg( ( quint64 )m_file ), QPixmap::fromImage( sprite.image ) );
    return sprite;
    /*if( !m_file )
    {
        if( m_type == RESOURCE_TYPE_SPRITE )
        {
            SpriteFile* spriteFile = g_tibiaHandler.getSpriteFile();
            if( spriteFile )
                return spriteFile->loadSprite( m_id );
        }

        if( m_type == RESOURCE_TYPE_PICTURE )
        {
            PictureFile* pictureFile = g_tibiaHandler.getPictureFile();
            if( pictureFile )
                return pictureFile->loadPicture( m_id );
        }
    }

    if( TibiaFile* tibiaFile = qobject_cast<TibiaFile*>( m_file ) )
    {
        TibiaModule* tibiaModule = tibiaFile->getTibiaModule();
        if( tibiaModule )
            return tibiaModule->getSprite( m_id, m_frame );
    }

    if( ResourceFile* resourceFile = qobject_cast<ResourceFile*>( m_file ) ){
        return resourceFile->getSprite();
    }*/

    //return dummy;
}

QDataStream& operator<< ( QDataStream& stream, const SharedResource& sharedResource )
{
    if( sharedResource ) {
        stream << sharedResource->getType();
        stream << sharedResource->getIdentifier();
        stream << sharedResource->getFrame();
        if( sharedResource->getFile() )
            stream << sharedResource->getFile()->fileName();
        else
            stream << QString("");
        return stream;
    }

    stream << ( quint8 )0;
    stream << ( quint32 )0;
    stream << ( quint16 )0;
    stream << QString("");
    return stream;
}

QDataStream& operator>> ( QDataStream& stream, SharedResource& sharedResource )
{
    quint8 type;
    stream >> type;
    quint32 id;
    stream >> id;
    quint16 frame;
    stream >> frame;
    QString fileName;
    stream >> fileName;
    sharedResource = g_resourceHandler.getResourceByData( type, id, frame, fileName );
    return stream;
}
