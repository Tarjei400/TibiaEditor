#ifndef FORMATHANDLER_H
#define FORMATHANDLER_H

#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QMultiHash>
#include <QtCore/QMap>

#define BASE_FORMAT_KEY -1

class BaseProperty;
class BaseValue;

class DatValue
{
public:
    DatValue( void ) {
        size = 0;
    };
    ~DatValue( void ) {};

    quint8 getSize( void ) const {
        return size;
    };

    virtual BaseValue *getBaseValue( void ) {
        return NULL;
    };
    virtual const BaseValue *getBaseValue( void ) const {
        return NULL;
    };

private:
    quint8 size;

    friend class FormatHandler;
};

class BaseValue : public DatValue
{
public:
    BaseValue( void ) {
        name = QString();
        tooltip = QString();
    };

    virtual BaseValue *getBaseValue( void ) {
        return this;
    };
    virtual const BaseValue *getBaseValue( void ) const {
        return this;
    };

    const QString& getName( void ) const {
        return name;
    };
    const QString& getToolTip( void ) const {
        return tooltip;
    };

private:
    QString name;
    QString tooltip;

    friend class FormatHandler;
};

typedef QMultiHash<quint8, DatValue *> DatValueHash;

class DatProperty
{
public:
    DatProperty( void ) {
        header = 0xFF;
        base = 0xFF;
    };
    ~DatProperty( void ) {
        qDeleteAll( valueHash );
        valueHash.clear();
    };

    virtual BaseProperty *getBaseProperty( void ) {
        return NULL;
    };
    virtual const BaseProperty *getBaseProperty( void ) const {
        return NULL;
    };

    quint8 getHeader( void ) const {
        return header;
    };
    quint8 getBase( void ) const {
        return base;
    };

    DatValueHash getValues( void ) const {
        return valueHash;
    };
    inline DatValueHash::const_iterator getValuesBegin() const {
        return valueHash.begin();
    }
    inline DatValueHash::const_iterator getValuesEnd() const {
        return valueHash.end();
    }
    quint32 getValuesSize( void ) const {
        return valueHash.size();
    };

private:
    quint8 header;
    quint8 base;

    DatValueHash valueHash;

    friend class FormatHandler;
};

class BaseProperty : public DatProperty
{
public:
    BaseProperty( void ) {
        name = QString();
        tooltip = QString();
    };

    const QString& getName( void ) const {
        return name;
    };
    const QString& getToolTip( void ) const {
        return tooltip;
    };

    virtual BaseProperty *getBaseProperty( void ) {
        return this;
    };
    virtual const BaseProperty *getBaseProperty( void ) const {
        return this;
    };

private:
    QString name;
    QString tooltip;

    friend class FormatHandler;
};

typedef QList<DatProperty *> DatPropertyList;

class DatFormat
{
public:
    DatFormat( void ) {
        name = QString();
        redirect = 0;
        version = 0;
        signature = 0;
        ZDivFactor = true;
    };
    ~DatFormat( void ) {
        qDeleteAll( propertyList );
        propertyList.clear();
    };

    const QString& getName( void ) const {
        return name;
    };
    qint32 getVersion( void ) const {
        return version;
    };
    qint32 getRedirect( void ) const {
        return redirect;
    };
    bool hasZFactor( void ) const {
        return ZDivFactor;
    };
    bool hasProperty( quint8 base ) const;

    inline DatPropertyList::const_iterator getPropertiesBegin() const {
        return propertyList.begin();
    }
    inline DatPropertyList::const_iterator getPropertiesEnd() const {
        return propertyList.end();
    }
    quint32 getPropertiesSize( void ) const {
        return propertyList.size();
    };

    DatProperty *getPropertyByBase( quint8 base ) const;
    DatProperty *getPropertyByHeader( quint8 header ) const;

    DatFormat *getBaseFormat( void ) const;

private:
    QString name;
    qint32 version;
    qint32 redirect;
    quint32 signature;
    bool ZDivFactor;
    DatPropertyList propertyList;

    friend class FormatHandler;
};

typedef QMap<qint32, DatFormat *> DatFormatMap;

class FormatHandler : public QObject
{
    Q_OBJECT

public:
    FormatHandler( QObject *parent = 0 );
    ~FormatHandler( void );

    bool loadFile( const QString& );

    inline DatFormatMap::const_iterator getFormatsBegin() const {
        return m_formats.begin();
    }
    inline DatFormatMap::const_iterator getFormatsEnd() const {
        return m_formats.end();
    }

    BaseProperty *getBaseProperty( quint8 base ) const;
    DatFormat *getBaseFormat( void ) const;
    DatFormat *getFormatBySignature( quint32 signature ) const;
    DatFormat *getFormatByName( const QString& version ) const;
    DatFormat *getFormatByClient( qint32 version ) const;

    bool isLoaded( void ) const {
        return m_loaded;
    };

signals:
    void parseError( QString, QFile::FileError );
    void documentError( QString, QString, int );

private:
    bool m_loaded;
    DatFormatMap m_formats;
};

#endif // FORMATHANDLER_H
