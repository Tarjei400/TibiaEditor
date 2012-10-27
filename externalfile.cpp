#include <QtCore/QTemporaryFile>

#include "tibiahandler.h"
#include "externalfile.h"
#include "resourcehandler.h"
#include "formathandler.h"

extern TibiaHandler g_tibiaHandler;
extern ResourceHandler g_resourceHandler;
extern FormatHandler g_formatHandler;

ExternalFile::ExternalFile( QObject *parent ) : TibiaModule( parent )
{
    m_address = 0;
    m_loaded = false;
    m_mustIdle = true;
    m_success = false;
}

ExternalFile::ExternalFile( quint16 version, const QString& name, const ItemData& item, QObject *parent ) : TibiaModule( parent )
{
    m_version = version;
    m_idle = false;
    m_mustIdle = false;
    m_success = false;
    m_internalItemData = item;
    save( name );
    deleteLater();
}

ExternalFile::~ExternalFile( void )
{

}

void ExternalFile::unload( void )
{
    m_version = 0;
    m_address = 0;
    m_loaded = false;
    m_mustIdle = true;
    m_internalItemData.reset();
}

bool ExternalFile::idle( const QString& name, bool read )
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
        QDataStream in( this );
        in.setByteOrder( QDataStream::LittleEndian );
        in >> m_version;

        m_datFormat = g_formatHandler.getFormatByClient( m_version );
        if( !m_datFormat ) {
            emit documentError( name, tr( "Unknown format: %1" ).arg( m_version ), -1 );
            return false;
        }
    }

    m_idle = true;
    m_success = true;
    return m_success;
}

bool ExternalFile::readItem( QDataStream& in, ItemData& itemData, DatFormat *datFormat, TibiaModule *tibiaModule, qint32 index, quint32& address, QString& error )
{
    QIODevice *device = in.device();
    if( !device )
        return false;

    quint8 type = 0;
    in >> type;

    if( type != ITEM_TYPE_ITEM && type != ITEM_TYPE_OUTFIT && type != ITEM_TYPE_EFFECT && type != ITEM_TYPE_PROJECTILE ) {
        error = QObject::tr( "Unknown Item type" );
        return false;
    }

    ItemData d_itemData;
    if( datFormat ) {
        if( !ItemFile::loadItem( datFormat, in, d_itemData, error, true ) )
            return false;
    }

    d_itemData.parent = ITEM_PARENT_EXTERNAL;
    d_itemData.type = type;

    address = 0;

    if( !device->atEnd() ) {
        quint32 spriteCount = 0, now = 0, offset = 0;
        in >> spriteCount;
        address = device->pos();
        now = device->pos();
        for( quint32 i = 0; i < spriteCount; i++ ) {
            device->seek( now );
            in >> offset;
            if ( offset == 0x00000000 || offset > device->size() ) { // Direct to an image that doesnt exist or out of boundaries
                now += sizeof( quint32 );
                continue;
            }

            QMutex mutex;
            mutex.lock();
            SharedResource resource = g_resourceHandler.addResource(RESOURCE_TYPE_SPRITE, index, i, tibiaModule);
            d_itemData.setSpriteResource(i, resource);
            mutex.unlock();
            now += sizeof( quint32 );
        }
    }

    itemData = d_itemData;
    return true;
}

bool ExternalFile::saveItem( QDataStream& out, const ItemData& itemData, DatFormat *datFormat )
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    QIODevice *device = out.device();
    if( !device )
        return false;

    out << itemData.type;

    ItemFile::saveItem( datFormat, out, itemData );

    if( itemData.getSpriteCount() == 0 ) {
        out << ( quint32 )0;
        return true;
    }

    quint32 spriteCount = itemData.getSpriteCount();
    out << spriteCount;

    quint32 now = device->pos();
    quint32 offset = now + spriteCount * sizeof( quint32 );
    for( quint32 i = 0; i < spriteCount; i++ ) {
        device->seek( now );
        TibiaSprite sprite = g_tibiaHandler.getSprite( i, itemData );
        if( sprite.isDummy() ) {
            out << ( quint32 )0x00000000;
            now += sizeof( quint32 );
            continue;
        }

        out << offset;
        device->seek( offset );
        TibiaFile::writeSprite( out, sprite, 0, 0, offset );
        now += sizeof( quint32 );
    }

    device->seek( offset );
    return true;
}

ItemData ExternalFile::getItemData( void )
{
    QMutexLocker locker( &mutex );

    if( isIdle() ) {
        QDataStream in( this );
        in.setByteOrder( QDataStream::LittleEndian );
        TibiaFile::seek( sizeof( m_version ) );

        ItemData itemData;
        QString error;
        m_loaded = readItem( in, itemData, m_datFormat, this, 0, m_address, error );
        if( m_loaded )
            return itemData;
        else
            emit documentError( fileName(), tr( "Document Error: %1" ).arg( error ), -1 );
    }

    return m_internalItemData;
}

TibiaSprite ExternalFile::getSprite( qint32, qint32 frame )
{
    QMutexLocker locker( &mutex );

    TibiaSprite sprite;
    if( m_address != 0 ) {
        QDataStream in( this );
        in.setByteOrder( QDataStream::LittleEndian );
        TibiaFile::seek( m_address + frame * sizeof( quint32 ) );
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

bool ExternalFile::save( const QString& name )
{
    if( isIdle() ) { // File is on idle, we need to stream our file to our new file
        // Create temporary file, write signature and count
        QTemporaryFile file( name );
        if( !file.open() ) {
            emit parseError( QObject::tr( "Open Error" ), file.error() );
            return false;
        }

        QDataStream dest( &file );
        dest.setByteOrder( QDataStream::LittleEndian );
        dest << m_version;

        m_success = false;

        m_datFormat = g_formatHandler.getFormatByClient( m_version );
        if( !m_datFormat ) {
            emit documentError( name, tr( "Unknown format: %1" ).arg( m_version ), -1 );
            return false;
        }

        if( QFile::exists( name ) ) {
            if( name.compare( TibiaFile::fileName(), Qt::CaseInsensitive ) == 0 ) // Destination == Source, Close source
                TibiaFile::close();

            if( !QFile::remove( name ) ) {
                emit documentError( name, tr( "Failed to overwrite existing file." ), -1 );
                return false;
            }
        }

        if( m_success = saveItem( dest, getItemData(), m_datFormat ) ) {
            file.copy( name );
            idle( name, true ); // Upon saving we must re-idle our new file to continue
        }

        return m_success;
    }

    // Not streaming, we can write item directly

    setFileName( name );

    if( !TibiaFile::open( QIODevice::WriteOnly ) ) {
        emit parseError( QObject::tr( "Open Error" ), TibiaFile::error() );
        return false;
    }

    TibiaFile::seek( 0 );

    QDataStream out( this );
    out.setByteOrder( QDataStream::LittleEndian );
    out << m_version;

    m_success = false;

    m_datFormat = g_formatHandler.getFormatByClient( m_version );
    if( !m_datFormat ) {
        emit documentError( name, tr( "Unknown format: %1" ).arg( m_version ), -1 );
        return false;
    }

    if( m_success = saveItem( out, getItemData(), m_datFormat ) ) {
        if( m_mustIdle )
            m_success = idle( name, true );
    }

    TibiaFile::close();
    return m_success;
}

void ExternalFile::setItemData( const ItemData& item )
{
    QMutexLocker locker( &mutex );
    m_internalItemData.copy( item );
}
