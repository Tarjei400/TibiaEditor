#include <QtCore/QTemporaryFile>

#include "libraryfile.h"
#include "tibiahandler.h"
#include "externalfile.h"
#include "resourcehandler.h"
#include "formathandler.h"
#include "userthread.h"

extern TibiaHandler g_tibiaHandler;
extern ResourceHandler g_resourceHandler;
extern FormatHandler g_formatHandler;

LibraryFile::LibraryFile( QObject *parent ) : TibiaModule( parent )
{
    m_thread = new LibraryThread( this );
    m_datFormat = NULL;
    m_count = 0;
    m_loaded = false;
    m_mustIdle = true;
}

LibraryFile::LibraryFile( DatFormat *format, const ItemList& items, QObject *parent ) : TibiaModule( parent )
{
    m_thread = new LibraryThread( this );
    m_idle = false;
    m_mustIdle = false;
    m_count = items.size();
    m_datFormat = format;
    m_version = m_datFormat->getVersion();
    m_items = items;
    QObject::connect( m_thread, SIGNAL( finished() ), this, SLOT( deleteLater() ) );
}

LibraryFile::~LibraryFile( void )
{
    if( m_thread ) {
        m_thread->wait();
        delete m_thread;
    }
}

void LibraryFile::unload( void )
{
    m_count = 0;
    m_version = 0;
    m_loaded = false;
    m_mustIdle = true;
    m_items.clear();
}

bool LibraryFile::idle( const QString& name, bool read )
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

    if( read ) {
        TibiaFile::seek( 0 );
        quint16 internalVersion = 0;
        QDataStream in( this );
        in.setByteOrder( QDataStream::LittleEndian );
        in >> internalVersion;
        in >> m_count;
        in >> m_version;

        m_datFormat = g_formatHandler.getFormatByClient( m_version );
        if( !m_datFormat ) {
            emit documentError( name, tr( "Unknown format: %1" ).arg( m_version ), -1 );
            return false;
        }
    }

    m_idle = true;
    m_loaded = true;
    return true;
}

ItemData LibraryFile::getItemData( qint32 index )
{
    QMutexLocker locker( &mutex );

    if( isIdle() ) {
        QDataStream in( this );
        in.setByteOrder( QDataStream::LittleEndian );
        TibiaFile::seek( sizeof( quint16 ) + sizeof( m_count ) + sizeof( m_version ) + index * sizeof( quint32 ) );
        quint32 offset;
        in >> offset;
        if( offset == 0x00000000 ) // Blank item
            return ItemData();

        TibiaFile::seek( offset );

        QString error;
        quint32 address = 0;
        ItemData itemData;
        if( ExternalFile::readItem( in, itemData, m_datFormat, this, index, address, error ) ) {
            m_spriteHash.insert( index, address );
            return itemData;
        } else
            emit documentError( fileName(), error, -1 );
    }

    if( m_internalItems.contains( index ) )
        return m_internalItems.value( index );

    if( m_items.value( index ) )
        return m_items.value( index )->getItemData();

    return ItemData();
}

TibiaSprite LibraryFile::getSprite( qint32 index, qint32 frame )
{
    QMutexLocker locker( &mutex );

    TibiaSprite sprite;
    quint32 address = m_spriteHash.value( index );
    if( address == 0 ) {
        getItemData( index );
    }
    if( address != 0 ) {
        QDataStream in( this );
        in.setByteOrder( QDataStream::LittleEndian );
        TibiaFile::seek( address + frame * sizeof( quint32 ) );
        quint32 offset;
        in >> offset;

        if( offset == 0x00000000 ) // Return a blank image
            return sprite;

        TibiaFile::seek( offset );
        sprite.setDummy( false );
        TibiaFile::readSprite( in, sprite, 0, 0 );
    }

    return sprite;
}

/*bool LibraryFile::save( const QString & name )
{
    if( isIdle() ) // File is on idle, we need to stream our file to our new file
    {
        // Create temporary file, write signature and count
        QTemporaryFile file( name );
        if( !file.open() ){
            emit parseError( QObject::tr( "Open Error" ), file.error() );
            return false;
        }

        QDataStream dest( &file );
        dest.setByteOrder( QDataStream::LittleEndian );
        dest << ( quint16 )0xACED;
        dest << ( quint32 )m_items.size();
        dest << m_version;

        bool success = false;

        m_datFormat = g_formatHandler.getFormatByClient( m_version );
        if( !m_datFormat ){
            emit documentError( name, tr( "Unknown format: %1" ).arg( m_version ), -1 );
            return false;
        }

        if( QFile::exists( name ) )
        {
            if( name.compare( TibiaFile::fileName(), Qt::CaseInsensitive ) == 0 ) // Destination == Source, Close source
                TibiaFile::close();

            if( !QFile::remove( name ) )
            {
                emit documentError( name, tr( "Failed to overwrite existing file." ), -1 );
                return false;
            }
        }

        quint32 now = file.pos();
        quint32 offset = now + m_items.size() * sizeof( quint32 );
        for( quint32 i = 0; i < m_count; i++ )
        {
            ItemData itemData = getItemData( i );
            file.seek( now );
            if( itemData.isNull() )
            {
                dest << ( quint32 )0x00000000;
                now += sizeof( quint32 );
                continue;
            }

            dest << offset;
            file.seek( offset );
            success = ExternalFile::saveItem( dest, itemData, m_datFormat );
            if( !success )
                break;

            offset = file.pos();
            now += sizeof( quint32 );
        }

        if( success )
        {
            file.copy( name );
            idle( name, true ); // Upon saving we must re-idle our new file to continue
        }

        return success;
    }

    // Not streaming, we can write item directly

    setFileName( name );

    if( !TibiaFile::open( QIODevice::WriteOnly ) ){
        emit parseError( QObject::tr( "Open Error" ), TibiaFile::error() );
        return false;
    }

    TibiaFile::seek( 0 );

    QDataStream out( this );
    out.setByteOrder( QDataStream::LittleEndian );
    out << ( quint16 )0xACED;
    out << ( quint32 )m_items.size();
    out << m_version;

    bool success = false;

    m_datFormat = g_formatHandler.getFormatByClient( m_version );
    if( !m_datFormat ){
        emit documentError( name, tr( "Unknown format: %1" ).arg( m_version ), -1 );
        return false;
    }

    quint32 now = TibiaFile::pos();
    quint32 offset = now + m_items.size() * sizeof( quint32 );
    for( quint32 i = 0; i < m_count; i++ )
    {
        ItemData itemData = getItemData( i );
        TibiaFile::seek( now );
        if( itemData.isNull() )
        {
            out << ( quint32 )0x00000000;
            now += sizeof( quint32 );
            continue;
        }

        out << offset;
        TibiaFile::seek( offset );

        success = ExternalFile::saveItem( out, itemData, m_datFormat );
        if( !success )
            break;

        offset = TibiaFile::pos();
        now += sizeof( quint32 );
    }

    if( success )
    {
        if( m_mustIdle )
            success = idle( name, true ); // Upon saving we must re-idle our new file to continue
    }

    TibiaFile::close();
    return success;
    //m_thread->setup();
    //m_thread->setName( name );
    //m_thread->execute();
    return true;
}*/

void LibraryFile::setItemData( qint32 index, const ItemData& item )
{
    QMutexLocker locker( &mutex );
    m_internalItems.insert( index, item );
}
