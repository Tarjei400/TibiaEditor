#include <QtCore/QMap>
#include <QtCore/QDir>
#include <QtCore/QTemporaryFile>
#include "spritefile.h"
#include "resourcehandler.h"
#include "userthread.h"

extern ResourceHandler g_resourceHandler;

SpriteFile::SpriteFile( QObject *parent ) : TibiaFile( parent )
{
    m_count = 0;
    m_loaded = false;
    m_thread = new SpriteThread( this );
}

SpriteFile::~SpriteFile( void )
{
    delete m_thread;
}

void SpriteFile::unload( void )
{
    TibiaFile::unload();
    m_count = 0;
    m_loaded = false;
}

bool SpriteFile::createNew( void )
{
    if( !m_loaded ) {
        m_count = 2;
        TibiaSprite sprite;
        sprite.id = 1;
        sprite.setDummy( false );
        SharedResource newResource = g_resourceHandler.createLocalResource( RESOURCE_TYPE_SPRITE, 1, sprite );
        ResourceList resources;
        resources.push_back( newResource );
        g_resourceHandler.addLocalResources( RESOURCE_TYPE_SPRITE, 1, resources );
        m_loaded = true;
        return true;
    }

    return false;
}

bool SpriteFile::idle( const QString& name, bool read )
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

    TibiaFile::seek( 0 );

    if( read ) {
        QDataStream in( this );
        in.setByteOrder( QDataStream::LittleEndian );
        in >> m_signature;
        in >> m_count;
    }

    m_idle = true;
    m_loaded = true;
    return m_idle;
}

/*bool SpriteFile::save( const QString& name )
{
    if( isIdle() ) // File is on idle, we need to stream our file to our new file
    {
        // Create temporary file, write signature and count
        QTemporaryFile file( name );
        if( !file.open() ){
            emit parseError( QObject::tr( "Open Error" ), file.error() );
            return false;
        }

        quint16 currentSprite = 1;
        quint32 offset = 0, now = 0;

        QDataStream dest( &file );
        dest.setByteOrder( QDataStream::LittleEndian );
        dest << m_signature;
        dest << m_count;

        now = file.pos();
        offset =  m_count * sizeof( offset ) + sizeof( m_signature ) + sizeof( m_count );

        // Loop 1 - Count, check "spriteChanges" before reading original
        while ( currentSprite < m_count )
        {
            file.seek( now );

            TibiaSprite sprite = getSprite( currentSprite );
            if( sprite.isDummy() ){
                dest << ( quint32 )0x00000000;
                now += sizeof( quint32 );
                currentSprite++;
                continue;
            }

            dest << offset;
            file.seek( offset );
            TibiaFile::writeSprite( dest, sprite, 0, 0, offset );
            now += sizeof( quint32 );
            currentSprite++;
        }

        if( QFile::exists( name ) )
        {
            if( name.compare( TibiaFile::fileName(), Qt::CaseInsensitive ) == 0 ) // Destination == Source, Close source
                TibiaFile::close();

            if( !QFile::remove( name ) ) // Overwrite Error
                return false;
        }

        file.copy( name );

        idle( name, true ); // Upon saving we must re-idle our new file to continue
        return true;
    }

    return false;
    //m_thread->setup();
    //m_thread->setName( name );
    //m_thread->execute();
    return true;
}*/

TibiaSprite SpriteFile::loadSprite( quint32 spriteId )
{
    QMutexLocker locker( &mutex );
    TibiaSprite sprite;
    sprite.id = spriteId;
    if( isIdle() ) { // We are idling our file so we can read constantly
        if( spriteId < 1 || spriteId > m_count )
            return sprite;

        QDataStream in( this );
        in.setByteOrder( QDataStream::LittleEndian );
        seek( sizeof( m_signature ) + sizeof( m_count ) + ( ( spriteId - 1 ) * sizeof( quint32 ) ) );
        quint32 offset;
        in >> offset;

        if ( offset == 0 ) // Don't cache dummy images
            return sprite;

        TibiaFile::seek( offset );
        sprite.id = spriteId;
        sprite.setDummy( false );
        TibiaFile::readSprite( in, sprite, 0, 0 );
        return sprite;
    }

    return sprite;
}

TibiaSprite SpriteFile::getSprite( quint32 id )
{
    SharedResource resource = g_resourceHandler.loadLocalResource( RESOURCE_TYPE_SPRITE, id, false );
    if( resource )
        return g_resourceHandler.getSpriteByResource( resource );

    if( isIdle() )
        return loadSprite( id );

    return dummy;
}
