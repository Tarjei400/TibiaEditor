#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtGui/QFileDialog>
#include <QtCore/QMimeData>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>
#include <QtCore/qtconcurrentrun.h>

#include "itemmodel.h"
#include "spritemodel.h"

#include "tibiaeditor.h"

#include "tibiahandler.h"
#include "resourcehandler.h"
#include "importhandler.h"

#include "externalfile.h"
#include "libraryfile.h"

extern TibiaHandler g_tibiaHandler;
extern ResourceHandler g_resourceHandler;

ImportHandler::ImportHandler( QWidget *parent ) : QDialog( parent ), ui( new Ui::ImportHandlerClass )
{
    itemModel = NULL;
    spriteModel = NULL;
    pictureModel = NULL;

    ui->setupUi( this );

    destItems = NULL;
    destOutfits = NULL;
    destEffects = NULL;
    destProjectiles = NULL;

    QMenu *menu = new QMenu( ui->buttonDestination );
    QActionGroup *actionGroup = new QActionGroup( menu );
    actionGroup->setExclusive( true );
    destItems = actionGroup->addAction( tr( "Items" ) );
    destOutfits = actionGroup->addAction( tr( "Outfits" ) );
    destEffects = actionGroup->addAction( tr( "Effects" ) );
    destProjectiles = actionGroup->addAction( tr( "Projectiles" ) );
    destItems->setCheckable( true );
    destOutfits->setCheckable( true );
    destEffects->setCheckable( true );
    destProjectiles->setCheckable( true );
    menu->addAction( destItems );
    menu->addAction( destOutfits );
    menu->addAction( destEffects );
    menu->addAction( destProjectiles );
    ui->buttonDestination->setMenu( menu );

    actionDelete = new QAction( this );
    actionDelete->setObjectName( QString::fromUtf8( "actionDelete" ) );
    actionDelete->setEnabled( true );
    QIcon icon;
    icon.addPixmap( QPixmap( QString::fromUtf8( ":/TibiaEditor/Resources/minus.png" ) ), QIcon::Normal, QIcon::Off );
    actionDelete->setIcon( icon );
    actionDelete->setText( QApplication::translate("TibiaEditorClass", "Delete", 0, QApplication::UnicodeUTF8 ) );
    actionDelete->setShortcut( QApplication::translate("TibiaEditorClass", "Del", 0, QApplication::UnicodeUTF8 ) );
    addAction( actionDelete );

    ui->buttonBox->button( QDialogButtonBox::Open )->setText( tr( "Open..." ) );
    ui->buttonBox->button( QDialogButtonBox::Apply )->setText( tr( "Import" ) );
    ui->buttonBox->button( QDialogButtonBox::Reset )->setText( tr( "Clear" ) );


    if( g_tibiaHandler.getItemFile() ) {
        itemModel = new ItemModel( this );
        itemModel->setDropMimeFormats( QStringList() << "text/uri-list" );
        ui->itemObjectView->setModel( itemModel );

        QObject::connect( itemModel, SIGNAL( decodeMimeDrop( const QMimeData *, int, const QModelIndex& ) ), this, SLOT( onDropFiles( const QMimeData *, int, const QModelIndex& ) ) );
    }
    if( g_tibiaHandler.getSpriteFile() ) {
        spriteModel = new SpriteModel( this, RESOURCE_TYPE_SPRITE );
        spriteModel->setDropMimeFormats( QStringList() << "text/uri-list" );
        ui->spriteObjectView->setModel( spriteModel );

        QObject::connect( spriteModel, SIGNAL( decodeMimeDrop( const QMimeData *, int, const QModelIndex& ) ), this, SLOT( onDropFiles( const QMimeData *, int, const QModelIndex& ) ) );
    }
    if( g_tibiaHandler.getPictureFile() ) {
        pictureModel = new SpriteModel( this, RESOURCE_TYPE_PICTURE );
        pictureModel->setDropMimeFormats( QStringList() << "text/uri-list" );
        ui->pictureObjectView->setModel( pictureModel );

        QObject::connect( pictureModel, SIGNAL( decodeMimeDrop( const QMimeData *, int, const QModelIndex& ) ), this, SLOT( onDropFiles( const QMimeData *, int, const QModelIndex& ) ) );
    }

    importThread = new ImportThread( this );
    QObject::connect( importThread, SIGNAL( started( ) ), this, SLOT( onImportStarted( ) ) );
    QObject::connect( importThread, SIGNAL( success( int ) ), this, SLOT( onImportSuccess( int ) ) );
    QObject::connect( importThread, SIGNAL( finished( ) ), this, SLOT( onImportComplete( ) ) );

    dropThread = new DropFileThread( this );
    QObject::connect( dropThread, SIGNAL( started( ) ), this, SLOT( onImportStarted( ) ) );
    QObject::connect( dropThread, SIGNAL( success( int ) ), this, SLOT( onDropSuccess( int ) ) );
    QObject::connect( dropThread, SIGNAL( finished( ) ), this, SLOT( onImportComplete( ) ) );
    QObject::connect( dropThread, SIGNAL( dropItems( ItemList ) ), this, SLOT( onDropItems( ItemList ) ) );
    QObject::connect( dropThread, SIGNAL( dropSprites( quint8, ResourceList ) ), this, SLOT( onDropSprites( quint8, ResourceList ) ) );
    QObject::connect( dropThread, SIGNAL( dropTibiaFile( QFile * ) ), this, SLOT( onDropTibiaFile( QFile * ) ) );
    QObject::connect( dropThread, SIGNAL( documentError( QString, QString, int ) ), g_tibiaHandler.getOutputWidget(), SLOT( onDocumentError( QString, QString, int ) ) );

    //QObject::connect( ui->itemObjectView, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( onDoubleClickedItem( const QModelIndex & ) ) );
    QObject::connect( actionDelete, SIGNAL( triggered() ), this, SLOT( onDelete() ) );
    QObject::connect( ui->buttonBox, SIGNAL( clicked( QAbstractButton * ) ), this, SLOT( onButtonClicked( QAbstractButton * ) ) );
    QObject::connect( ui->itemObjectView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTreeViewContextMenu( const QPoint& ) ) );
    QObject::connect( ui->spriteObjectView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTreeViewContextMenu( const QPoint& ) ) );
    QObject::connect( ui->pictureObjectView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTreeViewContextMenu( const QPoint& ) ) );
}

