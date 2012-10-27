#ifndef TIBIAHANDLER_H
#define TIBIAHANDLER_H

#include <QtGui/QListView>
#include <QtGui/QTreeView>

#include "tibiafile.h"
#include "spritefile.h"
#include "itemfile.h"
#include "picturefile.h"
#include "chasewidget.h"
#include "outputwidget.h"

class ItemModel;
class SpriteModel;
class PictureModel;
class DatFormat;

#define USER_OPERATION_SEARCH 0x01
#define USER_OPERATION_COMPILE 0x02
#define USER_OPERATION_EXPORT 0x03
#define USER_OPERATION_IMPORT 0x04

#define OUTFIT_DIRECTION_DEFAULT 2
#define OUTFIT_DIRECTION_NORTH 0
#define OUTFIT_DIRECTION_EAST 1
#define OUTFIT_DIRECTION_SOUTH 2
#define OUTFIT_DIRECTION_WEST 3

#define PROJECTILE_DIRECTION_DEFAULT 0
#define PROJECTILE_DIRECTION_NORTHWEST  0
#define PROJECTILE_DIRECTION_NORTH      1
#define PROJECTILE_DIRECTION_NORTHEAST  2
#define PROJECTILE_DIRECTION_EAST       3
#define PROJECTILE_DIRECTION_SOUTHEAST  4
#define PROJECTILE_DIRECTION_SOUTH      5
#define PROJECTILE_DIRECTION_SOUTHWEST  6
#define PROJECTILE_DIRECTION_WEST       7

/*static quint32 TemplateOutfitLookupTable[] = {
    0xFFFFFFFF, 0xFFFFD4BF, 0xFFFFE9BF, 0xFFFFFFBF, 0xFFE9FFBF, 0xFFD4FFBF,
    0xFFBFFFBF, 0xFFBFFFD4, 0xFFBFFFE9, 0xFFBFFFFF, 0xFFBFE9FF, 0xFFBFD4FF,
    0xFFBFBFFF, 0xFFD4BFFF, 0xFFE9BFFF, 0xFFFFBFFF, 0xFFFFBFE9, 0xFFFFBFD4,
    0xFFFFBFBF, 0xFFDADADA, 0xFFBF9F8F, 0xFFBFAF8F, 0xFFBFBF8F, 0xFFAFBF8F,
    0xFF9FBF8F, 0xFF8FBF8F, 0xFF8FBF9F, 0xFF8FBFAF, 0xFF8FBFBF, 0xFF8FAFBF,
    0xFF8F9FBF, 0xFF8F8FBF, 0xFF9F8FBF, 0xFFAF8FBF, 0xFFBF8FBF, 0xFFBF8FAF,
    0xFFBF8F9F, 0xFFBF8F8F, 0xFFB6B6B6, 0xFFBF7F5F, 0xFFBFAF8F, 0xFFBFBF5F,
    0xFF9FBF5F, 0xFF7FBF5F, 0xFF5FBF5F, 0xFF5FBF7F, 0xFF5FBF9F, 0xFF5FBFBF,
    0xFF5F9FBF, 0xFF5F7FBF, 0xFF5F5FBF, 0xFF7F5FBF, 0xFF9F5FBF, 0xFFBF5FBF,
    0xFFBF5F9F, 0xFFBF5F7F, 0xFFBF5F5F, 0xFF919191, 0xFFBF6A3F, 0xFFBF943F,
    0xFFBFBF3F, 0xFF94BF3F, 0xFF6ABF3F, 0xFF3FBF3F, 0xFF3FBF6A, 0xFF3FBF94,
    0xFF3FBFBF, 0xFF3F94BF, 0xFF3F6ABF, 0xFF3F3FBF, 0xFF6A3FBF, 0xFF943FBF,
    0xFFBF3FBF, 0xFFBF3F94, 0xFFBF3F6A, 0xFFBF3F3F, 0xFF6D6D6D, 0xFFFF5500,
    0xFFFFAA00, 0xFFFFFF00, 0xFFAAFF00, 0xFF54FF00, 0xFF00FF00, 0xFF00FF54,
    0xFF00FFAA, 0xFF00FFFF, 0xFF00A9FF, 0xFF0055FF, 0xFF0000FF, 0xFF5500FF,
    0xFFA900FF, 0xFFFE00FF, 0xFFFF00AA, 0xFFFF0055, 0xFFFF0000, 0xFF484848,
    0xFFBF3F00, 0xFFBF7F00, 0xFFBFBF00, 0xFF7FBF00, 0xFF3FBF00, 0xFF00BF00,
    0xFF00BF3F, 0xFF00BF7F, 0xFF00BFBF, 0xFF007FBF, 0xFF003FBF, 0xFF0000BF,
    0xFF3F00BF, 0xFF7F00BF, 0xFFBF00BF, 0xFFBF007F, 0xFFBF003F, 0xFFBF0000,
    0xFF242424, 0xFF7F2A00, 0xFF7F5500, 0xFF7F7F00, 0xFF557F00, 0xFF2A7F00,
    0xFF007F00, 0xFF007F2A, 0xFF007F55, 0xFF007F7F, 0xFF00547F, 0xFF002A7F,
    0xFF00007F, 0xFF2A007F, 0xFF54007F, 0xFF7F007F, 0xFF7F0055, 0xFF7F002A,
    0xFF7F0000,
};*/

