#include <math.h>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtCore/QMutableHashIterator>
#include <QtCore/QMutableListIterator>

#include "formathandler.h"
#include "itemattributes.h"

extern FormatHandler g_formatHandler;

ItemAttributes::ItemAttributes( QWidget *parent ) : QWidget( parent ), ui( new Ui::ItemAttributesClass )
{
    ui->setupUi( this );
    onSetupContents();
}

ItemAttributes::~ItemAttributes( void )
{
    onUnloadContents();
    delete ui;
}

void ItemAttributes::onUnloadContents( void )
{
    foreach( QWidget* widget, widgetHash ) {
        if( widget )
            widget->deleteLater();
    }

    widgetHash.clear();
}

void ItemAttributes::onSetupContents( void )
{
    DatFormat *format = g_formatHandler.getBaseFormat();
    if( !format )
        return;

    for( DatPropertyList::const_iterator prop = format->getPropertiesBegin(); prop != format->getPropertiesEnd(); prop++ ) {
        BaseProperty *baseProperty = ( *prop )->getBaseProperty();
        if( baseProperty->getBase() != 0xFF ) {
            QCheckBox *checkBox = new QCheckBox( ui->propertyContents );
            checkBox->setObjectName( QString::fromUtf8( "optionCheck_%1" ).arg( baseProperty->getHeader() ) );
            checkBox->setText( baseProperty->getName() );
            checkBox->setToolTip( baseProperty->getToolTip() );
            ui->propertyLayout->addWidget( checkBox );
            widgetHash.insertMulti( baseProperty->getBase(), checkBox );


            foreach( quint8 size, baseProperty->getValues().uniqueKeys() ) {
                int index = 0;
                foreach( DatValue* datValue, baseProperty->getValues().values( size ) ) {
                    BaseValue *baseValue = datValue->getBaseValue();
                    if( baseValue->getSize() > 0 ) {
                        QHBoxLayout *horizontalLayout = new QHBoxLayout;

                        QLabel *label = new QLabel( ui->valueContents );
                        label->setObjectName( QString::fromUtf8( "optionLabel_%1_%2_%3" ).arg( baseProperty->getHeader() ).arg( index ).arg( baseValue->getSize() ) );
                        label->setText( baseValue->getName() );
                        label->setToolTip( baseValue->getToolTip() );
                        label->setEnabled( false );
                        horizontalLayout->addWidget( label );

                        QSpinBox *spinBox = new QSpinBox( ui->valueContents );
                        spinBox->setObjectName( QString::fromUtf8( "optionValue_%1_%2_%3" ).arg( baseProperty->getHeader() ).arg( index ).arg( baseValue->getSize() ) );
                        spinBox->setButtonSymbols( QAbstractSpinBox::NoButtons );
                        spinBox->setMinimum( 0 );
                        spinBox->setMaximum( pow( ( long double )256, ( int )baseValue->getSize() ) );
                        spinBox->setToolTip( baseValue->getToolTip() );
                        spinBox->setEnabled( false );
                        horizontalLayout->addWidget( spinBox );

                        ui->valueLayout->addLayout( horizontalLayout );

                        QObject::connect( checkBox, SIGNAL( toggled( bool ) ), spinBox, SLOT( setEnabled( bool ) ) );
                        QObject::connect( checkBox, SIGNAL( toggled( bool ) ), label, SLOT( setEnabled( bool ) ) );

                        widgetHash.insertMulti( baseProperty->getBase(), label );
                        widgetHash.insertMulti( baseProperty->getBase(), spinBox );
                        index++;
                    }
                }
            }
            /*for( DatValueHash::const_iterator it = baseProperty->getValuesBegin(); it != baseProperty->getValuesEnd(); it++ )
            {
                BaseValue* baseValue = it.value()->getBaseValue();
                if( baseValue->getSize() > 0 )
                {
                    QHBoxLayout* horizontalLayout = new QHBoxLayout;

                    QLabel* label = new QLabel( ui->valueContents );
                    label->setObjectName( QString::fromUtf8( "optionLabel_%1_%2_%3" ).arg( baseProperty->getHeader() ).arg( index ).arg( baseValue->getSize() ) );
                    label->setText( baseValue->getName() );
                    label->setToolTip( baseValue->getToolTip() );
                    label->setEnabled( false );
                    horizontalLayout->addWidget( label );

                    QSpinBox* spinBox = new QSpinBox( ui->valueContents );
                    spinBox->setObjectName( QString::fromUtf8( "optionValue_%1_%2_%3" ).arg( baseProperty->getHeader() ).arg( index ).arg( baseValue->getSize() ) );
                    spinBox->setButtonSymbols( QAbstractSpinBox::NoButtons );
                    spinBox->setMinimum( 0 );
                    spinBox->setMaximum( pow( ( long double )256, ( int )baseValue->getSize() ) );
                    spinBox->setToolTip( baseValue->getToolTip() );
                    spinBox->setEnabled( false );
                    horizontalLayout->addWidget( spinBox );

                    ui->valueLayout->addLayout( horizontalLayout );

                    QObject::connect( checkBox, SIGNAL( toggled( bool ) ), spinBox, SLOT( setEnabled( bool ) ) );
                    QObject::connect( checkBox, SIGNAL( toggled( bool ) ), label, SLOT( setEnabled( bool ) ) );

                    widgetHash.insertMulti( baseProperty->getBase(), label );
                    widgetHash.insertMulti( baseProperty->getBase(), spinBox );
                    index++;
                }
            }*/
        }
    }
}