ImportHandler::~ImportHandler( void )
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
    if( importThread ) {
        if( importThread->isRunning() )
            importThread->cancel();

        delete importThread;
    }
    if( dropThread ) {
        if( dropThread->isRunning() )
            dropThread->cancel();

        delete dropThread;
    }

    foreach( QFile* file, fileList ) {
        g_resourceHandler.removeResourcesByFile( RESOURCE_TYPE_SPRITE, file, false, true );
        g_resourceHandler.removeResourcesByFile( RESOURCE_TYPE_PICTURE, file, false, true );
    }

    delete ui;
}

void ImportHandler::closeEvent( QCloseEvent *event )
{
    if( importThread && importThread->isRunning() ) {
        switch( QMessageBox::warning( this, tr( "Warning!" ), tr( "An import is in progress, do you want to cancel it?" ), QMessageBox::Yes | QMessageBox::No ) ) {
        case QMessageBox::Yes:
            importThread->cancel();
            event->accept();
            break;
        case QMessageBox::No:
            event->ignore();
            break;
        default:
            break;
        }
        return;
    }

    if( dropThread && dropThread->isRunning() ) {
        switch( QMessageBox::warning( this, tr( "Warning!" ), tr( "A drop is in progress, do you want to cancel it?" ), QMessageBox::Yes | QMessageBox::No ) ) {
        case QMessageBox::Yes:
            dropThread->cancel();
            event->accept();
            break;
        case QMessageBox::No:
            event->ignore();
            break;
        default:
            break;
        }
        return;
    }

    event->accept();
}

