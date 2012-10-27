#ifndef FORMATFILE_H
#define FORMATFILE_H

#include <QtCore/QVector>
#include <QtCore/QMap>

#include "tibiafile.h"

#define BASE_FORMAT_KEY -1

class PropertyValue
{
public:
    PropertyValue( void ) {
        size = 0;
        name = QString();
        tooltip = QString();
    };
    ~PropertyValue( void ) {};

    quint8 size;
    QString name;
    QString tooltip;
};

typedef QVector<PropertyValue> PropertyValueVector;

class FormatProperty
{
public:
    FormatProperty( void ) {
        header = 0xFF;
        base = 0xFF;
        name = QString();
        tooltip = QString();
    };
    ~FormatProperty( void ) {};

    quint8 header;
    quint8 base;
    QString name;
    QString tooltip;

    PropertyValueVector propertyValues;
};

typedef QVector<FormatProperty> FormatPropertyVector;

class ItemFormat
{
public:
    ItemFormat( void ) {
        name = QString();
        redirect = 0;
        version = 0;
        ZDivFactor = true;
        valid = false;
    };
    ~ItemFormat( void ) {};

    bool isValid( void ) const {
        return valid;
    };

    quint8 getBase( quint8 header );
    quint8 getHeader( quint8 base );

    FormatProperty getPropertyByBase( quint8 base ) const;
    FormatProperty getPropertyByHeader( quint8 header ) const;

    QString name;
    qint32 version;
    qint32 redirect;
    bool ZDivFactor;
    bool valid;
    FormatPropertyVector properties;
};

typedef QMap<qint32, ItemFormat> FormatMap;

class FormatFile : public TibiaFile
{
    Q_OBJECT

public:
    FormatFile( QObject *parent );
    virtual ~FormatFile( void );

    virtual bool isLoaded( void ) const {
        return _loaded;
    };
    virtual void unload( void );
    virtual bool load( const QString& );
    virtual bool save( const QString& ) {
        return false;
    };
    virtual bool idle( const QString&, bool ) {
        return false;
    };

    ItemFormat getFormatByName( const QString& version ) const;
    ItemFormat getFormatByClient( qint32 version ) const;

    inline FormatMap::const_iterator getFormatsIteratorBegin() const {
        return formats.begin();
    }
    inline FormatMap::const_iterator getFormatsIteratorEnd() const {
        return formats.end();
    }

private:
    FormatMap formats;
    bool _loaded;
};

#endif // FORMATFILE_H
