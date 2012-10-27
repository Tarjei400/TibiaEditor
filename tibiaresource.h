#ifndef TIBIARESOURCE_H
#define TIBIARESOURCE_H

#include <QtCore/QSharedPointer>
#include <QtCore/QHash>
#include <QtCore/QFile>

class TibiaSprite;

class TibiaResource
{
public:
    TibiaResource( quint8 resourceType, quint32 identifier, quint16 frame = 0, QFile *file = NULL );
    ~TibiaResource( void );

    quint8 getType( void ) const {
        return m_type;
    };
    quint32 getIdentifier( void ) const {
        return m_id;
    };
    quint16 getFrame( void ) const {
        return m_frame;
    };
    QFile *getFile( void ) const {
        return m_file;
    };
    void reset( void );
    void use( void );
    void free( void );
    void setType( quint8 type ) {
        m_type = type;
    };
    void setIdentifier( const quint32 id ) {
        m_id = id;
    };
    void setFrame( const quint16 frame ) {
        m_frame = frame;
    };
    void setFile( QFile *file, bool load = true );
    void unhook( void );
    void resetCache( void );
    TibiaSprite getTibiaSprite( void  );

private:
    quint8 m_type;
    quint32 m_id;
    quint16 m_frame;
    QFile *m_file;
};

typedef QSharedPointer<TibiaResource> SharedResource;

QDataStream& operator<< ( QDataStream& stream, const SharedResource& sharedResource );
QDataStream& operator>> ( QDataStream& stream, SharedResource& sharedResource );

typedef QHash<quint32, SharedResource> ResourceHash;

#endif // TIBIARESOURCE_H
