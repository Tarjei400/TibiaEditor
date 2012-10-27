#include <QtCore/QTemporaryFile>
#include <QtCore/QDataStream>
#include <QtCore/QTime>
#include <QtCore/QFileInfo>
#include <QtGui/QProgressDialog>
#include <QtGui/QIcon>

#include "formathandler.h"

#include "userthread.h"
#include "spritefile.h"
#include "picturefile.h"
#include "externalfile.h"
#include "libraryfile.h"

extern FormatHandler g_formatHandler;

void UserThread::cancel( void )
{
    mutex.lock();
    m_abort = true;
    mutex.unlock();
}

void UserThread::setup( void )
{
    m_abort = false;
    m_name = QString();
    m_labelText = QString();
    m_windowTitle = QString();
    m_minimum = 0;
    m_maximum = 0;
}

void UserThread::execute( QProgressDialog *progressDialog )
{
    QObject::connect( progressDialog, SIGNAL( canceled( ) ), this, SLOT( cancel( ) ), Qt::QueuedConnection );
    QObject::connect( this, SIGNAL( windowTitle( const QString& ) ), progressDialog, SLOT( setWindowTitle( const QString& ) ), Qt::BlockingQueuedConnection );
    QObject::connect( this, SIGNAL( labelText( const QString& ) ), progressDialog, SLOT( setLabelText( const QString& ) ), Qt::BlockingQueuedConnection );
    QObject::connect( this, SIGNAL( minimum( int ) ), progressDialog, SLOT( setMinimum( int ) ), Qt::BlockingQueuedConnection );
    QObject::connect( this, SIGNAL( maximum( int ) ), progressDialog, SLOT( setMaximum( int ) ), Qt::BlockingQueuedConnection );
    QObject::connect( this, SIGNAL( valueChanged( int ) ), progressDialog, SLOT( setValue( int ) ), Qt::BlockingQueuedConnection );
    QObject::connect( this, SIGNAL( finished( ) ), progressDialog, SLOT( close( ) ), Qt::QueuedConnection );
    QThread::start();
    if( progressDialog )
        progressDialog->exec();
}

bool UserThread::isCanceled( void ) const
{
    QMutexLocker locker( &mutex );
    return m_abort;
}

void UserThread::setName( const QString& name )
{
    QMutexLocker locker( &mutex );
    m_name = name;
}

void UserThread::setLabel( const QString& label )
{
    QMutexLocker locker( &mutex );
    m_labelText = label;
    emit labelText( m_labelText );
}

const QString& UserThread::getLabel( void ) const
{
    QMutexLocker locker( &mutex );
    return m_labelText;
}

void UserThread::setWindowTitle( const QString& title )
{
    QMutexLocker locker( &mutex );
    m_windowTitle = title;
    emit windowTitle( m_windowTitle );
}

const QString& UserThread::getWindowTitle( void ) const
{
    QMutexLocker locker( &mutex );
    return m_windowTitle;
}

void UserThread::setMinimum( int min )
{
    QMutexLocker locker( &mutex );
    m_minimum = min;
    emit minimum( m_minimum );
}

void UserThread::setMaximum( int max )
{
    QMutexLocker locker( &mutex );
    m_maximum = max;
    emit maximum( m_maximum );
}

int UserThread::getMinimum( void ) const
{
    QMutexLocker locker( &mutex );
    return m_minimum;
}

int UserThread::getMaximum( void ) const
{
    QMutexLocker locker( &mutex );
    return m_maximum;
}

