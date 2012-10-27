#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>
#include <QtGui/QItemSelectionModel>

#include "QtCore/qtconcurrentrun.h"

#include "itemsearch.h"
#include "tibiahandler.h"
#include "formathandler.h"
#include "itemmodel.h"

extern TibiaHandler g_tibiaHandler;
extern FormatHandler g_formatHandler;

ItemSearch::ItemSearch( QWidget *parent ) : QDialog( parent ), ui( new Ui::ItemSearchClass )
{
    ui->setupUi( this );
    ui->checkFilters->setChecked( false );
    ui->checkCombinations->setChecked( false );

    itemModel = new ItemModel( this );
    ui->itemView->setModel( itemModel );


    if( g_formatHandler.isLoaded() ) {
        for( DatFormatMap::const_iterator it = g_formatHandler.getFormatsBegin(); it != g_formatHandler.getFormatsEnd(); it++ )
            ui->comboVersions->insertItem( 0, it.value()->getName(), QVariant::fromValue( it.key() ) );
    } else
        ui->comboVersions->insertItem( 0, tr( "No Formats" ) );

    QObject::connect( &searchWatcher, SIGNAL( finished() ), this, SLOT( onSearchComplete() ) );

    toggleAdvanced();
}

ItemSearch::~ItemSearch( void )
{
    if( itemModel ) {
        itemModel->clear();
        delete itemModel;
    }
    delete ui;
}

void ItemSearch::closeEvent( QCloseEvent *event )
{
    if( searchWatcher.isRunning() ) {
        QMessageBox::critical( this, tr( "Warning!" ), tr( "A search is in progress and cannot be canceled." ) );
        event->ignore();
        return;
    }

    event->accept();
}

void ItemSearch::on_checkAdvanced_stateChanged( int )
{
    toggleAdvanced();
}

void ItemSearch::toggleAdvanced( void )
{
    ui->checkAdvanced->isChecked() ? ui->itemAttributes->showValues( true ) : ui->itemAttributes->showValues( false );
}

void ItemSearch::on_comboVersions_currentIndexChanged( int index )
{
    qint32 version = ui->comboVersions->itemData( index, Qt::UserRole ).toInt();
    DatFormat *datFormat = g_formatHandler.getFormatByClient( version );
    if( datFormat ) {
        ui->itemAttributes->setFormat( datFormat );
    }
}

quint32 ItemSearch::colorDominance( const QImage& image, quint8 colorIntensity ) const
{
    quint16 colorConcentration[6];
    for( quint8 i = 0; i < 6; i++ )
        colorConcentration[i] = 0;

    quint32 colorGrid[6];
    colorGrid[0] = qRgb( 0xFF, 0x00, 0x00 ); // Red
    colorGrid[1] = qRgb( 0x00, 0xFF, 0x00 ); // Green
    colorGrid[2] = qRgb( 0x00, 0x00, 0xFF ); // Blue
    colorGrid[3] = qRgb( 0xFF, 0xFF, 0x00 ); // Yellow
    colorGrid[4] = qRgb( 0xFF, 0x00, 0xFF ); // Magenta
    colorGrid[5] = qRgb( 0x00, 0xFF, 0xFF ); // Cyan

    for ( qint32 x = 0; x < image.width(); x++ ) {
        for ( qint32 y = 0; y < image.height(); y++ ) {
            QRgb pixel = image.pixel( x, y );
            quint8 Red = qRed( pixel );
            quint8 Green = qGreen( pixel );
            quint8 Blue = qBlue( pixel );
            quint8 Alpha = qAlpha( pixel );

            if( !( Red == 0xFF && Green == 0x00 && Blue == 0xFF ) && Alpha != 0x00 ) {
                if( Red > Green && Red > Blue && ( Red - Green >= colorIntensity ) && ( Red - Blue >= colorIntensity ) )
                    colorConcentration[0]++;
                if( Green > Red && Green > Blue && ( Green - Red >= colorIntensity ) && ( Green - Blue >= colorIntensity ) )
                    colorConcentration[1]++;
                if( Blue > Green && Blue > Red && ( Blue - Green >= colorIntensity ) && ( Blue - Red >= colorIntensity ) )
                    colorConcentration[2]++;
                if( Red > Blue && Green > Blue && ( Red - Blue >= colorIntensity ) && ( Green - Blue >= colorIntensity ) )
                    colorConcentration[3]++;
                if( Red > Green && Blue > Green && ( Red - Green >= colorIntensity ) && ( Blue - Green >= colorIntensity ) )
                    colorConcentration[4]++;
                if( Green > Red && Blue > Red && ( Green - Red >= colorIntensity ) && ( Blue - Red >= colorIntensity ) )
                    colorConcentration[5]++;
            }
        }
    }

    quint16 highest = 0;
    quint8 index = 0;
    for( quint8 i = 0; i < 6; i++ ) {
        if( colorConcentration[i] > highest ) {
            highest = colorConcentration[i];
            index = i;
        }
    }

    if( highest > 0 ) // Color with highest intensity/density
        return colorGrid[index];

    return qRgb( 0x00, 0x00, 0x00 );
}

