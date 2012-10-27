#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QEventLoop>
#include <QtGui/QProgressDialog>

#include "itemmodel.h"
#include "spritemodel.h"
#include "libraryfile.h"
#include "exportthread.h"

#include "exporthandler.h"
#include "formathandler.h"

extern TibiaHandler g_tibiaHandler;
extern FormatHandler g_formatHandler;

ExportHandler::ExportHandler( QWidget *parent ) : QDialog( parent ), ui( new Ui::ExportHandlerClass )
{
    itemModel = NULL;
    spriteModel = NULL;
    pictureModel = NULL;

    ui->setupUi( this );

    if( g_tibiaHandler.getItemFile() ) {
        itemModel = new ItemModel( this );
        ui->itemView->setModel( itemModel );
    }
    if( g_tibiaHandler.getSpriteFile() ) {
        spriteModel = new SpriteModel( this, RESOURCE_TYPE_SPRITE, g_tibiaHandler.getSpriteFile() );
        ui->spriteView->setModel( spriteModel );
    }
    if( g_tibiaHandler.getPictureFile() ) {
        pictureModel = new SpriteModel( this, RESOURCE_TYPE_PICTURE, g_tibiaHandler.getPictureFile() );
        ui->pictureView->setModel( pictureModel );
    }

    if( g_formatHandler.isLoaded() ) {
        for( DatFormatMap::const_iterator it = g_formatHandler.getFormatsBegin(); it != g_formatHandler.getFormatsEnd(); it++ )
            ui->comboVersions->insertItem( 0, it.value()->getName(), QVariant::fromValue( it.key() ) );
    } else
        ui->comboVersions->insertItem( 0, tr( "No Formats" ) );

    exportThread = new ExportThread( this );
    QObject::connect( exportThread, SIGNAL( started( ) ), this, SLOT( onExportStarted( ) ) );
    QObject::connect( exportThread, SIGNAL( success( int ) ), this, SLOT( onExportSuccess( int ) ) );
    QObject::connect( exportThread, SIGNAL( finished( ) ), this, SLOT( onExportComplete( ) ) );

    QObject::connect( ui->buttonExport, SIGNAL( clicked( bool ) ), this, SLOT( onExport( bool ) ) );
    QObject::connect( ui->buttonDirectory, SIGNAL( clicked( bool ) ), this, SLOT( onBrowse( bool ) ) );

    QObject::connect( ui->selectedItems, SIGNAL( toggled( bool ) ), this, SLOT( onSelectionChanged( bool ) ) );
    QObject::connect( ui->selectedOutfits, SIGNAL( toggled( bool ) ), this, SLOT( onSelectionChanged( bool ) ) );
    QObject::connect( ui->selectedEffects, SIGNAL( toggled( bool ) ), this, SLOT( onSelectionChanged( bool ) ) );
    QObject::connect( ui->selectedProjectiles, SIGNAL( toggled( bool ) ), this, SLOT( onSelectionChanged( bool ) ) );
    QObject::connect( ui->selectedPatterns, SIGNAL( toggled( bool ) ), this, SLOT( onPatternChanged( bool ) ) );
    QObject::connect( ui->parameterOutfitDirection, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onParameterChanged( int ) ) );
    QObject::connect( ui->parameterProjectileDirection, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onParameterChanged( int ) ) );
    QObject::connect( ui->parameterLayer, SIGNAL( valueChanged( int ) ), this, SLOT( onParameterChanged( int ) ) );
    QObject::connect( ui->parameterAnimation, SIGNAL( valueChanged( int ) ), this, SLOT( onParameterChanged( int ) ) );
    QObject::connect( ui->parameterAddons, SIGNAL( valueChanged( int ) ), this, SLOT( onParameterChanged( int ) ) );
}

ExportHandler::~ExportHandler( void )
{
    if( itemModel ) {
        itemModel->clear();
        delete itemModel;
    }
    if( spriteModel ) {
        spriteModel->clear();
        delete spriteModel;
    }
    if( pictureModel ) {
        pictureModel->clear();
        delete pictureModel;
    }

    if( exportThread ) {
        if( exportThread->isRunning() ) {
            exportThread->cancel();
            exportThread->terminate();
        }

        delete exportThread;
    }
    delete ui;
}

