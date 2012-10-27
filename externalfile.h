#ifndef EXTERNALFILE_H
#define EXTERNALFILE_H

#include <QtCore/QDataStream>

#include "tibiafile.h"
#include "itemfile.h"

class ExternalFile : public TibiaModule
{
    Q_OBJECT

public:
    ExternalFile( QObject *parent = NULL );
    ExternalFile( quint16 version, const QString& name, const ItemData& item, QObject *parent = NULL );
    virtual ~ExternalFile( void );

    bool isSuccess( void ) const {
        return m_success;
    };
    virtual bool isLoaded( void ) const {
        return m_loaded;
    };
    virtual void unload( void );
    virtual bool load( const QString& ) {
        return false;
    };
    virtual bool save( const QString& );
    virtual bool idle( const QString&, bool );
    virtual TibiaSprite getSprite( qint32, qint32 );
    static bool readItem( QDataStream& in, ItemData& itemData, DatFormat *datFormat, TibiaModule *tibiaModule, qint32 index, quint32& address, QString& error );
    static bool saveItem( QDataStream& out, const ItemData& itemData, DatFormat *datFormat );
    void setItemData( const ItemData& item );
    ItemData getItemData( void );
    void setVersion( quint16 version ) {
        m_version = version;
    };

private:
    DatFormat *m_datFormat;
    bool m_success;
    bool m_loaded;
    bool m_mustIdle;
    quint16 m_version;
    quint32 m_address;
    ItemData m_internalItemData;
    mutable QMutex mutex;
};

#endif // EXTERNALFILE_H
