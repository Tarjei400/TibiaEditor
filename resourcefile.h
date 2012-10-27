#ifndef RESOURCEFILE_H
#define RESOURCEFILE_H

#include <QtCore/QTemporaryFile>
#include <QtCore/QDir>
#include "tibiasprite.h"

class ResourceFile : public QTemporaryFile
{
    Q_OBJECT

public:
    ResourceFile( const QDir& directory, quint32 identifier, const TibiaSprite& tibiaSprite, QObject *parent = NULL );
    ~ResourceFile( void ) {};

    TibiaSprite getSprite( void );
    const QString& getSource( void ) const {
        return m_source;
    };

private:
    quint32 m_id;
    QString m_source;
};

#endif // RESOURCEFILE_H
