#include <QtCore/QString>
#include <QtCore/QTime>

#include "exportthread.h"
#include "libraryfile.h"
#include "externalfile.h"

#include "tibiahandler.h"
#include "formathandler.h"

extern TibiaHandler g_tibiaHandler;
extern FormatHandler g_formatHandler;

ExportThread::ExportThread( QObject *parent, ExportMode_t mode ) : UserThread( parent )
{
    setMode( mode );
}

ExportThread::~ExportThread( void )
{
    m_items.clear();
    m_outfits.clear();
    m_effects.clear();
    m_projectiles.clear();
    m_sprites.clear();
    m_pictures.clear();
}

void ExportThread::setup( void )
{
    UserThread::setup();
    m_format = 0;
    m_directory = QString();
    m_exportMode = EXPORT_NONE;
    m_items.clear();
    m_outfits.clear();
    m_effects.clear();
    m_projectiles.clear();
    m_sprites.clear();
    m_pictures.clear();
    m_dParams = DrawParameters();
}

void ExportThread::setFormat( const quint32 _format )
{
    m_format = _format;
}

void ExportThread::setDirectory( const QString& dir )
{
    m_directory = dir;
}

void ExportThread::setMode( const ExportMode_t mode )
{
    m_exportMode = mode;
}

void ExportThread::setItems( const ItemList& list )
{
    m_items = list;
}

void ExportThread::setOutfits( const ItemList& list )
{
    m_outfits = list;
}

void ExportThread::setEffects( const ItemList& list )
{
    m_effects = list;
}

void ExportThread::setProjectiles( const ItemList& list )
{
    m_projectiles = list;
}

void ExportThread::setSprites( const ResourceList& list )
{
    m_sprites = list;
}

void ExportThread::setPictures( const ResourceList& list )
{
    m_pictures = list;
}

void ExportThread::setDrawParameters( const DrawParameters& params )
{
    m_dParams = params;
}

void ExportThread::run()
{
    if( m_exportMode != EXPORT_NONE ) {
        quint32 objectCount = 0;
        quint32 totalCount = m_items.size() + m_outfits.size() + m_effects.size() + m_projectiles.size() + m_sprites.size() + m_pictures.size();
        QTime time;
        time.start();
        setMinimum( 0 );
        setMaximum( totalCount );
        emit valueChanged( 0 );
        if( m_exportMode != EXPORT_LIBRARY ) {
            if( m_items.size() > 0 ) {
                setLabel( tr( "Exporting Items..." ) );
                exportItems( tr( "Items" ), m_items, objectCount, time );
            }
            if( m_outfits.size() > 0 ) {
                setLabel( tr( "Exporting Outfits..." ) );
                exportItems( tr( "Outfits" ), m_outfits, objectCount, time );
            }
            if( m_effects.size() > 0 ) {
                setLabel( tr( "Exporting Effects..." ) );
                exportItems( tr( "Effects" ), m_effects, objectCount, time );
            }
            if( m_projectiles.size() > 0 ) {
                setLabel( tr( "Exporting Projectiles..." ) );
                exportItems( tr( "Projectiles" ), m_projectiles, objectCount, time );
            }
        }
        if( m_exportMode != EXPORT_ITEM && m_exportMode != EXPORT_LIBRARY ) {
            if( m_sprites.size() > 0 ) {
                setLabel( tr( "Exporting Sprites..." ) );
                exportSprites( tr( "Sprites" ), m_sprites, objectCount, time );
            }
            if( m_pictures.size() > 0 ) {
                setLabel( tr( "Exporting Pictures..." ) );
                exportSprites( tr( "Pictures" ), m_pictures, objectCount, time );
            }
        }
        if( !isCanceled() && objectCount > 0 )
            emit success( time.elapsed() );
    }
}

void ExportThread::exportItems( const QString& name, ItemList& items, quint32& currentItem, QTime& time )
{
    if( !items.isEmpty() ) {
        QDir dir( m_directory );
        if( dir.exists() ) {
            if( !dir.cd( name ) )
                dir.mkdir( name );
            dir.cd( name );

            QTime rate;
            rate.start();
            quint32 count = 0;
            foreach( TibiaItem* item, items ) {
                if( isCanceled() )
                    break;
                if( exportItem( dir, item ) ) {
                    emit valueChanged( currentItem );
                    currentItem++;

                    if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
                        emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( m_maximum - currentItem ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                        //emit rateChanged( count, libraryFile->getCount() - i, time.elapsed() ); // Items/sec, Items remaining
                        rate.restart();
                        count = 0;
                    }

                    count++;
                }
            }
        }
    }
}

