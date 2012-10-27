#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomNode>
#include "formathandler.h"

FormatHandler::FormatHandler( QObject *parent ) : QObject( parent )
{
    m_loaded = false;
}

FormatHandler::~FormatHandler( void )
{
    qDeleteAll( m_formats );
    m_formats.clear();
}

bool FormatHandler::loadFile( const QString& fileName )
{
    QFile file( fileName );
    if( !file.open( QIODevice::ReadOnly ) ) {
        emit parseError( QObject::tr( "Open Error" ), file.error() );
        return false;
    }

    QString error;
    int line;

    QDomDocument doc( "formats" );
    if( !doc.setContent( &file, true, &error, &line ) ) {
        emit documentError( fileName, error, line );
        file.close();
        return false;
    }

    file.close();

    QDomElement docElem = doc.documentElement();
    if( docElem.tagName().compare( "formats", Qt::CaseInsensitive ) == 0 ) {
        QDomNode node = docElem.firstChild();
        while( !node.isNull() ) {
            if( node.isElement() ) {
                QDomElement element = node.toElement();
                if( !element.isNull() ) {
                    if( element.tagName().compare( "format", Qt::CaseInsensitive ) == 0 ) {
                        DatFormat *datFormat = new DatFormat;
                        datFormat->name = element.attribute( "name" );
                        datFormat->version = element.attribute( "version" ).toInt();
                        datFormat->redirect = element.attribute( "redirect" ).toInt();
                        datFormat->ZDivFactor = element.attribute( "zdiv", "1" ).toInt();
                        bool ok;
                        datFormat->signature = element.attribute( "signature" ).toInt( &ok, 16 );

                        QDomElement subelement = element.firstChildElement( "property" );
                        while( !subelement.isNull() ) {
                            if( datFormat->version == -1 ) {
                                BaseProperty *baseProperty = new BaseProperty;
                                baseProperty->header = subelement.attribute( "header", "255" ).toInt();
                                baseProperty->base = baseProperty->header;
                                baseProperty->name = subelement.attribute( "name" );
                                baseProperty->tooltip = subelement.attribute( "tooltip" );

                                QDomElement child = subelement.firstChildElement( "child" );
                                while( !child.isNull() ) {
                                    BaseValue *baseValue = new BaseValue;
                                    baseValue->size = child.attribute( "size", "0" ).toInt();
                                    baseValue->name = child.attribute( "name" );
                                    baseValue->tooltip = child.attribute( "tooltip" );
                                    baseProperty->valueHash.insertMulti( baseValue->size, baseValue );

                                    child = child.nextSiblingElement( "child" );
                                }

                                datFormat->propertyList.push_back( baseProperty );
                            } else {
                                DatProperty *datProperty = new DatProperty;
                                datProperty->header = subelement.attribute( "header", "255" ).toInt();
                                datProperty->base = subelement.attribute( "base", "255" ).toInt();

                                QDomElement child = subelement.firstChildElement( "child" );
                                while( !child.isNull() ) {
                                    DatValue *datValue = new DatValue;
                                    datValue->size = child.attribute( "size", "0" ).toInt();
                                    datProperty->valueHash.insertMulti( datValue->size, datValue );

                                    child = child.nextSiblingElement( "child" );
                                }

                                datFormat->propertyList.push_back( datProperty );
                            }

                            subelement = subelement.nextSiblingElement( "property" );
                        }

                        m_formats.insert( datFormat->version, datFormat );
                    }
                }
            }
            node = node.nextSibling();
        }
    } else {
        emit documentError( fileName, tr( "Invalid Element: " ) + docElem.tagName(), -1 );
        return false;
    }

    m_loaded = true;
    return true;
}

DatProperty *DatFormat::getPropertyByBase( quint8 base ) const
{
    for( DatPropertyList::const_iterator it = propertyList.begin(); it != propertyList.end(); it++ ) {
        DatProperty *itemProperty = ( *it );
        if( itemProperty->getBase() == base )
            return itemProperty;
    }

    return NULL;
}

DatProperty *DatFormat::getPropertyByHeader( quint8 header ) const
{
    for( DatPropertyList::const_iterator it = propertyList.begin(); it != propertyList.end(); it++ ) {
        DatProperty *itemProperty = ( *it );
        if( itemProperty->getHeader() == header )
            return itemProperty;
    }

    return NULL;
}

bool DatFormat::hasProperty( quint8 base ) const
{
    for( DatPropertyList::const_iterator it = propertyList.begin(); it != propertyList.end(); it++ ) {
        DatProperty *itemProperty = ( *it );
        if( itemProperty->getBase() == base )
            return true;
    }

    return false;
}

BaseProperty *FormatHandler::getBaseProperty( quint8 base ) const
{
    DatFormat *datFormat = getBaseFormat();
    if( datFormat ) {
        foreach( DatProperty* datProperty, datFormat->propertyList ) {
            if( BaseProperty *baseProperty = datProperty->getBaseProperty() ) {
                if( baseProperty->header == base )
                    return baseProperty;
            }
        }
    }

    return NULL;
}

DatFormat *FormatHandler::getBaseFormat( void ) const
{
    return m_formats.value( BASE_FORMAT_KEY );
}

DatFormat *FormatHandler::getFormatBySignature( quint32 signature ) const
{
    foreach( DatFormat* datFormat, m_formats ) {
        if( datFormat->signature == signature )
            return getFormatByClient( datFormat->getVersion() );
    }

    return getBaseFormat();
}

DatFormat *FormatHandler::getFormatByName( const QString& version ) const
{
    foreach( DatFormat* datFormat, m_formats ) {
        if( datFormat->getName().compare( version, Qt::CaseInsensitive ) == 0 )
            return getFormatByClient( datFormat->getVersion() );
    }

    return getBaseFormat(); // All versions failed, lets get our base format
}

DatFormat *FormatHandler::getFormatByClient( qint32 version ) const
{
    DatFormat *format = m_formats.value( version );
    if( !format ) {
        return getBaseFormat();
    }
    if( format->version == 0 ) { // Our found version failed
        return getBaseFormat();
    }
    if( format->redirect != 0 ) { // Our version is a redirected version
        format = m_formats.value( format->getRedirect() );
    }
    if( format->version == 0 ) { // Our redirected version is invalid
        return getBaseFormat();
    }

    return format;
}
