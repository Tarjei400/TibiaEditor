#include "tibiafile.h"
#include "tibiaitem.h"
#include "tibiasprite.h"
#include "resourcehandler.h"

extern ResourceHandler g_resourceHandler;

TibiaFile::TibiaFile( QObject *parent ) : QFile( parent )
{
    m_signature = 0;
    m_idle = false;
}

void TibiaFile::unload( void )
{
    m_signature = 0;
    m_idle = false;
}

quint32 TibiaFile::peekSignature( const QString& fileName )
{
    quint32 signature = 0;
    QFile file( fileName );
    if( file.open( QIODevice::ReadOnly ) ) {
        QDataStream in( &file );
        in.setByteOrder( QDataStream::LittleEndian );
        in >> signature;
    }
    file.close();
    return signature;
}

void TibiaFile::readSprite ( QDataStream& in, TibiaSprite& sprite, quint32 offset_x, quint32 offset_y, bool read_rgb )
{
    bool transparent = true;
    quint16 dumpSize = 0, chunkSize = 0, aux = 0;

    quint32 x = 0, y = 0, transparency;
    transparency = SPRITE_BACKGROUND;
    if( read_rgb ) {
        in >> sprite.r;
        in >> sprite.g;
        in >> sprite.b;
    }

    in >> dumpSize;
    if( dumpSize == 0 || dumpSize > 1024 * 3 * 2 ) {
        for( x = 0; x < 32; x++ ) {
            for( y = 0; y < 32; y++ ) {
                sprite.image.setPixel ( x + offset_x, y + offset_y, transparency );
            }
        }

        return;
    }

    while( dumpSize > 0 ) {
        in >> chunkSize;
        if( chunkSize > 1024 * 3 ) // If every pixel has rgb it cannot exceed this
            break;

        dumpSize -= 2;

        if ( transparent ) {
            aux = chunkSize;

            while( aux > 0 ) {
                sprite.image.setPixel ( x + offset_x, y + offset_y, transparency );

                x++;

                if ( x == 32 ) {
                    x = 0;

                    y++;
                }

                aux--;
            }
        } else {
            quint8 r = 0, g = 0, b = 0;

            aux = chunkSize;

            while( aux > 0 ) {
                in >> r;
                in >> g;
                in >> b;

                sprite.image.setPixel ( x + offset_x, y + offset_y, qRgb( r, g, b ) );

                x++;

                if ( x == 32 ) {
                    x = 0;

                    y++;
                }

                aux--;
            }

            dumpSize -= ( chunkSize * 3 );
        }

        transparent = !transparent;
    }

    while( y < 32 ) {
        sprite.image.setPixel ( x + offset_x, y + offset_y, transparency );

        x++;

        if ( x == 32 ) {
            x = 0;

            y++;
        }
    }
}

/*void TibiaFile::readSprite ( QDataStream& in, TibiaSprite& sprite, quint32 offset_x, quint32 offset_y, bool read_rgb )
{
    bool transparent = true;
    quint16 dumpSize, chunkSize, aux;

    quint32 x = 0, y = 0, transparency;
    transparency = SPRITE_BACKGROUND;
    if( read_rgb ){
        in >> sprite.r;
        in >> sprite.g;
        in >> sprite.b;
    }

    in >> dumpSize;
    if( dumpSize == 0 || dumpSize > 1024 * 3 * 2 ){
        sprite.image.fill( transparency );
        return;
    }

    while( dumpSize > 0 )
    {
        in >> chunkSize;
        if( chunkSize > 1024 * 3 ) // If every pixel has rgb it cannot exceed this
            break;

        dumpSize -= 2;

        if ( transparent )
        {
            aux = chunkSize;

            while( aux > 0 )
            {
                sprite.image.setPixel ( x + offset_x, y + offset_y, transparency );

                x++;

                if ( x == 32 ){
                    x = 0;
                    y++;
                }
                if ( y == 32 )
                    y = 0;

                aux--;
            }
        }
        else
        {
            quint8 r, g, b;

            aux = chunkSize;

            while( aux > 0 )
            {
                in >> r;
                in >> g;
                in >> b;

                sprite.image.setPixel( x + offset_x, y + offset_y, qRgb( r, g, b ) );

                x++;

                if ( x == 32 ){
                    x = 0;
                    y++;
                }
                if ( y == 32 )
                    y = 0;

                aux--;
            }

            dumpSize -= ( chunkSize * 3 );
        }

        transparent = !transparent;
    }
}*/

void TibiaFile::writeSprite( QDataStream& out, const TibiaSprite& sprite, quint32 offset_x, quint32 offset_y, quint32& dumpOffset, bool write_rgb )
{
    bool transparent = true;
    quint32 index;
    quint16 dumpSize, chunkSize, alphaCount;
    quint32 chunkSizeOffset, previousOffset, finishOffset;
    quint8 rgbSize = 0;

    QIODevice *device = out.device();
    if( !device )
        return;

    previousOffset = device->pos();
    device->seek( dumpOffset );
    if( write_rgb ) {
        out << sprite.r;
        out << sprite.g;
        out << sprite.b;
        rgbSize = 3;
    }

    device->seek( device->pos() + sizeof( dumpSize ) ); // Skip dumpsize for now

    alphaCount = 0;
    index = 0;
    while( index < 1024 ) {
        chunkSize = 0;
        while( index < 1024 ) {
            QRgb curPixel = sprite.image.pixel ( offset_x + ( index % 32 ), offset_y + ( index / 32 ) );
            transparent = ( qAlpha( curPixel ) == 0x00 || ( curPixel == SPRITE_BACKGROUND ) ); // Allow QImage support
            if( !transparent )
                break;

            alphaCount++;
            chunkSize++;
            index++;
        }
        if( alphaCount < 1024 ) { // Entire image is transparent
            if( index < 1024 ) { // Already at the end
                out << chunkSize;

                chunkSizeOffset = device->pos(); // Store this address to be referenced
                device->seek( device->pos() + sizeof( chunkSize ) ); // Skip chunksize indicator

                chunkSize = 0;
                while( index < 1024 ) {
                    QRgb curPixel = sprite.image.pixel ( offset_x + ( index % 32 ), offset_y + ( index / 32 ) );
                    transparent = ( qAlpha( curPixel ) == 0x00 || ( curPixel == SPRITE_BACKGROUND ) ); // Allow QImage support
                    if( transparent )
                        break;

                    //out << (quint8)0xFF; // Simulate Alpha Pixel
                    out << (quint8)qRed( curPixel );
                    out << (quint8)qGreen( curPixel );
                    out << (quint8)qBlue( curPixel );

                    chunkSize++;
                    index++;
                }

                finishOffset = device->pos();
                device->seek( chunkSizeOffset ); // Go back to chunksize indicator
                out << chunkSize; // Write chunkSize after calculation
                device->seek( finishOffset );
            } else
                finishOffset = device->pos();
        }
    }

    device->seek( dumpOffset + rgbSize ); // Return to the original dumpSize
    if( alphaCount < 1024 )
        index = finishOffset - dumpOffset - rgbSize - sizeof( dumpSize );
    else {
        index = 0;
        finishOffset = dumpOffset + rgbSize + sizeof( dumpSize );
    }

    out << (quint16)index; // Write the new dumpSize
    device->seek( previousOffset );

    dumpOffset = finishOffset;
}
