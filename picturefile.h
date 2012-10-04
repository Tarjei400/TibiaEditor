#ifndef PICTUREFILE_H
#define PICTUREFILE_H

#include "tibiafile.h"
#include "tibiasprite.h"
#include "spritefile.h"

class UserThread;

class PictureFile : public TibiaFile
{
    Q_OBJECT

public:
    PictureFile( QObject *parent );
    virtual ~PictureFile( void );

    virtual bool createNew( void );
    virtual bool isLoaded( void ) const {
        return m_loaded;
    };
    virtual void unload( void );
    virtual bool load( const QString& ) {
        return false;
    };
    //virtual bool save( const QString& );
    virtual bool save( const QString& ) {
        return false;
    };
    virtual bool idle( const QString&, bool );

    TibiaSprite loadPicture( quint16 pictureId );

    virtual PictureFile *getPictureFile() {
        return this;
    };
    virtual const PictureFile *getPictureFile() const {
        return this;
    };

    quint16 getCount( void ) const {
        return m_count;
    };
    void setCount( quint16 count ) {
        m_count = count;
    };

    TibiaSprite getPicture( quint16 id );

    UserThread *getThread( void ) const {
        return m_thread;
    };

private:
    quint16 m_count;
    UserThread *m_thread;

    bool m_loaded;
    mutable QMutex mutex;
};

#endif // PICTUREFILE_H
