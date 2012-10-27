#include <QtCore/QCoreApplication>
#include <QtCore/QMimeData>
#include <QtGui/QPainter>
#include "tibiahandler.h"
#include "resourcehandler.h"
#include "formathandler.h"

extern ResourceHandler g_resourceHandler;
extern FormatHandler g_formatHandler;

TibiaHandler::TibiaHandler( void ) : QObject( NULL )
{
    m_itemFile = NULL;
    m_spriteFile = NULL;
    m_pictureFile = NULL;
}

TibiaHandler::~TibiaHandler()
{
    unloadSpriteFile();
    unloadItemFile();
    unloadPictureFile();
}

bool TibiaHandler::loadItemFile( const QString& fileName, DatFormat *datFormat, bool setModel, bool newFile )
{
    m_itemFile = new ItemFile( this );
    if( m_itemFile ) {
        QObject::connect( m_itemFile, SIGNAL( parseError( QString, QFile::FileError ) ), this, SLOT( onFileError( QString, QFile::FileError ) ) );
        QObject::connect( m_itemFile, SIGNAL( documentError( QString, QString, int ) ), this, SLOT( onDocumentError( QString, QString, int ) ) );

        getOutputWidget()->addLine( QColor( Qt::red ), tr( "Loading begi.. tibiaHandler.cpp" ) );
        if( datFormat || newFile ) { // We dont care about the format if we're creating a file
            m_itemFile->setFormat( datFormat );
            if( ( !newFile && m_itemFile->load( fileName ) ) || ( newFile && m_itemFile->createNew() ) ) {
                if( setModel ) {
                    emit itemsLoaded( m_itemFile );
                }
                return true;
            } else {
                unloadItemFile();
                return false;
            }
        } else {
            emit documentError( fileName, tr( "Invalid format." ), -1 );
            unloadItemFile();
            return false;
        }
    }

    unloadItemFile();
    return false;
}

void TibiaHandler::unloadItemFile( void )
{
    if( m_itemFile ) { // File cleanup
        emit itemsUnloaded();
        delete m_itemFile;
        m_itemFile = NULL;
    }
}

bool TibiaHandler::loadSpriteFile( const QString& fileName, bool setModel, bool newFile )
{
    m_spriteFile = new SpriteFile( this );
    if( m_spriteFile ) {
        QObject::connect( m_spriteFile, SIGNAL( parseError( QString, QFile::FileError ) ), this, SLOT( onFileError( QString, QFile::FileError ) ) );
        QObject::connect( m_spriteFile, SIGNAL( documentError( QString, QString, int ) ), this, SLOT( onDocumentError( QString, QString, int ) ) );
        if( ( !newFile && m_spriteFile->idle( fileName, true ) ) || ( newFile && m_spriteFile->createNew() ) ) {
            if( setModel ) {
                emit spritesLoaded( m_spriteFile );
            }

            emit invalidateItems();
            return m_spriteFile->isLoaded();
        } else {
            unloadSpriteFile();
            return false;
        }
    }

    unloadSpriteFile();
    return false;
}

void TibiaHandler::unloadSpriteFile( void )
{
    if( m_spriteFile ) { // File cleanup
        emit spritesUnloaded();
        delete m_spriteFile;
        m_spriteFile = NULL;
        emit invalidateItems();
    }
}

void TibiaHandler::onAddSprites( quint8 resourceType, quint32 amount )
{
    if( resourceType == RESOURCE_TYPE_SPRITE ) {
        if( m_spriteFile )
            m_spriteFile->setCount( m_spriteFile->getCount() + amount );
    }

    if( resourceType == RESOURCE_TYPE_PICTURE ) {
        if( m_pictureFile )
            m_pictureFile->setCount( m_pictureFile->getCount() + amount );
    }
}

void TibiaHandler::onRemoveSprites( quint8 resourceType, quint32 amount )
{
    if( resourceType == RESOURCE_TYPE_SPRITE ) {
        if( m_spriteFile )
            m_spriteFile->setCount( m_spriteFile->getCount() - amount );
    }

    if( resourceType == RESOURCE_TYPE_PICTURE ) {
        if( m_pictureFile )
            m_pictureFile->setCount( m_pictureFile->getCount() - amount );
    }
}