void ImportHandler::selectItems( void )
{
    ui->tabWidget->setCurrentIndex( 0 );
    destItems->setChecked( true );
}

void ImportHandler::selectOutfits( void )
{
    ui->tabWidget->setCurrentIndex( 0 );
    destOutfits->setChecked( true );
}

void ImportHandler::selectEffects( void )
{
    ui->tabWidget->setCurrentIndex( 0 );
    destEffects->setChecked( true );
}

void ImportHandler::selectProjectiles( void )
{
    ui->tabWidget->setCurrentIndex( 0 );
    destProjectiles->setChecked( true );
}

void ImportHandler::selectSprites( void )
{
    ui->tabWidget->setCurrentIndex( 1 );
}

void ImportHandler::selectPictures( void )
{
    ui->tabWidget->setCurrentIndex( 2 );
}

void ImportHandler::onDropFiles( const QMimeData *data, int row, const QModelIndex& parent )
{
    Q_UNUSED( row );
    Q_UNUSED( parent );

    if( data->hasUrls() ) {
        DropMode_t dropMode;
        switch( ui->tabWidget->currentIndex() ) {
        case TAB_INDEX_ITEMS: // Items
            dropMode = DROP_ITEMS;
            break;
        case TAB_INDEX_SPRITES: // Sprites
            dropMode = DROP_SPRITES;
            break;
        case TAB_INDEX_PICTURES: // Pictures
            dropMode = DROP_PICTURES;
            break;
        }

        dropFiles( QStringList(), data->urls(), dropMode );
    }
}

void ImportHandler::onTreeViewContextMenu( const QPoint& point )
{
    Q_UNUSED( point );
    QTreeView *treeView = qobject_cast<QTreeView *>( sender() );
    if( treeView->selectionModel() && treeView->selectionModel()->hasSelection() ) {
        QPoint realPos = QCursor::pos();
        QMenu menu;
        menu.addAction( actionDelete );
        menu.exec( realPos );
    }
}

void ImportHandler::onDelete( void )
{
    switch( ui->tabWidget->currentIndex() ) {
    case TAB_INDEX_ITEMS: { // Items
        ItemList items;
        if( itemModel )
            items = TibiaEditor::itemSelection( ui->itemObjectView->selectionModel() );

        if( !items.isEmpty() ) {
            foreach( TibiaItem* item, items ) {
                itemModel->removeItem( -1, item );
                delete item;
            }
        }
    }
    break;
    case TAB_INDEX_SPRITES: { // Sprites
        ResourceList resources;
        if( spriteModel )
            resources = TibiaEditor::spriteSelection( ui->spriteObjectView->selectionModel() );

        if( !resources.isEmpty() ) {
            foreach( SharedResource resource, resources )
            spriteModel->removeSprite( RESOURCE_TYPE_SPRITE, spriteModel->getIndexByResource( resource ) );
        }
    }
    break;
    case TAB_INDEX_PICTURES: { // Pictures
        ResourceList resources;
        if( spriteModel )
            resources = TibiaEditor::spriteSelection( ui->pictureObjectView->selectionModel() );

        if( !resources.isEmpty() ) {
            foreach( SharedResource resource, resources )
            pictureModel->removeSprite( RESOURCE_TYPE_PICTURE, pictureModel->getIndexByResource( resource ) );
        }
    }
    break;
    }
}

