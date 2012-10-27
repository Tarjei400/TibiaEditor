#include <QtGui/QColor>
#include "tibiasprite.h"

TibiaSprite::TibiaSprite( quint32 _id, quint8 _width, quint8 _height ) : TibiaObject()
{
    id = _id;
    width = _width;
    height = _height;
    r = SPRITE_BG_R; //0xF8
    g = SPRITE_BG_G; //0xF8
    b = SPRITE_BG_B; //0xF0
    image = QImage( width * 32, height * 32, SPRITE_FORMAT );
    image.fill( SPRITE_BACKGROUND );
    dummy = true;
}

TibiaSprite::~TibiaSprite( void )
{

}

void TibiaSprite::setDummy( bool _dummy )
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    dummy = _dummy;
    if( dummy ) {
        image = QImage( width * 32, height * 32, SPRITE_FORMAT );
        image.fill( SPRITE_BACKGROUND );
    }
}

void TibiaSprite::setImage( const QImage& img )
{
    //if( img.width() % 32 == 0 && img.height() % 32 == 0 )
    image = img;
    if( !image.isNull() ) {
        setDummy( false );
        formatImage( image );
    }
}

void TibiaSprite::formatImage( QImage& image )
{
    if( image.format() != SPRITE_FORMAT ) // Convert
        image = image.convertToFormat( SPRITE_FORMAT );

    for ( qint32 x = 0; x < image.width(); x++ ) {
        for ( qint32 y = 0; y < image.height(); y++ ) {
            QColor color = QColor::fromRgba( image.pixel( x, y ) );
            if( color.red() == SPRITE_BG_R && color.green() == SPRITE_BG_G && color.blue() == SPRITE_BG_B )
                image.setPixel( x, y, SPRITE_BACKGROUND );
            else if( color.alpha() == 0x00 ) // Has REAL transparency
                image.setPixel( x, y, SPRITE_BACKGROUND );
            else if( color.alpha() < 0xFF && color.alpha() > 0x00 ) // Partial transparency to solid
                image.setPixel( x, y, qRgba( color.red(), color.green(), color.blue(), 0xFF ) );
        }
    }
}

void TibiaSprite::reset( void )
{
    r = SPRITE_BG_R; //0xF8
    g = SPRITE_BG_G; //0xF8
    b = SPRITE_BG_B; //0xF0
    setDummy( true );
}

QDataStream& operator<< ( QDataStream& stream, const TibiaSprite& tibiaSprite )
{
    stream << tibiaSprite.id;
    stream << tibiaSprite.width;
    stream << tibiaSprite.height;
    stream << tibiaSprite.r;
    stream << tibiaSprite.g;
    stream << tibiaSprite.b;
    stream << tibiaSprite.image;
    return stream;
}

QDataStream& operator>> ( QDataStream& stream, TibiaSprite& tibiaSprite )
{
    stream >> tibiaSprite.id;
    stream >> tibiaSprite.width;
    stream >> tibiaSprite.height;
    stream >> tibiaSprite.r;
    stream >> tibiaSprite.g;
    stream >> tibiaSprite.b;
    stream >> tibiaSprite.image;
    return stream;
}
