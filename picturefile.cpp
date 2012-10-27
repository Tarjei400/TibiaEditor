#include <QtCore/QTemporaryFile>

#include "picturefile.h"
#include "resourcehandler.h"
#include "userthread.h"

extern ResourceHandler g_resourceHandler;

PictureFile::PictureFile( QObject *parent ) : TibiaFile( parent )
{
    m_count = 0;
    m_loaded = false;
    m_thread = new PictureThread( this );
}

PictureFile::~PictureFile( void )
{
    delete m_thread;
}

void PictureFile::unload( void )
{
    TibiaFile::unload();
    m_count = 0;
    m_loaded = false;
}

bool PictureFile::createNew( void )
{
    if( !m_loaded ) {
        m_count = 1;
        TibiaSprite picture;
        picture.id = 1;
        picture.setDummy( false );
        SharedResource newResource = g_resourceHandler.createLocalResource( RESOURCE_TYPE_PICTURE, 0, picture );
        ResourceList resources;
        resources.push_back( newResource );
        g_resourceHandler.addLocalResources( RESOURCE_TYPE_PICTURE, 0, resources );
        m_loaded = true;
        return true;
    }

    return false;
}

bool PictureFile::idle( const QString& name, bool read )
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

/*bool PictureFile::save( const QString& name )
{
    if( isIdle() ) // File is on idle, we need to stream our file to our new file
    {
        // Create temporary file, write signature and count
        QTemporaryFile file( name );
        if( !file.open() ){
            emit parseError( QObject::tr( "Open Error" ), file.error() );
            return false;
        }

        quint32 offset = 0, now = 0;

        QDataStream dest( &file );
        dest.setByteOrder( QDataStream::LittleEndian );
        dest << m_signature;
        dest << m_count;

        now = file.pos();
        offset = sizeof( m_signature ) + sizeof( m_count ) + m_count * 5;
        for( quint16 i = 0; i < m_count; i++ )
        {
            TibiaSprite picture = getPicture( i );
            if( !picture.isDummy() ){
                offset += picture.width * picture.height * sizeof( offset );
            }
        }

        for( quint16 i = 0; i < m_count; i++ )
        {
            TibiaSprite picture = getPicture( i );
            if( !picture.isDummy() ){
                dest << picture.width;
                dest << picture.height;

                dest << picture.r; // Forget what these are for
                dest << picture.g;
                dest << picture.b;

                quint32 now = TibiaFile::pos();

                for( quint8 offset_y = 0; offset_y < picture.height; offset_y++ )
                {
                    for( quint8 offset_x = 0; offset_x < picture.width; offset_x++ )
                    {
                        TibiaFile::seek( now );
                        dest << offset;
                        TibiaFile::writeSprite ( dest, picture, offset_x * 32, offset_y * 32, offset, false );
                        now += sizeof( quint32 );
                    }
                }
            }
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
    //return true;
}*/

TibiaSprite PictureFile::loadPicture( quint16 pictureId )
{
    QMutexLocker locker( &mutex );
    if( isIdle() ) { // We are idling our file so we can read constantly
        if( pictureId > m_count )
            return dummy;

        QDataStream in( this );
        in.setByteOrder( QDataStream::LittleEndian );
        TibiaFile::seek( 0 );
        TibiaFile::seek( sizeof( m_signature ) + sizeof( m_count ) );

        quint32 offset = 0;
        for( quint16 i = 0; i < m_count; i++ ) {

            quint8 width;
            quint8 height;

            in >> width;
            in >> height;

            if( i == pictureId ) {
                TibiaSprite picture( pictureId, width, height );
                in >> picture.r;
                in >> picture.g;
                in >> picture.b;
                picture.setDummy( false );

                for( quint8 offset_y = 0; offset_y < picture.height; offset_y++) {
                    for( quint8 offset_x = 0; offset_x < picture.width; offset_x++) {
                        in >> offset;
                        quint32 lastOffset = TibiaFile::pos();

                        if( offset == 0 ) {
                            lastOffset += sizeof( quint32 );
                            TibiaFile::seek( lastOffset );
                            continue;
                        }

                        TibiaFile::seek( offset );
                        TibiaFile::readSprite ( in, picture, offset_x * 32, offset_y * 32, false );
                        TibiaFile::seek( lastOffset );
                    }
                }

                return picture;
            } else // width, height, r, g, b, offsets -> data
                TibiaFile::seek( TibiaFile::pos() + 3 + ( width * height * sizeof( offset ) ) );
        }
    }

    return dummy;
}

TibiaSprite PictureFile::getPicture( quint16 id )
{
    SharedResource resource = g_resourceHandler.loadLocalResource( RESOURCE_TYPE_PICTURE, id, false );
    if( resource )
        return g_resourceHandler.getSpriteByResource( resource );

    if( isIdle() )
        return loadPicture( id );

    return dummy;
}