void ImportHandler::onButtonClicked( QAbstractButton *button )
{
    switch( ui->buttonBox->buttonRole( button ) ) {
    case QDialogButtonBox::ApplyRole: { // Import
        switch( ui->tabWidget->currentIndex() ) {
        case TAB_INDEX_ITEMS: { // Items
            if( itemModel ) {
                if( destItems->isChecked() ) {
                    if( !itemModel->getItemList().isEmpty() ) {
                        importItems( itemModel->getItemList(), ITEM_PARENT_ITEMS );
                        return;
                    }
                } else if( destOutfits->isChecked() ) {
                    if( !itemModel->getItemList().isEmpty() ) {
                        importItems( itemModel->getItemList(), ITEM_PARENT_OUTFITS );
                        return;
                    }
                } else if( destEffects->isChecked() ) {
                    if( !itemModel->getItemList().isEmpty() ) {
                        importItems( itemModel->getItemList(), ITEM_PARENT_EFFECTS );
                        return;
                    }
                } else if( destProjectiles->isChecked() ) {
                    if( !itemModel->getItemList().isEmpty() ) {
                        importItems( itemModel->getItemList(), ITEM_PARENT_PROJECTILES );
                        return;
                    }
                }
            }
        }
        break;

        case TAB_INDEX_SPRITES: { // Sprites
            if( spriteModel ) {
                if( !spriteModel->getResourceList().isEmpty() ) {
                    importSprites( spriteModel->getResourceList(), RESOURCE_TYPE_SPRITE, g_tibiaHandler.getSpriteFile()->getCount() );
                    return;
                }
            }
        }
        break;

        case TAB_INDEX_PICTURES: { // Pictures
            if( pictureModel ) {
                if( !pictureModel->getResourceList().isEmpty() ) {
                    importSprites( pictureModel->getResourceList(), RESOURCE_TYPE_PICTURE, g_tibiaHandler.getPictureFile()->getCount() );
                    return;
                }
            }
        }
        break;
        }
    }
    break;

    case QDialogButtonBox::AcceptRole: { // Open
        switch( ui->tabWidget->currentIndex() ) {
        case TAB_INDEX_ITEMS: // Items
            dropFiles( QFileDialog::getOpenFileNames ( this, tr( "Select appropriate files." ), QString(), tr( "Supported Files (*.png *.bmp *.idf *.idfl);;Image Files (*.png *.bmp);;Item Data Files (*.idf);;Item Data Library Files (*.idlf);;All Files (*)" ) ), QUrlList(), DROP_ITEMS );
            break;
        case TAB_INDEX_SPRITES: // Sprites
            dropFiles( QFileDialog::getOpenFileNames ( this, tr( "Select appropriate files." ), QString(), tr( "Supported Files (*.png *.bmp);;Portable Network Graphics File (*.png);;Bitmap (*.bmp);;All Files (*)" ) ), QUrlList(), DROP_SPRITES );
            break;
        case TAB_INDEX_PICTURES: // Pictures
            dropFiles( QFileDialog::getOpenFileNames ( this, tr( "Select appropriate files." ), QString(), tr( "Supported Files (*.png *.bmp);;Portable Network Graphics File (*.png);;Bitmap (*.bmp);;All Files (*)" ) ), QUrlList(), DROP_PICTURES );
            break;
        }
    }
    break;

    case QDialogButtonBox::ResetRole: { // Clear
        switch( ui->tabWidget->currentIndex() ) {
        case TAB_INDEX_ITEMS: { // Items
            if( itemModel ) {
                itemModel->deleteAll();

                foreach( QFile* file, fileList ) {
                    g_resourceHandler.removeResourcesByFile( RESOURCE_TYPE_SPRITE, file, false, true );
                }
            }
        }
        break;

        case TAB_INDEX_SPRITES: { // Sprites
            if( spriteModel )
                spriteModel->clear();
        }
        break;

        case TAB_INDEX_PICTURES: { // Pictures
            if( pictureModel )
                pictureModel->clear();
        }
        break;
        }
    }
    default:
        break;
        break;
    }
}