void ItemAttributes::setFormat( DatFormat *datFormat )
{
    m_datFormat = datFormat;

    bool hideAll = false;
    if( !datFormat )
        hideAll = true;

    QListIterator<quint8> propIt( widgetHash.uniqueKeys() );
    while( propIt.hasNext() ) {
        quint8 property = propIt.next();
        bool showAttribute = hideAll ? false : datFormat->hasProperty( property );
        QListIterator<QWidget *> widgetIt( widgetHash.values( property ) );
        while( widgetIt.hasNext() ) {
            QWidget *widget = widgetIt.next();
            if( widget )
                widget->setShown( showAttribute );
        }
    }
}

void ItemAttributes::onResetContents( void )
{
    foreach( QWidget* widget, widgetHash ) {
        if( QCheckBox *checkBox = qobject_cast<QCheckBox *>( widget ) ) {
            checkBox->blockSignals( true );
            checkBox->setChecked ( false );
            checkBox->blockSignals( false );
        }
        if( QLabel *label = qobject_cast<QLabel *>( widget ) )
            label->setEnabled( false );
        if( QSpinBox *spinBox = qobject_cast<QSpinBox *>( widget ) ) {
            spinBox->blockSignals( true );
            spinBox->setValue( spinBox->minimum() );
            spinBox->blockSignals( false );
            spinBox->setEnabled( false );
        }
    }
}

void ItemAttributes::setAttributes( const PropertyList& properties )
{
    onResetContents();

    if( !m_datFormat )
        m_datFormat = g_formatHandler.getBaseFormat();

    for( PropertyList::const_iterator prop = properties.begin(); prop != properties.end(); prop++ ) {
        ItemProperty itemProperty = ( *prop );
        QCheckBox *checkBox = getHeader( itemProperty.header );
        if( checkBox ) {
            checkBox->blockSignals( true );
            checkBox->setChecked ( true );
            checkBox->blockSignals( false );
        }

        int shortCount = 0;
        for( QVector<quint16>::const_iterator itSh = itemProperty.childU16.begin(); itSh != itemProperty.childU16.end(); itSh++ ) {
            QLabel *label = getLabel( itemProperty.header, shortCount, sizeof( quint16 ) );
            if( label )
                label->setEnabled( true );
            QSpinBox *spinBox = getValue( itemProperty.header, shortCount, sizeof( quint16 ) );
            if( spinBox ) {
                spinBox->blockSignals( true );
                spinBox->setValue( (int)( *itSh ) );
                spinBox->blockSignals( false );
                spinBox->setEnabled( true );
            }

            shortCount++;
        }

        int charCount = 0;
        for( QVector<quint8>::const_iterator itCh = itemProperty.childU8.begin(); itCh != itemProperty.childU8.end(); itCh++ ) {
            QLabel *label = getLabel( itemProperty.header, charCount, sizeof( quint8 ) );
            if( label )
                label->setEnabled( true );
            QSpinBox *spinBox = getValue( itemProperty.header, charCount, sizeof( quint8 ) );
            if( spinBox ) {
                spinBox->blockSignals( true );
                spinBox->setValue( (int)( *itCh ) );
                spinBox->blockSignals( false );
                spinBox->setEnabled( true );
            }

            charCount++;
        }
    }
}

