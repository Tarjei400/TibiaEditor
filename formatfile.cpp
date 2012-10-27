#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include "formatfile.h"


/*












            REVISE THIS FILE ASAP!!!!!!!!!!!



            FormatHandler g_formatHandler






*/

FormatFile::FormatFile( QObject *parent ) : TibiaFile( parent )
{
    _loaded = false;
}

FormatFile::~FormatFile( void )
{
    formats.clear();
}

void FormatFile::unload( void )
{
    TibiaFile::unload();

    _loaded = false;
    formats.clear();
}

bool FormatFile::load( const QString& fileName )
{
    setFileName( fileName );

    if( !TibiaFile::open( QIODevice::ReadOnly ) ) {
        emit parseError( QObject::tr( "Open Error" ), TibiaFile::error() );
        return false;
    }

    QString error;
    int line;

    QDomDocument doc( "formats" );
    if( !doc.setContent( this, true, &error, &line ) ) {
        emit documentError( fileName, error, line );
        TibiaFile::close();
        return false;
    }

    TibiaFile::close();

    QDomElement docElem = doc.documentElement();
    if( docElem.tagName().compare( "formats", Qt::CaseInsensitive ) == 0 ) {
        QDomNode node = docElem.firstChild();
        while( !node.isNull() ) {
            if( node.isElement() ) {
                QDomElement element = node.toElement();
                if( !element.isNull() ) {
                    if( element.tagName().compare( "format", Qt::CaseInsensitive ) == 0 ) {
                        ItemFormat format;
                        format.name = element.attribute( "name" );
                        format.version = element.attribute( "version" ).toInt();
                        format.redirect = element.attribute( "redirect" ).toInt();
                        format.ZDivFactor = element.attribute( "zdiv", "1" ).toInt();

                        QDomElement subelement = element.firstChildElement( "property" );
                        while( !subelement.isNull() ) {
                            FormatProperty formatProperty;
                            formatProperty.header = subelement.attribute( "header", "255" ).toInt();
                            if( format.version == -1 )
                                formatProperty.base = formatProperty.header;
                            else
                                formatProperty.base = subelement.attribute( "base", "255" ).toInt();
                            formatProperty.name = subelement.attribute( "name" );
                            formatProperty.tooltip = subelement.attribute( "tooltip" );

                            QDomElement child = subelement.firstChildElement( "child" );
                            while( !child.isNull() ) {
                                PropertyValue propertyValue;
                                propertyValue.size = child.attribute( "size", "0" ).toInt();
                                propertyValue.name = child.attribute( "name" );
                                propertyValue.tooltip = child.attribute( "tooltip" );
                                formatProperty.propertyValues.push_back( propertyValue );

                                child = child.nextSiblingElement( "child" );
                            }

                            format.properties.push_back( formatProperty );

                            subelement = subelement.nextSiblingElement( "property" );
                        }

                        format.valid = true;
                        formats.insert( format.version, format );
                    }
                }
            }
            node = node.nextSibling();
        }
    } else {
        emit documentError( fileName, tr( "Invalid Element: " ) + docElem.tagName(), -1 );
        return false;
    }

    _loaded = true;
    return true;
}

quint8 ItemFormat::getBase( quint8 header )
{
    for( FormatPropertyVector::iterator it = properties.begin(); it != properties.end(); it++ ) {
        FormatProperty itemProperty = (*it);
        if( itemProperty.header == header )
            return itemProperty.base;
    }

    return 0xFF;
}

quint8 ItemFormat::getHeader( quint8 base )
{
    for( FormatPropertyVector::iterator it = properties.begin(); it != properties.end(); it++ ) {
        FormatProperty itemProperty = (*it);
        if( itemProperty.base == base )
            return itemProperty.header;
    }

    return 0xFF;
}

FormatProperty ItemFormat::getPropertyByBase( quint8 base ) const
{
    for( FormatPropertyVector::const_iterator it = properties.begin(); it != properties.end(); it++ ) {
        FormatProperty itemProperty = (*it);
        if( itemProperty.base == base )
            return itemProperty;
    }

    return FormatProperty();
}

FormatProperty ItemFormat::getPropertyByHeader( quint8 header ) const
{
    for( FormatPropertyVector::const_iterator it = properties.begin(); it != properties.end(); it++ ) {
        FormatProperty itemProperty = (*it);
        if( itemProperty.header == header )
            return itemProperty;
    }

    return FormatProperty();
}

ItemFormat FormatFile::getFormatByName( const QString& version ) const
{
    for( FormatMap::const_iterator it = formats.begin(); it != formats.end(); it++ ) {
        ItemFormat format = (*it);
        if( format.name.compare( version, Qt::CaseInsensitive ) == 0 )
            return getFormatByClient( format.version ); // Do any redirecting if its redirected
    }

    return formats.value( BASE_FORMAT_KEY ); // All versions failed, lets get our base format
}

ItemFormat FormatFile::getFormatByClient( qint32 version ) const
{
    ItemFormat format = formats.value( version );
    if( format.version == 0 ) { // Our found version failed
        return formats.value( BASE_FORMAT_KEY );
    }
    if( format.redirect != 0 ) { // Our version is a redirected version
        format = formats.value( format.redirect );
    }
    if( format.version == 0 ) { // Our redirected version is invalid
        return formats.value( BASE_FORMAT_KEY );
    }

    return format;
}