void ExportHandler::closeEvent( QCloseEvent *event )
{
    if( exportThread && exportThread->isRunning() ) {
        quint32 choice = QMessageBox::warning( this, tr( "Warning!" ), tr( "An export is in progress, do you want to cancel it?" ), QMessageBox::Yes | QMessageBox::No );
        switch( choice ) {
        case QMessageBox::Yes:
            exportThread->cancel();
            event->accept();
            break;
        case QMessageBox::No:
            event->ignore();
            break;
        }

        return;
    }

    event->accept();
}

void ExportHandler::setItems( const ItemList& list )
{
    items = list;
}

void ExportHandler::setOutfits( const ItemList& list )
{
    outfits = list;
}

void ExportHandler::setEffects( const ItemList& list )
{
    effects = list;
}

void ExportHandler::setProjectiles( const ItemList& list )
{
    projectiles = list;
}

void ExportHandler::updateItems( void )
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

void ExportHandler::updateSprites( void )
{
    if( spriteModel )
        spriteModel->setResourceList( sprites );
}

void ExportHandler::updatePictures( void )
{
    if( pictureModel )
        pictureModel->setResourceList( pictures );
}

void ExportHandler::setSprites( const ResourceList& list )
{
    sprites = list;
}

void ExportHandler::setPictures( const ResourceList& list )
{
    pictures = list;
}

void ExportHandler::setParameters( void )
{
    dParams.showPattern = ui->selectedPatterns->isChecked();
    dParams.addons = ui->parameterAddons->value();
    dParams.blendFrame = ui->parameterLayer->value();
    dParams.blendOverlay = ui->parameterLayer->value();
    dParams.animation = ui->parameterAnimation->value();
    dParams.outfitDirection = ui->parameterOutfitDirection->currentIndex();
    dParams.projectileDirection = ui->parameterProjectileDirection->currentIndex();

    if( itemModel )
        itemModel->setDrawParameters( dParams );
}

void ExportHandler::selectItems( bool value )
{
    ui->tabWidget->setCurrentIndex( 0 );
    ui->selectedItems->setChecked( value );
}

void ExportHandler::selectOutfits( bool value )
{
    ui->tabWidget->setCurrentIndex( 0 );
    ui->selectedOutfits->setChecked( value );
}

void ExportHandler::selectEffects( bool value )
{
    ui->tabWidget->setCurrentIndex( 0 );
    ui->selectedEffects->setChecked( value );
}

void ExportHandler::selectProjectiles( bool value )
{
    ui->tabWidget->setCurrentIndex( 0 );
    ui->selectedProjectiles->setChecked( value );
}

void ExportHandler::selectSprites( void )
{
    ui->tabWidget->setCurrentIndex( 1 );
}

void ExportHandler::selectPictures( void )
{
    ui->tabWidget->setCurrentIndex( 2 );
}

void ExportHandler::onSelectionChanged( bool )
{
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Selected Items changed." ) );
    updateItems( );
}

void ExportHandler::onPatternChanged( bool )
{
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Parameters changed." ) );
    setParameters( );
}

void ExportHandler::onParameterChanged( int )
{
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Parameters changed." ) );
    setParameters( );
}