void ItemThread::run( void )
{
    if( m_name.isEmpty() )
        return;

    QTime time;
    ItemFile *itemFile = qobject_cast<ItemFile *>( parent() );
    if( itemFile ) {
        setWindowTitle( tr( "Saving Item File..." ) );
        setLabel( QFileInfo( m_name ).fileName() );

        time.start();
        // Create temporary file, write signature and count
        QTemporaryFile file( m_name );
        if( !file.open() ) {
            emit parseError( tr( "Open Error" ), file.error() );
            return;
        }

        quint16 currentItem = itemFile->getFirst();

        setMinimum( currentItem );
        setMaximum( itemFile->getLast() );
        emit valueChanged( 0 );

        QDataStream dest( &file );
        dest.setByteOrder( QDataStream::LittleEndian );
        dest << itemFile->getSignature();
        dest << itemFile->getItemCount();
        dest << itemFile->getOutfitCount();
        dest << itemFile->getEffectCount();
        dest << itemFile->getProjectileCount();

        QTime rate;
        rate.start();
        quint32 count = 0;
        setWindowTitle( tr( "Saving Items..." ) );
        foreach( TibiaItem* tibiaItem, itemFile->getItems() ) {
            if( isCanceled() )
                break;
            if( !ItemFile::saveItem( itemFile->getFormat(), dest, tibiaItem->getItemData() ) )
                return;

            emit valueChanged( currentItem );

            if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
                emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( itemFile->getItemCount() + itemFile->getOutfitCount() + itemFile->getEffectCount() + itemFile->getProjectileCount() - currentItem ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                //emit rateChanged( count, itemFile->getItemCount() + itemFile->getOutfitCount() + itemFile->getEffectCount() + itemFile->getProjectileCount() - currentItem, time.elapsed() ); // Items/sec, Items remaining
                rate.restart();
                count = 0;
            }

            currentItem++;
            count++;
        }
        setWindowTitle( tr( "Saving Outfits..." ) );
        foreach( TibiaItem* tibiaItem, itemFile->getOutfits() ) {
            if( isCanceled() )
                break;
            if( !ItemFile::saveItem( itemFile->getFormat(), dest, tibiaItem->getItemData() ) )
                return;

            emit valueChanged( currentItem );

            if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
                emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( itemFile->getItemCount() + itemFile->getOutfitCount() + itemFile->getEffectCount() + itemFile->getProjectileCount() - currentItem ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                //emit rateChanged( count, itemFile->getItemCount() + itemFile->getOutfitCount() + itemFile->getEffectCount() + itemFile->getProjectileCount() - currentItem, time.elapsed() ); // Items/sec, Items remaining
                rate.restart();
                count = 0;
            }

            currentItem++;
            count++;
        }
        setWindowTitle( tr( "Saving Effects..." ) );
        foreach( TibiaItem* tibiaItem, itemFile->getEffects() ) {
            if( isCanceled() )
                break;
            if( !ItemFile::saveItem( itemFile->getFormat(), dest, tibiaItem->getItemData() ) )
                return;

            emit valueChanged( currentItem );

            if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
                emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( itemFile->getItemCount() + itemFile->getOutfitCount() + itemFile->getEffectCount() + itemFile->getProjectileCount() - currentItem ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                //emit rateChanged( count, itemFile->getItemCount() + itemFile->getOutfitCount() + itemFile->getEffectCount() + itemFile->getProjectileCount() - currentItem, time.elapsed() ); // Items/sec, Items remaining
                rate.restart();
                count = 0;
            }

            currentItem++;
            count++;
        }
        setWindowTitle( tr( "Saving Projectiles..." ) );
        foreach( TibiaItem* tibiaItem, itemFile->getProjectiles() ) {
            if( isCanceled() )
                break;
            if( !ItemFile::saveItem( itemFile->getFormat(), dest, tibiaItem->getItemData() ) )
                return;

            emit valueChanged( currentItem );

            if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
                emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( itemFile->getItemCount() + itemFile->getOutfitCount() + itemFile->getEffectCount() + itemFile->getProjectileCount() - currentItem ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                //emit rateChanged( count, itemFile->getItemCount() + itemFile->getOutfitCount() + itemFile->getEffectCount() + itemFile->getProjectileCount() - currentItem, time.elapsed() ); // Items/sec, Items remaining
                rate.restart();
                count = 0;
            }

            currentItem++;
            count++;
        }


        if( !isCanceled() ) {
            if( QFile::exists( m_name ) ) {
                if( !QFile::remove( m_name ) ) // Overwrite Error
                    return;
            }

            file.copy( m_name );
            emit success( time.elapsed() );
        }
    }
}