void ExportThread::exportSprites( const QString& name, ResourceList& sprites, quint32& currentSprite, QTime& time )
{
    if( !sprites.isEmpty() ) {
        QDir dir( m_directory );
        if( dir.exists() ) {
            if( !dir.cd( name ) )
                dir.mkdir( name );
            dir.cd( name );

            QTime rate;
            rate.start();
            quint32 count = 0;
            for( ResourceList::const_iterator it = sprites.constBegin(); it != sprites.constEnd(); it++ ) {
                if( isCanceled() )
                    break;
                if( exportSprite( dir, *it ) ) {
                    emit valueChanged( currentSprite );
                    currentSprite++;

                    if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
                        emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( m_maximum - currentSprite ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                        //emit rateChanged( count, libraryFile->getCount() - i, time.elapsed() ); // Items/sec, Items remaining
                        rate.restart();
                        count = 0;
                    }

                    count++;
                }
            }
        }
    }
}

bool ExportThread::exportSprite( const QDir& dir, const SharedResource& sharedResource )
{
    switch( m_exportMode ) {
    case EXPORT_BMP:
    case EXPORT_PNG: {
        if( sharedResource ) {
            TibiaSprite sprite = sharedResource->getTibiaSprite();
            if( !sprite.image.isNull() )
                return sprite.image.save( dir.path() + QString( tr( "/%1.%2" ) ).arg( sprite.id ).arg( m_exportMode == EXPORT_PNG ? QString( "png" ) : QString( "bmp" ) ) );

            return false;
        }
    }
    break;
    default:
        return false;
        break;
    }
    return false;
}

bool ExportThread::exportItem( const QDir& dir, TibiaItem *tibiaItem )
{
    switch( m_exportMode ) {
    case EXPORT_BMP:
    case EXPORT_PNG: {
        if( ItemFile *itemFile = g_tibiaHandler.getItemFile() ) {
            QString additional;
            switch( tibiaItem->getType() ) {
            case ITEM_TYPE_ITEM: {
                if( m_dParams.blendFrame != -1 && m_dParams.blendFrame != 0 )
                    additional += QString( "_%1" ).arg( m_dParams.blendFrame );
            }
            break;
            case ITEM_TYPE_OUTFIT: {
                quint8 animcount = m_dParams.animation;
                if( animcount > tibiaItem->getAnimation() )
                    animcount = tibiaItem->getAnimation();

                quint8 addon = m_dParams.addons;
                if( addon > tibiaItem->getYDiv()-1 && tibiaItem->getYDiv() != 3 ) // If YDiv is 3 it has addon combination
                    addon = tibiaItem->getYDiv()-1;

                additional += QString( "_%1_%2_%3" ).arg( m_dParams.outfitDirection ).arg( animcount ).arg( addon );
            }
            break;
            case ITEM_TYPE_EFFECT: {
                quint8 animcount = m_dParams.animation;
                if( animcount > tibiaItem->getAnimation() )
                    animcount = tibiaItem->getAnimation();

                additional += QString( "_%1" ).arg( animcount );
            }
            break;
            case ITEM_TYPE_PROJECTILE:
                additional += QString( "_%1" ).arg( m_dParams.projectileDirection );
                break;
            }

            return g_tibiaHandler.drawItem( tibiaItem, m_dParams ).save( dir.path() + QString( "/%1%2.%3" ).arg( itemFile->getItemType( tibiaItem ) ).arg( additional ).arg( m_exportMode == EXPORT_PNG ? QString( "png" ) : QString( "bmp" ) ) );
        }

        return false;
    }
    break;
    case EXPORT_ITEM: {
        if( ItemFile *itemFile = g_tibiaHandler.getItemFile() )
            return ExternalFile( m_format, dir.path() + QString( "/%1.idf" ).arg( itemFile->getItemType( tibiaItem ) ), tibiaItem->getItemData() ).isSuccess();

        return false;
    }
    break;
    default:
        return false;
        break;
    }

    return false;
}