typedef QPair<quint8, quint8> DCoord;
static DCoord DirectionTable[] = {
    DCoord( 0, 0 ),
    DCoord( 1, 0 ),
    DCoord( 2, 0 ),
    DCoord( 2, 1 ),
    DCoord( 2, 2 ),
    DCoord( 1, 2 ),
    DCoord( 0, 2 ),
    DCoord( 0, 1 )
};

struct DrawParameters {
    DrawParameters() {
        outfitDirection = OUTFIT_DIRECTION_DEFAULT;
        projectileDirection = PROJECTILE_DIRECTION_DEFAULT;
        animation = 0;
        addons = 0;
        floor = 0;
        blendOverlay = 0;
        blendFrame = -1;
        cropImage = false;
        showPattern = false;
    }
    quint8 outfitDirection;
    quint8 projectileDirection;
    quint8 animation;
    quint8 addons;
    quint8 floor;
    qint16 blendOverlay;
    qint16 blendFrame;
    bool cropImage;
    bool showPattern;

    bool operator==( const DrawParameters& other ) const {
        return ( outfitDirection == other.outfitDirection &&
                 projectileDirection == other.projectileDirection &&
                 animation == other.animation &&
                 addons == other.addons &&
                 blendOverlay == other.blendOverlay &&
                 blendFrame == other.blendFrame &&
                 cropImage == other.cropImage &&
                 showPattern == other.showPattern );

    }
    inline bool operator!=( const DrawParameters& other ) const {
        return !operator==( other );
    }
};

class TibiaHandler : public QObject
{
    Q_OBJECT

public:
    TibiaHandler( void );
    virtual ~TibiaHandler( void );

    bool loadFormatFile( const QString& );
    bool loadSpriteFile( const QString&, bool, bool newFile = false );
    bool loadItemFile( const QString&, DatFormat *, bool, bool newFile = false );
    bool loadPictureFile( const QString&, bool, bool newFile = false );

    void unloadSpriteFile( void );
    void unloadItemFile( void );
    void unloadPictureFile( void );

    SpriteFile *getSpriteFile( void ) const {
        return m_spriteFile;
    };
    ItemFile *getItemFile( void ) const {
        return m_itemFile;
    };
    PictureFile *getPictureFile( void ) const {
        return m_pictureFile;
    };

    // Drawing functions
    //QImage drawItemByLooktype( quint32 lookType, quint8 head, quint8 body, quint8 legs, quint8 feet, const DrawParameters& dParam = DrawParameters() );
    //QImage drawItemTemplate( const TibiaItem *tibiaItem, quint8 head, quint8 body, quint8 legs, quint8 feet, const DrawParameters& dParam = DrawParameters() );
    QImage drawItemById( quint32 itemId, const DrawParameters& dParam = DrawParameters() );
    QImage drawItem( const TibiaItem *tibiaItem, const DrawParameters& dParam = DrawParameters() );
    QImage drawItem( const ItemData& itemData, const DrawParameters& dParam = DrawParameters() );
    //void translateDirection( quint8 direction, const TibiaItem *tibiaItem, quint8 &divx, quint8 &divy );
    void mergeImage( QImage& itemImage, const ItemData& itemData, quint8 divx, quint8 divy, quint8 divz, quint8 animation, qint16 blendFrame, quint32 offset_x = 0, quint32 offset_y = 0 );
    void renderImage( qint16 blendframe, QImage& itemImage, const ItemData& itemData, quint8 divx, quint8 divy, quint8 divz, quint8 animation, quint32 override_offset_x, quint32 override_offset_y );
    quint32 getSpriteFrame( const ItemData& itemData, quint8 divx, quint8 divy, quint8 divz, quint8 frame, quint32 x, quint32 y, quint8 animation ) const;
    SharedResource getSpriteResource( quint32 frame, const ItemData& itemData );
    TibiaSprite getSprite( quint32 frame, const TibiaItem *tibiaItem );
    TibiaSprite getSprite( quint32 frame, const ItemData& itemData );
    qint32 getSpriteFrameByPosition( const ItemData& itemData, quint32 x, quint32 y, quint32 z, quint8 outfitDirection = 0, quint8 projectileDirection = 0, quint8 addons = 0, quint8 blendFrame = 0xFF, quint8 animation = 0 );

    static ChaseWidget *getChaseWidget() {
        static ChaseWidget *instance;
        if( !instance )
            instance = new ChaseWidget;
        return instance;
    }

    static OutputWidget *getOutputWidget() {
        static OutputWidget *instance;
        if( !instance )
            instance = new OutputWidget;
        return instance;
    }

private:
    SpriteFile *m_spriteFile;
    ItemFile *m_itemFile;
    PictureFile *m_pictureFile;

    mutable QMutex mutex;

signals:
    void outputString( const QString& );
    void parseError( QString, QFile::FileError );
    void documentError( QString, QString, int );
    void itemsLoaded( ItemFile * );
    void spritesLoaded( SpriteFile * );
    void picturesLoaded( PictureFile * );
    void itemsUnloaded( void );
    void spritesUnloaded( void );
    void picturesUnloaded( void );
    void invalidateItems( void );

public slots:
    void onAddSprites( quint8, quint32 );
    void onRemoveSprites( quint8, quint32 );

// Bounce signals
private slots:
    void onDocumentError( QString, QString, int );
    void onFileError( QString, QFile::FileError );
};

#endif // TIBIAHANDLER_H