void SpriteThread::run( void )
{
    if( m_name.isEmpty() )
        return;

    QTime time;
    SpriteFile *spriteFile = qobject_cast<SpriteFile *>( parent() );
    if( spriteFile ) {
        setWindowTitle( tr( "Saving Sprite File..." ) );
        setLabel( QFileInfo( m_name ).fileName() );

        time.start();
        // Create temporary file, write signature and count
        QTemporaryFile file( m_name );
        if( !file.open() ) {
            emit parseError( tr( "Open Error" ), file.error() );
            return;
        }

        quint32 currentSprite = 1;
        quint32 offset = 0, now = 0;

        setMinimum( currentSprite );
        setMaximum( spriteFile->getCount() );
        emit valueChanged( 0 );

        QDataStream dest( &file );
        dest.setByteOrder( QDataStream::LittleEndian );
        dest << spriteFile->getSignature();
        dest << spriteFile->getCount();

        now = file.pos();
        offset =  spriteFile->getCount() * sizeof( offset ) + sizeof( spriteFile->getSignature() ) + sizeof( spriteFile->getCount() );

        QTime rate;
        rate.start();
        quint32 count = 0;
        // Loop 1 - Count, check "spriteChanges" before reading original
        while ( currentSprite < spriteFile->getCount() ) {
            if( isCanceled() )
                break;

            file.seek( now );

            TibiaSprite sprite = spriteFile->getSprite( currentSprite );
            if( sprite.isDummy() ) {
                dest << ( quint32 )0x00000000;
                now += sizeof( quint32 );
                emit valueChanged( currentSprite );
                currentSprite++;
                continue;
            }

            dest << offset;
            file.seek( offset );
            TibiaFile::writeSprite( dest, sprite, 0, 0, offset );
            now += sizeof( quint32 );
            emit valueChanged( currentSprite );

            if( rate.elapsed() >= 1000 ) {
                emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( spriteFile->getCount() - currentSprite ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                rate.restart();
                count = 0;
            }

            currentSprite++;
            count++;
        }

        if( !isCanceled() ) {
            if( QFile::exists( m_name ) ) {
                if( m_name.compare( spriteFile->fileName(), Qt::CaseInsensitive ) == 0 ) // Destination == Source, Close source
                    spriteFile->close();

                if( !QFile::remove( m_name ) ) // Overwrite Error
                    return;
            }

            file.copy( m_name );
            spriteFile->idle( m_name, true ); // Upon saving we must re-idle our new file to continue
            emit success( time.elapsed() );
        }
    }
}

void PictureThread::run( void )
{
    if( m_name.isEmpty() )
        return;

    QTime time;
    PictureFile *pictureFile = qobject_cast<PictureFile *>( parent() );
    if( pictureFile ) {
        setWindowTitle( tr( "Saving Picture File..." ) );
        setLabel( QFileInfo( m_name ).fileName() );

        time.start();
        // Create temporary file, write signature and count
        QTemporaryFile file( m_name );
        if( !file.open() ) {
            emit parseError( tr( "Open Error" ), file.error() );
            return;
        }

        quint32 offset = 0, now = 0, total = 0;

        QDataStream dest( &file );
        dest.setByteOrder( QDataStream::LittleEndian );
        dest << pictureFile->getSignature();
        dest << pictureFile->getCount();

        now = file.pos();
        offset = sizeof( pictureFile->getSignature() ) + sizeof( pictureFile->getCount() ) + pictureFile->getCount() * 5;
        for( quint16 i = 0; i < pictureFile->getCount(); i++ ) {
            TibiaSprite picture = pictureFile->getPicture( i );
            if( !picture.isDummy() ) {
                total += picture.width * picture.height;
                offset += picture.width * picture.height * sizeof( offset );
            }
        }

        setMinimum( 1 );
        setMaximum( total );
        emit valueChanged( 0 );

        QTime rate;
        rate.start();
        quint32 count = 0;
        quint32 current = 0;
        for( quint16 i = 0; i < pictureFile->getCount(); i++ ) {
            if( isCanceled() )
                break;

            TibiaSprite picture = pictureFile->getPicture( i );
            if( !picture.isDummy() ) {
                dest << picture.width;
                dest << picture.height;

                dest << picture.r; // Forget what these are for
                dest << picture.g;
                dest << picture.b;

                quint32 now = file.pos();

                for( quint8 offset_y = 0; offset_y < picture.height; offset_y++ ) {
                    if( isCanceled() )
                        break;

                    for( quint8 offset_x = 0; offset_x < picture.width; offset_x++ ) {
                        if( isCanceled() )
                            break;

                        file.seek( now );
                        dest << offset;
                        TibiaFile::writeSprite ( dest, picture, offset_x * 32, offset_y * 32, offset, false );
                        now += sizeof( quint32 );
                        emit valueChanged( current );

                        if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
                            emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( total - current ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                            //emit rateChanged( count, total - current, time.elapsed() ); // Items/sec, Items remaining
                            rate.restart();
                            count = 0;
                        }

                        current++;
                        count++;
                    }
                }
            }
        }

        if( !isCanceled() ) {
            if( QFile::exists( m_name ) ) {
                if( m_name.compare( pictureFile->fileName(), Qt::CaseInsensitive ) == 0 ) // Destination == Source, Close source
                    pictureFile->close();

                if( !QFile::remove( m_name ) ) // Overwrite Error
                    return;
            }

            file.copy( m_name );
            pictureFile->idle( m_name, true ); // Upon saving we must re-idle our new file to continue
            emit success( time.elapsed() );
        }
    }
}

void LibraryThread::run( void )
{
    if( m_name.isEmpty() )
        return;

    QTime time;
    LibraryFile *libraryFile = qobject_cast<LibraryFile *>( parent() );
    if( libraryFile ) {
        if( libraryFile->isIdle() ) { // File is on idle, we need to stream our file to our new file
            setWindowTitle( tr( "Saving Library..." ) );
            setLabel( QFileInfo( m_name ).fileName() );

            time.start();
            // Create temporary file, write signature and count
            QTemporaryFile file( m_name );
            if( !file.open() ) {
                emit parseError( tr( "Open Error" ), file.error() );
                return;
            }

            QDataStream dest( &file );
            dest.setByteOrder( QDataStream::LittleEndian );
            dest << ( quint16 )0xACED;
            dest << ( quint32 )libraryFile->getItemsSize();
            dest << libraryFile->getVersion();

            bool isSuccess = false;

            DatFormat *datFormat = g_formatHandler.getFormatByClient( libraryFile->getVersion() );
            if( !datFormat ) {
                emit documentError( m_name, tr( "Unknown format: %1" ).arg( libraryFile->getVersion() ), -1 );
                return;
            }

            if( QFile::exists( m_name ) ) {
                if( m_name.compare( libraryFile->fileName(), Qt::CaseInsensitive ) == 0 ) // Destination == Source, Close source
                    libraryFile->close();

                if( !QFile::remove( m_name ) ) {
                    emit documentError( m_name, tr( "Failed to overwrite existing file." ), -1 );
                    return;
                }
            }

            setMinimum( 0 );
            setMaximum( libraryFile->getCount()-1 );
            emit valueChanged( 0 );

            QTime rate;
            rate.start();
            quint32 count = 0;
            quint32 now = file.pos();
            quint32 offset = now + libraryFile->getItemsSize() * sizeof( quint32 );
            for( quint32 i = 0; i < libraryFile->getCount(); i++ ) {
                if( isCanceled() ) {
                    isSuccess = false;
                    break;
                }

                ItemData itemData = libraryFile->getItemData( i );
                file.seek( now );
                if( itemData.isNull() ) {
                    dest << ( quint32 )0x00000000;
                    now += sizeof( quint32 );
                    continue;
                }

                dest << offset;
                file.seek( offset );
                isSuccess = ExternalFile::saveItem( dest, itemData, datFormat );
                if( !isSuccess )
                    break;

                offset = file.pos();
                now += sizeof( quint32 );
                emit valueChanged( i );

                if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
                    emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( libraryFile->getCount() - i ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                    //emit rateChanged( count, libraryFile->getCount() - i, time.elapsed() ); // Items/sec, Items remaining
                    rate.restart();
                    count = 0;
                }

                count++;
            }

            if( !isCanceled() ) {
                if( isSuccess ) {
                    file.copy( m_name );
                    libraryFile->idle( m_name, true ); // Upon saving we must re-idle our new file to continue
                }

                emit success( time.elapsed() );
            }

            return;
        }

        // Not streaming, we can write item directly
        setWindowTitle( tr( "Saving Library..." ) );
        setLabel( QFileInfo( m_name ).fileName() );

        time.start();
        libraryFile->setFileName( m_name );

        if( !libraryFile->open( QIODevice::WriteOnly ) ) {
            emit parseError( tr( "Open Error" ), libraryFile->error() );
            return;
        }

        libraryFile->seek( 0 );

        QDataStream out( libraryFile );
        out.setByteOrder( QDataStream::LittleEndian );
        out << ( quint16 )0xACED;
        out << ( quint32 )libraryFile->getItemsSize();
        out << libraryFile->getVersion();

        bool isSuccess = false;

        DatFormat *datFormat = g_formatHandler.getFormatByClient( libraryFile->getVersion() );
        if( !datFormat ) {
            emit documentError( m_name, tr( "Unknown format: %1" ).arg( libraryFile->getVersion() ), -1 );
            return;
        }

        setMinimum( 0 );
        setMaximum( libraryFile->getCount()-1 );
        emit valueChanged( 0 );

        QTime rate;
        rate.start();
        quint32 count = 0;
        quint32 now = libraryFile->pos();
        quint32 offset = now + libraryFile->getItemsSize() * sizeof( quint32 );
        for( quint32 i = 0; i < libraryFile->getCount(); i++ ) {
            if( isCanceled() ) {
                isSuccess = false;
                break;
            }

            ItemData itemData = libraryFile->getItemData( i );
            libraryFile->seek( now );
            if( itemData.isNull() ) {
                out << ( quint32 )0x00000000;
                now += sizeof( quint32 );
                continue;
            }

            out << offset;
            libraryFile->seek( offset );

            isSuccess = ExternalFile::saveItem( out, itemData, datFormat );
            if( !isSuccess )
                break;

            offset = libraryFile->pos();
            now += sizeof( quint32 );
            emit valueChanged( i );

            if( ( float )( rate.elapsed() / 1000 ) >= 1 ) {
                emit labelText( tr( "%1\n\nElapsed Time: %2\nTime left: %3" ).arg( m_labelText ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( int )( ( ( float )( libraryFile->getCount() - i ) / count ) * ( float )1000 ) ).toString( "hh:mm:ss" ) ) );
                //setLabel( tr( "Saving Library...\nElapsed Time: %1\n%2Time left: %2" ).arg( QTime().addMSecs( time.elapsed() ).toString( "hh:mm:ss" ) ).arg( QTime().addMSecs( ( libraryFile->getCount() - i ) / count ).toString( "hh:mm:ss" ) ) );
                //emit rateChanged( count, libraryFile->getCount() - i, time.elapsed() ); // Items/sec, Items remaining
                rate.restart();
                count = 0;
            }

            count++;
        }

        if( !isCanceled() ) {
            if( isSuccess ) {
                if( libraryFile->mustIdle() )
                    isSuccess = libraryFile->idle( m_name, true ); // Upon saving we must re-idle our new file to continue
            }

            if( !libraryFile->mustIdle() )
                libraryFile->close();

            emit success( time.elapsed() );
        }
    }
}