void ExportHandler::onExport( bool )
{
    exportThread->setup();
    exportThread->setDirectory( ui->filePath->text() );
    switch( ui->tabWidget->currentIndex() ) {
    case 0: {
        switch( ui->comboFormatItems->currentIndex() ) {
        case 0:
            exportThread->setMode( EXPORT_ITEM );
            break;
        case 1:
            exportThread->setMode( EXPORT_LIBRARY );
            break;
        case 2:
            exportThread->setMode( EXPORT_PNG );
            break;
        case 3:
            exportThread->setMode( EXPORT_BMP );
            break;
        }
        if( ui->selectedItems->isChecked() )
            exportThread->setItems( items );
        if( ui->selectedOutfits->isChecked() )
            exportThread->setOutfits( outfits );
        if( ui->selectedEffects->isChecked() )
            exportThread->setEffects( effects );
        if( ui->selectedProjectiles->isChecked() )
            exportThread->setProjectiles( projectiles );
    }
    break;
    case 1: {
        switch( ui->comboFormatSprites->currentIndex() ) {
        case 0:
            exportThread->setMode( EXPORT_PNG );
            break;
        case 1:
            exportThread->setMode( EXPORT_BMP );
            break;
        }
        exportThread->setSprites( sprites );
    }
    break;
    case 2: {
        switch( ui->comboFormatPictures->currentIndex() ) {
        case 0:
            exportThread->setMode( EXPORT_PNG );
            break;
        case 1:
            exportThread->setMode( EXPORT_BMP );
            break;
        }
        exportThread->setPictures( pictures );
    }
    break;
    }
    exportThread->setDrawParameters( dParams );
    exportThread->setName( ui->filePath->text() );

    if( !items.isEmpty() || !outfits.isEmpty() || !effects.isEmpty() || !projectiles.isEmpty() || !sprites.isEmpty() || !pictures.isEmpty() ) {
        qint32 version = ui->comboVersions->itemData( ui->comboVersions->currentIndex(), Qt::UserRole ).toInt();
        DatFormat *datFormat = g_formatHandler.getFormatByClient( version );
        if( datFormat )
            exportThread->setFormat( datFormat->getVersion() );
        if( exportThread->getMode() != EXPORT_LIBRARY ) {
            QProgressDialog progressDialog( this, Qt::Tool );
            progressDialog.setWindowTitle( tr( "Export Progress" ) );
            progressDialog.setWindowIcon( windowIcon() );
            progressDialog.setAutoClose( false );
            exportThread->execute( &progressDialog );
        } else {
            QProgressDialog progressDialog( this, Qt::Tool );
            progressDialog.setWindowTitle( tr( "Export Progress" ) );
            progressDialog.setWindowIcon( windowIcon() );
            progressDialog.setAutoClose( false );
            LibraryFile *libraryFile = new LibraryFile( datFormat, items + outfits + effects + projectiles );
            QObject::connect( libraryFile->getThread(), SIGNAL( started( ) ), this, SLOT( onExportStarted( ) ) );
            QObject::connect( libraryFile->getThread(), SIGNAL( finished( ) ), this, SLOT( onExportComplete( ) ) );
            QObject::connect( libraryFile->getThread(), SIGNAL( success( int ) ), this, SLOT( onExportSuccess( int ) ) );
            libraryFile->getThread()->setup();
            libraryFile->getThread()->setName( ui->filePath->text() );
            libraryFile->getThread()->execute( &progressDialog );
        }
    }
}

void ExportHandler::onExportStarted( void )
{
    g_tibiaHandler.getChaseWidget()->addOperation( USER_OPERATION_EXPORT );
    ui->buttonExport->setEnabled( false );
}

void ExportHandler::onExportComplete( void )
{
    g_tibiaHandler.getChaseWidget()->removeOperation( USER_OPERATION_EXPORT );
    ui->buttonExport->setEnabled( true );
}

void ExportHandler::onExportSuccess( int elapsed )
{
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkCyan ), tr( "Export Completed: ( %1 )" ).arg( QTime().addMSecs( elapsed ).toString( tr( "hh:mm:ss.zzz" ) ) ) );
}

void ExportHandler::onBrowse( bool )
{
    if( ui->comboFormatItems->currentIndex() == 1 && ui->tabWidget->currentIndex() == 0 ) {
        QString filePath = QFileDialog::getSaveFileName ( this, tr( "Export Library" ), tr( "" ), tr( "Item Data Library File (*.idlf);;All Files (*)" ) );
        if( !filePath.isEmpty() )
            ui->filePath->setText( filePath );
    } else {
        QString filePath = QFileDialog::getExistingDirectory( this );
        if( !filePath.isEmpty() )
            ui->filePath->setText( filePath );
    }
}
