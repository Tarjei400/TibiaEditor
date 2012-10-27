#ifndef TIBIASPRITE_H
#define TIBIASPRITE_H

#include <QtGui/QImage>
#include <QtCore/QHash>

#include "tibiaobject.h"

#define SPRITE_FORMAT       QImage::Format_ARGB32
#define SPRITE_BACKGROUND   0x00FF00FF
#define SPRITE_BG_R 0xFF
#define SPRITE_BG_G 0x00
#define SPRITE_BG_B 0xFF

class TibiaSprite : public TibiaObject
{
public:
    TibiaSprite( quint32 _id = 0, quint8 _width = 1, quint8 _height = 1 );
    virtual ~TibiaSprite( void );

    bool isDummy() const {
        return dummy;
    };
    void setDummy( bool dummy );
    virtual void reset( void );

    void setImage( const QImage& );
    static void formatImage( QImage& image );

    quint32 id;
    quint8 width, height, r, g, b;
    QImage image;
    bool dummy;
};

QDataStream& operator<< ( QDataStream& stream, const TibiaSprite& tibiaSprite );
QDataStream& operator>> ( QDataStream& stream, TibiaSprite& tibiaSprite );

static TibiaSprite dummy;

//typedef QCache<quint16, TibiaSprite> SpriteCache;
typedef QHash<quint32, TibiaSprite> SpriteHash;

#endif