void ImportHandler::importItems( ItemList& items, qint8 importDestination )
{
    if( !items.isEmpty() ) {
        importThread->setup();
        importThread->setDestination( importDestination );
        importThread->setItemList( items );
        importThread->setFilter( true );
        importThread->setResourceType( RESOURCE_TYPE_SPRITE );
        if( SpriteFile *spriteFile = g_tibiaHandler.getSpriteFile() ) {
            importThread->setMode( IMPORT_ITEMS_AND_SPRITES );
            importThread->setIndex( spriteFile->getCount() );
        } else
            importThread->setMode( IMPORT_ITEMS );

        QProgressDialog progressDialog( this, Qt::Tool );
        progressDialog.setWindowIcon( windowIcon() );
        progressDialog.setAutoClose( false );
        progressDialog.setWindowTitle( tr( "Import Progress" ) );
        importThread->execute( &progressDialog );
    }
}

void ImportHandler::importSprites( const ResourceList& resources, quint8 resourceType, quint32 count )
{
    if( !resources.isEmpty() ) {
        importThread->setup();
        importThread->setIndex( count );
        importThread->setResourceType( resourceType );
        importThread->setSpriteList( resources );

        switch( resourceType ) {
        case RESOURCE_TYPE_SPRITE:
            importThread->setMode( IMPORT_SPRITES );
            break;
        case RESOURCE_TYPE_PICTURE:
            importThread->setMode( IMPORT_PICTURES );
            break;
        }

        QProgressDialog progressDialog( this, Qt::Tool );
        progressDialog.setWindowIcon( windowIcon() );
        progressDialog.setAutoClose( false );
        progressDialog.setWindowTitle( tr( "Import Progress" ) );
        importThread->execute( &progressDialog );
    }
}

void ImportHandler::onImportStarted( void )
{
    g_tibiaHandler.getChaseWidget()->addOperation( USER_OPERATION_IMPORT );
}

void ImportHandler::onImportComplete( void )
{
    g_tibiaHandler.getChaseWidget()->removeOperation( USER_OPERATION_IMPORT );
}

void ImportHandler::onImportSuccess( int elapsed )
{
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkCyan ), tr( "Import Completed: ( %1 )" ).arg( QTime().addMSecs( elapsed ).toString( tr( "hh:mm:ss.zzz" ) ) ) );
    if( importThread->getMode() == IMPORT_ITEMS || importThread->getMode() == IMPORT_ITEMS_AND_SPRITES ) {
        if( itemModel )
            itemModel->clear();
    } else if( importThread->getMode() == IMPORT_SPRITES ) {
        if( spriteModel )
            spriteModel->clear();
    } else if( importThread->getMode() == IMPORT_PICTURES ) {
        if( pictureModel )
            pictureModel->clear();
    }
}

void ImportHandler::dropFiles( const QStringList& strings, const QUrlList& urls, DropMode_t dropMode )
{
    dropThread->setup();
    dropThread->setDropMode( dropMode );
    dropThread->setStringList( strings );
    dropThread->setUrlList( urls );

    QProgressDialog progressDialog( this, Qt::Tool );
    progressDialog.setWindowIcon( windowIcon() );
    progressDialog.setAutoClose( false );
    progressDialog.setWindowTitle( tr( "File Progress" ) );
    dropThread->execute( &progressDialog );
}

void ImportHandler::onDropSuccess( int elapsed )
{
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkCyan ), tr( "Drop Completed: ( %1 )" ).arg( QTime().addMSecs( elapsed ).toString( tr( "hh:mm:ss.zzz" ) ) ) );
}

void ImportHandler::onDropTibiaFile( QFile *file )
{
    fileList.push_back( file );
}

void ImportHandler::onDropItems( ItemList items )
{
    if( itemModel )
        itemModel->addItemList( items );
}

void ImportHandler::onDropSprites( quint8 type, ResourceList resources )
{
    switch( type ) {
    case RESOURCE_TYPE_SPRITE: {
        if( spriteModel )
            spriteModel->addSprites( type, resources );
    }
    break;
    case RESOURCE_TYPE_PICTURE: {
        if( pictureModel )
            pictureModel->addSprites( type, resources );
    }
    break;
    }
}
