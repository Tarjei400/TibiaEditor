#ifndef TIBIAOBJECT_H
#define TIBIAOBJECT_H

#include <QtCore/QMutex>
#include <QtCore/QMap>

typedef QMap<quint32, quint16> SpriteList;

class TibiaObject
{
public:
    TibiaObject( void ) {};
    virtual ~TibiaObject( void ) {};

    virtual void reset() = 0;
    const QString& getSource( void ) const {
        return source;
    };
    void setSource( const QString& src ) {
        source = src;
    };

private:
    QString source;
    friend class TibiaItem;
    friend class TibiaSprite;
};

typedef QVector<TibiaObject *> ObjectVector;

#endif // TIBIAOBJECT_H
