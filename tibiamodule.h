#ifndef TIBIAMODULE_H
#define TIBIAMODULE_H

#include "tibiafile.h"
#include "tibiasprite.h"

class TibiaModule : public TibiaFile
{
public:
    TibiaModule( QObject *parent ) : TibiaFile( parent ) {
        _usecount = 0;
    };
    virtual ~TibiaModule() {};

    void use( void ) {
        QMutexLocker locker( &mutex );
        _usecount++;
    };

    void free( void ) {
        QMutexLocker locker( &mutex );
        _usecount--;
        if( _usecount <= 0 ) {
            unload();

            if( isOpen() )
                close();

            deleteLater();
        }
    };

    virtual TibiaModule *getTibiaModule() {
        return this;
    };
    virtual const TibiaModule *getTibiaModule() const {
        return this;
    };

    qint32 getUseCount( void ) const {
        return _usecount;
    };

    virtual TibiaSprite getSprite( qint32, qint32 ) {
        return dummy;
    }; // Index of item, frame of sprite

private:
    mutable QMutex mutex;
    qint32 _usecount;
};

#endif // TIBIAMODULE_H