bool TibiaHandler::loadPictureFile( const QString& fileName, bool setModel, bool newFile )
{
    m_pictureFile = new PictureFile( this );
    if( m_pictureFile ) {
        QObject::connect( m_pictureFile, SIGNAL( parseError( QString, QFile::FileError ) ), this, SLOT( onFileError( QString, QFile::FileError ) ) );
        QObject::connect( m_pictureFile, SIGNAL( documentError( QString, QString, int ) ), this, SLOT( onDocumentError( QString, QString, int ) ) );

        if( ( !newFile && m_pictureFile->idle( fileName, true ) ) || ( newFile && m_pictureFile->createNew() ) ) {
            if( setModel ) {
                emit picturesLoaded( m_pictureFile );
            }

            return m_pictureFile->isLoaded();
        } else {
            unloadPictureFile();
            return false;
        }
    }

    unloadPictureFile();
    return false;
}

void TibiaHandler::unloadPictureFile( void )
{
    if( m_pictureFile ) { // File cleanup
        emit picturesUnloaded();
        delete m_pictureFile;
        m_pictureFile = NULL;
    }
}

void TibiaHandler::onFileError( QString type, QFile::FileError error )
{
    emit parseError( type, error );
}

void TibiaHandler::onDocumentError( QString fileName, QString error, int line )
{
    emit documentError( fileName, error, line );
}

QImage TibiaHandler::drawItemById( quint32 itemId, const DrawParameters& dParam )
{
    if( m_itemFile ) {
        if( TibiaItem *item = m_itemFile->getItemById( itemId ) )
            return drawItem( item, dParam );
    }

    return QImage();
}
/*// Working but unused
QImage TibiaHandler::drawItemByLooktype( quint32 lookType, quint8 head, quint8 body, quint8 legs, quint8 feet, const DrawParameters& dParam )
{
    if( m_itemFile )
    {
        if( TibiaItem* item = m_itemFile->getItemByIndex( ITEM_PARENT_OUTFITS, lookType-1 ) )
            return drawItemTemplate( item, head, body, legs, feet, dParam );
    }

    return QImage();
}

QImage TibiaHandler::drawItemTemplate( const TibiaItem *tibiaItem, quint8 head, quint8 body, quint8 legs, quint8 feet, const DrawParameters& dParam )
{
    if( !tibiaItem )
        return QImage(); // Return blank image

    DrawParameters proxyParam = dParam;
    proxyParam.blendOverlay = 0;

    QImage sourceImage = drawItem( tibiaItem, proxyParam );

    proxyParam.blendOverlay = 1;
    QImage templateImage = drawItem( tibiaItem, proxyParam );

    for ( qint32 cx = 0; cx < templateImage.width(); cx++ )
    {
        for ( qint32 cy = 0; cy < templateImage.height(); cy++ )
        {
            QRgb rgb = templateImage.pixel( cx, cy );
            if( rgb == qRgba( 0xFF, 0xFF, 0x00, 0xFF ) )
                templateImage.setPixel( cx, cy, TemplateOutfitLookupTable[head] );
            else if( rgb == qRgba( 0xFF, 0x00, 0x00, 0xFF ) )
                templateImage.setPixel( cx, cy, TemplateOutfitLookupTable[body] );
            else if( rgb == qRgba( 0x00, 0xFF, 0x00, 0xFF ) )
                templateImage.setPixel( cx, cy, TemplateOutfitLookupTable[legs] );
            else if( rgb == qRgba( 0x00, 0x00, 0xFF, 0xFF ) )
                templateImage.setPixel( cx, cy, TemplateOutfitLookupTable[feet] );
        }
    }

    QImage itemImage( 32 * tibiaItem->getWidth(), 32 * tibiaItem->getHeight(), SPRITE_FORMAT );
    itemImage.fill( SPRITE_BACKGROUND );

    QPainter painter( &itemImage );
    painter.drawImage( 0, 0, sourceImage );
    //painter.setCompositionMode( QPainter::CompositionMode_Darken );
    painter.setCompositionMode( QPainter::CompositionMode_Multiply );
    painter.drawImage( 0, 0, templateImage );
    painter.end();

    TibiaSprite::formatImage( itemImage );

    return itemImage;
}
*/

QImage TibiaHandler::drawItem( const TibiaItem *tibiaItem, const DrawParameters& dParam )
{
    return drawItem( tibiaItem->getItemData(), dParam );
}

