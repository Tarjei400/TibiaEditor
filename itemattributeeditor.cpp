#include <QtGui/QPushButton>
#include "itemattributeeditor.h"
#include "itemmodel.h"
#include "tibiahandler.h"
#include "formathandler.h"

extern TibiaHandler g_tibiaHandler;
extern FormatHandler g_formatHandler;

ItemAttributeEditor::ItemAttributeEditor( QWidget *parent ) : QDialog( parent ), ui( new Ui::ItemAttributeEditorClass )
{
    ui->setupUi( this );
    ui->buttonBox->button( QDialogButtonBox::Apply )->setIcon( QIcon( ":/TibiaEditor/Resources/apply.png" ) );

    itemModel = new ItemModel( this );
    ui->itemView->setModel( itemModel );

    if( g_formatHandler.isLoaded() ) {
        for( DatFormatMap::const_iterator it = g_formatHandler.getFormatsBegin(); it != g_formatHandler.getFormatsEnd(); it++ )
            ui->comboVersions->insertItem( 0, it.value()->getName(), QVariant::fromValue( it.key() ) );
    } else
        ui->comboVersions->insertItem( 0, tr( "No Formats" ) );

    QObject::connect( ui->selectedItems, SIGNAL( toggled( bool ) ), this, SLOT( onSelectionChanged( bool ) ) );
    QObject::connect( ui->selectedOutfits, SIGNAL( toggled( bool ) ), this, SLOT( onSelectionChanged( bool ) ) );
    QObject::connect( ui->selectedEffects, SIGNAL( toggled( bool ) ), this, SLOT( onSelectionChanged( bool ) ) );
    QObject::connect( ui->selectedProjectiles, SIGNAL( toggled( bool ) ), this, SLOT( onSelectionChanged( bool ) ) );
    QObject::connect( ui->buttonBox, SIGNAL( clicked( QAbstractButton * ) ), this, SLOT( onButtonClicked( QAbstractButton * ) ) );
}

ItemAttributeEditor::~ItemAttributeEditor( void )
{
    if( itemModel ) {
        itemModel->clear();
        delete itemModel;
    }
    delete ui;
}

void ItemAttributeEditor::setItems( const ItemList& list )
{
    items = list;
}

void ItemAttributeEditor::setOutfits( const ItemList& list )
{
    outfits = list;
}

void ItemAttributeEditor::setEffects( const ItemList& list )
{
    effects = list;
}

void ItemAttributeEditor::setProjectiles( const ItemList& list )
{
    projectiles = list;
}

void ItemAttributeEditor::updateItems( void )
{
    if( itemModel ) {
        ItemList itemList;
        if( ui->selectedItems->isChecked() )
            itemList += items;
        if( ui->selectedOutfits->isChecked() )
            itemList += outfits;
        if( ui->selectedEffects->isChecked() )
            itemList += effects;
        if( ui->selectedProjectiles->isChecked() )
            itemList += projectiles;

        itemModel->setItemList( itemList );
    }
}

void ItemAttributeEditor::onSelectionChanged( bool )
{
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Selected Items Changed." ) );
    updateItems( );
}

void ItemAttributeEditor::on_comboVersions_currentIndexChanged( int index )
{
    qint32 version = ui->comboVersions->itemData( index, Qt::UserRole ).toInt();
    DatFormat *datFormat = g_formatHandler.getFormatByClient( version );
    if( datFormat ) {
        ui->itemAttributes->setFormat( datFormat );
    }
}

void ItemAttributeEditor::onButtonClicked( QAbstractButton *button )
{
    switch( ui->buttonBox->buttonRole( button ) ) {
    case QDialogButtonBox::ApplyRole: {
        PropertyList properties;
        ui->itemAttributes->getAttributes( properties );
        if( ui->selectedItems->isChecked() ) {
            foreach( TibiaItem* item, items )
            setProperties( item, properties );
        }
        if( ui->selectedOutfits->isChecked() ) {
            foreach( TibiaItem* item, outfits )
            setProperties( item, properties );
        }
        if( ui->selectedEffects->isChecked() ) {
            foreach( TibiaItem* item, effects )
            setProperties( item, properties );
        }
        if( ui->selectedProjectiles->isChecked() ) {
            foreach( TibiaItem* item, projectiles )
            setProperties( item, properties );
        }

        close();
    }
    break;
    case QDialogButtonBox::RejectRole:
        close();
        break;
    case QDialogButtonBox::ResetRole:
        ui->itemAttributes->onResetContents();
        break;
    }
}
void ItemAttributeEditor::setProperties( TibiaItem *item, const PropertyList& properties )
{
    if( ui->buttonSelected->isChecked() ) {
        PropertyList itemProperties = item->getProperties();
        for( PropertyList::const_iterator it = properties.begin(); it != properties.end(); it++ ) {
            if( !ui->buttonToggle->isChecked() ) {
                if( !item->hasHeader( (*it).header ) )
                    itemProperties.push_back( *it );
                else {
                    int index = itemProperties.indexOf( *it );
                    if( index != -1 )
                        itemProperties.replace( index, *it );
                }
            } else { // Toggle is enabled, remove if it has it, add if it doesnt
                if( !item->hasHeader( (*it).header ) )
                    itemProperties.push_back( *it );
                else
                    itemProperties.removeOne( *it );
            }
        }

        qSort( itemProperties );
        emit changeItem( g_tibiaHandler.getItemFile(), item, itemProperties );
    } else if( ui->buttonAll->isChecked() )
        emit changeItem( g_tibiaHandler.getItemFile(), item, properties );
}