void ItemSearch::on_buttonFind_clicked()
{
    HeaderList headerSearch;
    ui->itemAttributes->getHeaders( headerSearch );

    PropertyList propertySearch;
    ui->itemAttributes->getAttributes( propertySearch );

    ItemFile *itemFile = g_tibiaHandler.getItemFile();
    if( itemFile ) {
        results.clear();
        searchWatcher.setFuture( QtConcurrent::run( this, &ItemSearch::performSearch, headerSearch, propertySearch, itemFile ) );
        g_tibiaHandler.getChaseWidget()->addOperation( USER_OPERATION_SEARCH );
        time.start();
    }
}

void ItemSearch::on_buttonSelect_clicked()
{
    ItemList selectedItems = itemSelection( ui->itemView->selectionModel() );
    if( !selectedItems.isEmpty() ) {
        ItemList items, outfits, effects, projectiles;
        foreach( TibiaItem* item, selectedItems ) {
            switch( item->getParentType() ) {
            case ITEM_PARENT_ITEMS:
                items.push_back( item );
                break;
            case ITEM_PARENT_OUTFITS:
                outfits.push_back( item );
                break;
            case ITEM_PARENT_EFFECTS:
                effects.push_back( item );
                break;
            case ITEM_PARENT_PROJECTILES:
                projectiles.push_back( item );
                break;
            }
        }

        emit selectItems( items, outfits, effects, projectiles );
    }
}

ItemList ItemSearch::itemSelection( QItemSelectionModel *selectionModel )
{
    ItemList items;
    if( selectionModel ) {
        QModelIndexList indexes = selectionModel->selectedIndexes();
        if( !indexes.isEmpty() ) {
            foreach( QModelIndex index, indexes ) {
                if( index.isValid() ) {
                    TibiaItem *item = index.data( Qt::UserRole ).value<TibiaItem *>();
                    if( item )
                        items.push_back( item );
                }
            }
        }
    }

    return items;
}

void ItemSearch::onSearchComplete( void )
{
    if( itemModel )
        itemModel->setItemList( results );

    g_tibiaHandler.getChaseWidget()->removeOperation( USER_OPERATION_SEARCH );
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkCyan ), tr( "Search Completed: ( %1 )" ).arg( QTime().addMSecs( time.elapsed() ).toString( tr( "hh:mm:ss.zzz" ) ) ) );
}