QImage TibiaHandler::drawItem( const ItemData& itemData, const DrawParameters& dParam )
{
    QMutexLocker locker( &mutex );

    if( itemData.isNull() )
        return dummy.image; // Return dummy image

    QImage itemImage( 32 * itemData.width, 32 * itemData.height, SPRITE_FORMAT );
    itemImage.fill( SPRITE_BACKGROUND );

    bool isPattern = ( itemData.type == ITEM_TYPE_ITEM && ( itemData.xdiv > 1 || itemData.ydiv > 1 ) );
    bool isOutfit = ( itemData.type == ITEM_TYPE_OUTFIT );
    bool hasAddons = ( isOutfit && itemData.ydiv > 1 );
    bool blendingOutfit = ( isOutfit && itemData.blendframes > 1 );

    quint8 divx;
    quint8 divy;
    quint8 divz = dParam.floor;
    if( isOutfit ) {
        divx = ( dParam.outfitDirection % 4 ) % itemData.xdiv;
        divy = 0;
    } else if( itemData.type == ITEM_TYPE_PROJECTILE ) {
        divx = DirectionTable[dParam.projectileDirection].first;
        divy = DirectionTable[dParam.projectileDirection].second;
    } else {
        divx = 0 % itemData.xdiv;
        divy = 0 % itemData.ydiv;
    }

    // Limiters
    quint8 animation = dParam.animation;
    if( dParam.animation > itemData.animcount-1 )
        animation = itemData.animcount-1;

    qint16 blendFrames = dParam.blendFrame;
    if( dParam.blendFrame > itemData.blendframes-1 )
        blendFrames = itemData.blendframes-1;

    qint16 blendOverlay = dParam.blendOverlay;
    if( dParam.blendOverlay > itemData.blendframes-1 )
        blendOverlay = itemData.blendframes-1;

    if( hasAddons && dParam.addons > 0) {
        if( dParam.addons == 1 ) {
            mergeImage( itemImage, itemData, divx, (quint8)0, divz, animation, blendOverlay );
            mergeImage( itemImage, itemData, divx, (quint8)1, divz, animation, blendOverlay );
        } else if( dParam.addons == 2 ) {
            mergeImage( itemImage, itemData, divx, (quint8)0, divz, animation, blendOverlay );
            mergeImage( itemImage, itemData, divx, (quint8)2, divz, animation, blendOverlay );
        } else if( dParam.addons == 3 ) {
            mergeImage( itemImage, itemData, divx, (quint8)0, divz, animation, blendOverlay );
            mergeImage( itemImage, itemData, divx, (quint8)1, divz, animation, blendOverlay );
            mergeImage( itemImage, itemData, divx, (quint8)2, divz, animation, blendOverlay );
        }
    } else if( blendingOutfit )
        mergeImage( itemImage, itemData, divx, divy, divz, animation, blendOverlay );
    else if( isPattern && dParam.showPattern ) {
        QImage newImage( 32 * itemData.width * itemData.xdiv, 32 * itemData.height * itemData.ydiv, SPRITE_FORMAT );
        newImage.fill( SPRITE_BACKGROUND );
        for ( quint32 x = 0; x < itemData.xdiv; x++ ) {
            for ( quint32 y = 0; y < itemData.ydiv; y++ ) {
                mergeImage( newImage, itemData, x, y, divz, animation, blendFrames, x * itemData.width * 32, y * itemData.height * 32 );
            }
        }

        itemImage = newImage;
    } else
        mergeImage( itemImage, itemData, divx, divy, divz, animation, blendFrames );

    if( dParam.cropImage && isOutfit && ( itemData.width > 1 || itemData.height > 1 ) )
        return itemImage.copy ( QRect( ( 32 * itemData.width ) - itemData.cropsize, ( 32 * itemData.height ) - itemData.cropsize, itemData.cropsize, itemData.cropsize ) );

    return itemImage;
}

void TibiaHandler::mergeImage( QImage& itemImage, const ItemData& itemData, quint8 divx, quint8 divy, quint8 divz, quint8 animation, qint16 blendFrame, quint32 offset_x, quint32 offset_y )
{
    if( blendFrame != -1 && itemData.blendframes > 1 )
        renderImage( blendFrame, itemImage, itemData, divx, divy, divz, animation, offset_x, offset_y );
    else {
        for ( quint8 frame = 0; frame < itemData.blendframes; frame++ )
            renderImage( frame, itemImage, itemData, divx, divy, divz, animation, offset_x, offset_y );
    }
}