void ItemAttributes::setHeaders( const HeaderList& headerList )
{
    for( HeaderList::const_iterator head = headerList.begin(); head != headerList.end(); head++ ) {
        DatProperty *datProperty = m_datFormat->getPropertyByHeader( *head );
        if( datProperty ) {
            if( QCheckBox *checkBox = getHeader( datProperty->getBase() ) ) {
                checkBox->blockSignals( true );
                checkBox->setChecked( true );
                checkBox->blockSignals( false );
            }
        }
    }
}

void ItemAttributes::getHeaders( HeaderList& headerList )
{
    for( DatPropertyList::const_iterator prop = m_datFormat->getPropertiesBegin(); prop != m_datFormat->getPropertiesEnd(); prop++ ) {
        DatProperty *datProperty = ( *prop );
        if( QCheckBox *checkBox = getHeader( datProperty->getBase() ) ) {
            if( checkBox->isChecked() )
                headerList.push_back( datProperty->getBase() );
        }
    }
}

void ItemAttributes::getAttributes( PropertyList& properties )
{
    properties.clear();

    if( !m_datFormat )
        m_datFormat = g_formatHandler.getBaseFormat();

    for( DatPropertyList::const_iterator prop = m_datFormat->getPropertiesBegin(); prop != m_datFormat->getPropertiesEnd(); prop++ ) {
        DatProperty *datProperty = ( *prop );
        if( QCheckBox *checkBox = getHeader( datProperty->getBase() ) ) {
            if( checkBox->isChecked() ) {
                ItemProperty itemProperty;
                itemProperty.header = datProperty->getBase();
                int index = 0;
                BaseProperty *baseProperty = g_formatHandler.getBaseProperty( datProperty->getBase() );
                if( baseProperty ) {
                    for( DatValueHash::const_iterator it = baseProperty->getValuesBegin(); it != baseProperty->getValuesEnd(); it++ ) {
                        BaseValue *baseValue = it.value()->getBaseValue();
                        if( baseValue ) {
                            if( QSpinBox *spinBox = getValue( datProperty->getBase(), index, baseValue->getSize() ) ) {
                                if( baseValue->getSize() == sizeof( quint8 ) )
                                    itemProperty.childU8.push_back ( spinBox->value() );
                                if( baseValue->getSize() == sizeof( quint16 ) )
                                    itemProperty.childU16.push_back ( spinBox->value() );
                            }
                        }

                        index++;
                    }
                }

                properties.push_back( itemProperty );
            }
        }
    }
}

QCheckBox *ItemAttributes::getHeader( quint8 header ) const
{
    if( QCheckBox *checkBox = findChild<QCheckBox *>( QString( "optionCheck_%1" ).arg( header ) ) )
        return checkBox;

    return NULL;
}

QLabel *ItemAttributes::getLabel( quint8 header, quint8 index, quint8 size ) const
{
    if( QLabel *label = findChild<QLabel *>( QString( "optionLabel_%1_%2_%3" ).arg( header ).arg( index ).arg( size ) ) )
        return label;

    return NULL;
}

QSpinBox *ItemAttributes::getValue( quint8 header, quint8 index, quint8 size ) const
{
    if( QSpinBox *spinBox = findChild<QSpinBox *>( QString( "optionValue_%1_%2_%3" ).arg( header ).arg( index ).arg( size ) ) )
        return spinBox;

    return NULL;
}