void ItemSearch::performSearch( const HeaderList& headerSearch, const PropertyList& propertySearch, ItemFile *itemFile )
{
    if( !ui->checkFilters->isChecked() || !ui->filterItems->isChecked() ) {
        foreach( TibiaItem* item, itemFile->getItems() ) {
            if( verifyItem( headerSearch, propertySearch, item ) )
                results.push_back( item );
        }
    }
    if( !ui->checkFilters->isChecked() || !ui->filterOutfits->isChecked() ) {
        foreach( TibiaItem* item, itemFile->getOutfits() ) {
            if( verifyItem( headerSearch, propertySearch, item ) )
                results.push_back( item );
        }
    }
    if( !ui->checkFilters->isChecked() || !ui->filterEffects->isChecked() ) {
        foreach( TibiaItem* item, itemFile->getEffects() ) {
            if( verifyItem( headerSearch, propertySearch, item ) )
                results.push_back( item );
        }
    }
    if( !ui->checkFilters->isChecked() || !ui->filterProjectiles->isChecked() ) {
        foreach( TibiaItem* item, itemFile->getProjectiles() ) {
            if( verifyItem( headerSearch, propertySearch, item ) )
                results.push_back( item );
        }
    }
}

bool ItemSearch::verifyItem( const HeaderList& headerSearch, const PropertyList& propertySearch, const TibiaItem *item ) const
{
    if( ui->tabAttributeTypes->currentIndex() == 0 || ( ui->checkCombinations->isChecked() && ui->combineAttributes->isChecked() ) ) {
        if( ui->checkAdvanced->isChecked() && ( !item->contains( headerSearch ) || !item->contains( propertySearch ) ) )
            return false;
        if( !ui->checkAdvanced->isChecked() && !item->contains( headerSearch ) )
            return false;
    }
    if( ui->tabAttributeTypes->currentIndex() == 1 || ( ui->checkCombinations->isChecked() && ui->combineValues->isChecked() ) ) {
        if( ui->buttonID->isChecked() ) {
            if( g_tibiaHandler.getItemFile()->getItemById( ( quint32 )ui->valueID->value() ) != item )
                return false;
        } else if( ui->buttonProperty->isChecked() ) {
            if( ui->checkWidth->isChecked() && ui->valueWidth->value() != item->getWidth() )
                return false;
            if( ui->checkHeight->isChecked() && ui->valueHeight->value() != item->getHeight() )
                return false;
            if( ui->checkCropsize->isChecked() && ui->valueCropsize->value() != item->getCropsize() )
                return false;
            if( ui->checkLayers->isChecked() && ui->valueLayers->value() != item->getLayers() )
                return false;
            if( ui->checkXDiv->isChecked() && ui->valueXDiv->value() != item->getXDiv() )
                return false;
            if( ui->checkYDiv->isChecked() && ui->valueYDiv->value() != item->getYDiv() )
                return false;
            if( ui->checkZDiv->isChecked() && ui->valueZDiv->value() != item->getZDiv() )
                return false;
            if( ui->checkAnimations->isChecked() && ui->valueAnimations->value() != item->getAnimation() )
                return false;
        }
    }
    if( ui->tabAttributeTypes->currentIndex() == 2 || ( ui->checkCombinations->isChecked() && ui->combineColors->isChecked() ) ) {
        quint32 dominantColor = colorDominance( g_tibiaHandler.drawItem( item ), ui->spinIntensity->value() );
        if( ui->radioColorRed->isChecked() && dominantColor != qRgb( 0xFF, 0x00, 0x00 ) )
            return false;
        if( ui->radioColorGreen->isChecked() && dominantColor != qRgb( 0x00, 0xFF, 0x00 ) )
            return false;
        if( ui->radioColorBlue->isChecked() && dominantColor != qRgb( 0x00, 0x00, 0xFF ) )
            return false;
        if( ui->radioColorYellow->isChecked() && dominantColor != qRgb( 0xFF, 0xFF, 0x00 ) )
            return false;
        if( ui->radioColorMagenta->isChecked() && dominantColor != qRgb( 0xFF, 0x00, 0xFF ) )
            return false;
        if( ui->radioColorCyan->isChecked() && dominantColor != qRgb( 0x00, 0xFF, 0xFF ) )
            return false;
    }

    return true;
}