void TibiaHandler::renderImage( qint16 blendframe, QImage& itemImage, const ItemData& itemData, quint8 divx, quint8 divy, quint8 divz, quint8 animation, quint32 override_offset_x, quint32 override_offset_y )
{
    qint16 real_x = 0, real_y = 0, dest_x = 0, dest_y = 0; // Re-use variables
    qint16 offset_y, offset_x, frame;
    for ( offset_y = itemData.height - 1; offset_y >= 0; offset_y-- ) { // Do it backwards
        real_x = 0;
        for ( offset_x = itemData.width - 1; offset_x >= 0; offset_x-- ) {
            dest_x = 0;
            dest_y = 0;
            if( itemData.width > 1 )
                dest_x = offset_x * 32;
            if( itemData.height > 1 )
                dest_y = offset_y * 32;
            frame = TibiaItem::getSpriteFrame( itemData, divx, divy, divz, blendframe, real_x, real_y, animation );
            TibiaSprite sprite = getSprite( frame, itemData );
            if( !sprite.isDummy() ) {
                for ( quint8 cx = 0; cx < 32; cx++ ) {
                    for ( quint8 cy = 0; cy < 32; cy++ ) {
                        QRgb pixel = sprite.image.pixel( cx, cy );
                        if( pixel && qAlpha ( pixel ) != 0x00 )
                            itemImage.setPixel ( cx + dest_x + override_offset_x, cy + dest_y + override_offset_y, pixel );
                    }
                }
            }

            real_x++;
        }
        real_y++;
    }
}

SharedResource TibiaHandler::getSpriteResource( quint32 frame, const ItemData& itemData )
{
    SharedResource reference = itemData.getSpriteResource( frame );
    if( reference )
        return reference;
    else {
        reference = g_resourceHandler.loadLocalResource( RESOURCE_TYPE_SPRITE, itemData.getLocalSprite( frame ), false );
        if( reference )
            return reference;
    }

    return SharedResource();
}

TibiaSprite TibiaHandler::getSprite( quint32 frame, const ItemData& itemData )
{
    SharedResource reference = itemData.getSpriteResource( frame );
    if( reference )
        return reference->getTibiaSprite();
    else {
        reference = g_resourceHandler.loadLocalResource( RESOURCE_TYPE_SPRITE, itemData.getLocalSprite( frame ), false );
        if( reference )
            return reference->getTibiaSprite();
    }

    return dummy;
}

TibiaSprite TibiaHandler::getSprite( quint32 frame, const TibiaItem *tibiaItem )
{
    SharedResource reference = tibiaItem->getSpriteResource( frame );
    if( reference )
        return reference->getTibiaSprite();
    else {
        reference = g_resourceHandler.loadLocalResource( RESOURCE_TYPE_SPRITE, tibiaItem->getLocalSprite( frame ), false );
        if( reference )
            return reference->getTibiaSprite();
    }

    return dummy;
}

qint32 TibiaHandler::getSpriteFrameByPosition( const ItemData& itemData, quint32 x, quint32 y, quint32 z, quint8 outfitDirection, quint8 projectileDirection, quint8 addons, quint8 blendFrame, quint8 animation )
{
    if( itemData.isNull() )
        return -1;

    bool isPattern = ( itemData.type == ITEM_TYPE_ITEM && ( itemData.xdiv > 1 || itemData.ydiv > 1 ) );
    bool isOutfit = ( itemData.type == ITEM_TYPE_OUTFIT );
    bool isProjectile = ( itemData.type == ITEM_TYPE_PROJECTILE );

    quint8 divx = (0) % itemData.xdiv;
    quint8 divy = (0) % itemData.ydiv;
    quint8 divz = (0) % itemData.zdiv;
    if( isPattern ) {
        qint32 new_x = -1;
        qint32 new_y = -1;

        quint8 real_x = 0;
        quint8 real_y = 0;
        for ( quint8 dy = itemData.ydiv - 1; dy >= 0; dy-- ) {
            real_x = 0;
            for ( quint8 dx = itemData.xdiv - 1; dx >= 0; dx-- ) {
                if( real_x * itemData.width == x && real_y * itemData.height == y ) {
                    new_x = dx * itemData.width;
                    new_y = dy * itemData.height;
                }
                real_x++;
            }
            real_y++;
        }

        if( new_x != -1 )
            x = new_x;
        if( new_y != -1 )
            y = new_y;

        divx = ( x / itemData.width ) % itemData.xdiv;
        divy = ( y / itemData.height ) % itemData.ydiv;
        divz = z % itemData.zdiv;

        x = x % itemData.width;
        y = y % itemData.height;
    }
    if( isOutfit ) {
        divx = ( outfitDirection % 4 ) % itemData.xdiv;
        divy = 0;

        if( addons > 0 && addons < 3 ) // 3 is a combination of 1 and 2, its not actually a layer
            divy = addons;
        divz = z % itemData.zdiv;
    }
    if( isProjectile ) {
        divx = DirectionTable[projectileDirection].first;
        divy = DirectionTable[projectileDirection].second;
        divz = z % itemData.zdiv;
    }

    quint8 frame = blendFrame;
    if( blendFrame == 0xFF )
        frame = itemData.blendframes - 1; // Select the topmost stack

    return TibiaItem::getSpriteFrame( itemData, divx, divy, divz, frame, x, y, animation );
}
