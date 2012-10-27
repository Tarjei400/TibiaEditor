#include <QtCore/QMutex>
#include "resourcefile.h"

ResourceFile::ResourceFile( const QDir& directory, quint32 identifier, const TibiaSprite& tibiaSprite, QObject *parent ) : QTemporaryFile( parent )
{
    m_id = identifier;
    m_source = tibiaSprite.getSource();
    setFileTemplate( directory.path() + QString( "/%1_XXXXXX.PNG" ).arg( identifier ) );
    if( open() )
        tibiaSprite.image.save( this, "PNG" );
}

TibiaSprite ResourceFile::getSprite( void )
{
    QMutex mutex;
    QMutexLocker locker( &mutex );
    seek( 0 );
    QImage image;
    image.load( this, "PNG" );
    TibiaSprite tibiaSprite;
    tibiaSprite.id = m_id;
    tibiaSprite.setSource( m_source );
    tibiaSprite.width = image.width() / 32;
    tibiaSprite.height = image.height() / 32;
    tibiaSprite.setImage( image );
    return tibiaSprite;
}
