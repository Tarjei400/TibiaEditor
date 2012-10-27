#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtGui/QPixmap>
#include <QtGui/QMessageBox>
#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtGui/QProgressDialog>

#include "mimeconstants.h"
#include "QtCore/qtconcurrentrun.h"

#include "tibiahandler.h"
#include "resourcehandler.h"
#include "formathandler.h"

#include "tibiaeditor.h"
#include "spritefile.h"
#include "itemmodel.h"

#include "fileinformation.h"
#include "importhandler.h"
#include "exporthandler.h"
#include "itemsearch.h"
#include "itemattributeeditor.h"
#include "itemeditor.h"

#include "chasewidget.h"

extern TibiaHandler g_tibiaHandler;
extern ResourceHandler g_resourceHandler;
extern FormatHandler g_formatHandler;

Q_DECLARE_METATYPE( ResourceList )
Q_DECLARE_METATYPE( ItemList )

TibiaEditor::TibiaEditor( QWidget *parent, Qt::WFlags flags ) : QMainWindow( parent, flags ), ui( new Ui::TibiaEditorClass )
{
    itemModel = NULL;
    outfitModel = NULL;
    effectModel = NULL;
    projectileModel = NULL;
    spriteModel = NULL;
    pictureModel = NULL;

    ui->setupUi( this );

    ui->actionFileNew->setMenu( ui->menuNew );
    ui->actionFileOpen->setMenu( ui->menuOpen );
    ui->actionFileCompile->setMenu( ui->menuCompile );

    setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

    undoGroup = new QUndoGroup;
    itemUndoStack = new QUndoStack;
    spriteUndoStack = new QUndoStack;
    pictureUndoStack = new QUndoStack;
    undoGroup->addStack( itemUndoStack );
    undoGroup->addStack( spriteUndoStack );
    undoGroup->addStack( pictureUndoStack );
    //undoGroup->setActiveStack( itemUndoStack );
    ui->historyView->setGroup( undoGroup );

    fillWidget = new QWidget( ui->progressToolBar );
    fillWidget->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::DefaultType ) );
    ui->progressToolBar->addWidget( fillWidget );
    ui->progressToolBar->addWidget( g_tibiaHandler.getChaseWidget() );

    outputDock = new QDockWidget( tr( "Output" ), this );
    outputDock->setWidget( g_tibiaHandler.getOutputWidget() );
    g_tibiaHandler.getOutputWidget()->setContextMenuPolicy( Qt::ActionsContextMenu );
    g_tibiaHandler.getOutputWidget()->addAction( ui->actionEditCopy );
    g_tibiaHandler.getOutputWidget()->addAction( ui->actionFileExport );
    g_tibiaHandler.getOutputWidget()->addAction( ui->actionEditSelectAll );
    addDockWidget( static_cast<Qt::DockWidgetArea>(2), outputDock );

    ui->menuView->addAction( ui->fileToolBar->toggleViewAction() );
    ui->menuView->addAction( ui->editToolBar->toggleViewAction() );
    ui->menuView->addAction( ui->editToolToolBar->toggleViewAction() );
    ui->menuView->addAction( ui->progressToolBar->toggleViewAction() );
    ui->menuView->addSeparator();
    ui->menuView->addAction( ui->objectDock->toggleViewAction() );
    ui->menuView->addAction( ui->itemDock->toggleViewAction() );
    ui->menuView->addAction( ui->outfitDock->toggleViewAction() );
    ui->menuView->addAction( ui->effectDock->toggleViewAction() );
    ui->menuView->addAction( ui->projectileDock->toggleViewAction() );
    ui->menuView->addSeparator();
    ui->menuView->addAction( ui->spriteDock->toggleViewAction() );
    ui->menuView->addAction( ui->pictureDock->toggleViewAction() );
    ui->menuView->addSeparator();
    ui->menuView->addAction( ui->historyDock->toggleViewAction() );
    ui->menuView->addSeparator();
    ui->menuView->addAction( outputDock->toggleViewAction() );

    /*splitDockWidget ( ui->objectDock, ui->itemDock, Qt::Horizontal );
    tabifyDockWidget( ui->itemDock, ui->projectileDock );
    tabifyDockWidget( ui->itemDock, ui->effectDock );
    tabifyDockWidget( ui->itemDock, ui->outfitDock );
    ui->itemDock->raise();*/

    // Hook undo signals
    QObject::connect( ui->actionEditUndo, SIGNAL( triggered() ), undoGroup, SLOT( undo() ) );
    QObject::connect( ui->actionEditRedo, SIGNAL( triggered() ), undoGroup, SLOT( redo() ) );

    // Hook undo signals to actions
    QObject::connect( undoGroup, SIGNAL( canUndoChanged( bool ) ), ui->actionEditUndo, SLOT( setEnabled( bool ) ) );
    QObject::connect( undoGroup, SIGNAL( canRedoChanged( bool ) ), ui->actionEditRedo, SLOT( setEnabled( bool ) ) );

    // Hook context menus
    QObject::connect( ui->spriteView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTreeViewContextMenu( const QPoint& ) ) );
    QObject::connect( ui->itemView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTreeViewContextMenu( const QPoint& ) ) );
    QObject::connect( ui->outfitView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTreeViewContextMenu( const QPoint& ) ) );
    QObject::connect( ui->effectView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTreeViewContextMenu( const QPoint& ) ) );
    QObject::connect( ui->projectileView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTreeViewContextMenu( const QPoint& ) ) );
    QObject::connect( ui->pictureView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTreeViewContextMenu( const QPoint& ) ) );

    // Hook error displays
    QObject::connect( &g_tibiaHandler, SIGNAL( parseError( QString, QFile::FileError ) ), this, SLOT( onFileError( QString, QFile::FileError ) ) );
    QObject::connect( &g_tibiaHandler, SIGNAL( documentError( QString, QString, int ) ), this, SLOT( onDocumentError( QString, QString, int ) ) );

    // Hook loaders and unloaders
    QObject::connect( &g_tibiaHandler, SIGNAL( itemsLoaded( ItemFile * ) ), this, SLOT( onLoadItemFile( ItemFile * ) ) );
    QObject::connect( &g_tibiaHandler, SIGNAL( spritesLoaded( SpriteFile * ) ), this, SLOT( onLoadSpriteFile( SpriteFile * ) ) );
    QObject::connect( &g_tibiaHandler, SIGNAL( picturesLoaded( PictureFile * ) ), this, SLOT( onLoadPictureFile( PictureFile * ) ) );
    QObject::connect( &g_tibiaHandler, SIGNAL( itemsUnloaded( void ) ), this, SLOT( onUnloadItemFile( void ) ) );
    QObject::connect( &g_tibiaHandler, SIGNAL( spritesUnloaded( void ) ), this, SLOT( onUnloadSpriteFile( void ) ) );
    QObject::connect( &g_tibiaHandler, SIGNAL( picturesUnloaded( void ) ), this, SLOT( onUnloadPictureFile( void ) ) );
    QObject::connect( &g_tibiaHandler, SIGNAL( invalidateItems( void ) ), this, SLOT( onInvalidateItems( void ) ) );

    g_formatHandler.loadFile( QCoreApplication::applicationDirPath() + "/formats.xml" );
}

TibiaEditor::~TibiaEditor( void )
{
    delete fillWidget;
    delete outputDock;
    delete itemUndoStack;
    delete spriteUndoStack;
    delete pictureUndoStack;
    delete undoGroup;
    delete ui;
}

void TibiaEditor::closeEvent( QCloseEvent *event )
{
    foreach( QMdiSubWindow* subWindow, ui->mdiArea->subWindowList() ) {
        if( !subWindow->close() ) {
            event->ignore();
            return;
        }
    }

    event->accept();
}

void TibiaEditor::setSelection( QAbstractItemView *itemView, const ItemList& items )
{
    QItemSelectionModel *selectionModel = itemView->selectionModel();
    ItemModel *model = qobject_cast<ItemModel *>( itemView->model() );
    if( selectionModel && model ) {
        QItemSelection selection;
        foreach( TibiaItem* item, items ) {
            QModelIndex index = model->indexByItem( item );
            selection.select( index, index );
        }

        selectionModel->select( selection, QItemSelectionModel::Select );
        itemView->scrollTo( selection.last().bottomRight() );
    }
}

ItemList TibiaEditor::itemSelection( QItemSelectionModel *selectionModel )
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

ResourceList TibiaEditor::spriteSelection( QItemSelectionModel *selectionModel )
{
    ResourceList sprites;
    if( selectionModel ) {
        QModelIndexList indexes = selectionModel->selectedIndexes();
        if( !indexes.isEmpty() ) {
            foreach( QModelIndex index, indexes ) {
                if( index.isValid() )
                    sprites.push_back( index.data( Qt::UserRole ).value<SharedResource>() );
            }
        }
    }

    return sprites;
}

bool TibiaEditor::isFileOpened( const QWidget *widget ) const
{
    const QAbstractItemView *itemView = qobject_cast<const QAbstractItemView *>( widget );
    if( itemView && widget == focusWidget() ) {
        if( isItemView( itemView ) )
            return ( g_tibiaHandler.getItemFile() && g_tibiaHandler.getItemFile()->isLoaded() );
        else if( itemView == ui->spriteView )
            return ( g_tibiaHandler.getSpriteFile() && g_tibiaHandler.getSpriteFile()->isLoaded() );
        else if( itemView == ui->pictureView )
            return ( g_tibiaHandler.getPictureFile() && g_tibiaHandler.getPictureFile()->isLoaded() );
    }

    return false;
}

QPair<bool, bool> TibiaEditor::isOpenedClosed( const QWidget *widget )
{
    QPair<bool, bool> OpenClose;
    const QAbstractItemView *itemView = qobject_cast<const QAbstractItemView *>( widget );
    if( itemView && widget == focusWidget() ) {
        quint32 openCount = 0;
        quint32 closedCount = 0;
        if( isItemView( itemView ) ) {
            QList<ItemEditor *> openEditors = itemEditors();
            ItemList items = itemSelection( itemView->selectionModel() );
            foreach( TibiaItem* item, items ) {
                bool isOpen = false;
                foreach( ItemEditor* itemEditor, openEditors ) {
                    if( itemEditor->getTibiaItem() == item ) {
                        openCount++;
                        isOpen = true;
                    }

                }
                if( !isOpen )
                    closedCount++;
            }

            // If more items are closed than open, or there are no items open and there can be items open, or there are as many closed as there are open - Allow open
            // If there are more open than closed, and they can be closed, or there are as many closed as there are open - Allow close
            OpenClose.first = ( closedCount > openCount || ( items.size() > 0 && openEditors.size() == 0 ) || ( closedCount > 0 && closedCount == openCount ) );
            OpenClose.second = ( ( openCount > closedCount && openEditors.size() > 0 ) || ( openCount > 0 && closedCount == openCount ) );
        } else if( itemView == ui->spriteView ) {
            ResourceList sprites = spriteSelection( itemView->selectionModel() );
            OpenClose.first = false;
            OpenClose.second = false;
        } else if( itemView == ui->pictureView ) {
            ResourceList pictures = spriteSelection( itemView->selectionModel() );
            OpenClose.first = false;
            OpenClose.second = false;
        }
    }

    return OpenClose;
}

QList<ItemEditor *> TibiaEditor::itemEditors( void ) const
{
    QList<ItemEditor *> openEditors;
    QList<QMdiSubWindow *> subWindows = ui->mdiArea->subWindowList();
    if( subWindows.isEmpty() )
        return openEditors;

    for ( int i = 0; i < subWindows.size(); ++i ) {
        QMdiSubWindow *subWindow = subWindows.at( i );
        if( ItemEditor *itemEditor = qobject_cast<ItemEditor *>( subWindow ) )
            openEditors.push_back( itemEditor );
    }

    return openEditors;
}


void TibiaEditor::onFileError( QString preset, QFile::FileError error )
{
    QString errorString;
    switch( error ) {
    case QFile::ReadError:
        errorString = tr( "An error occurred when reading from the file." );
        break;
    case QFile::WriteError:
        errorString = tr( "An error occurred when writing to the file." );
        break;
    case QFile::FatalError:
        errorString = tr( "A fatal error occurred." );
        break;
    case QFile::ResourceError:
        errorString = tr( "A resource error occured." );
        break;
    case QFile::OpenError:
        errorString = tr( "The file could not be opened." );
        break;
    case QFile::AbortError:
        errorString = tr( "The operation was aborted." );
        break;
    case QFile::TimeOutError:
        errorString = tr( "A timeout occurred." );
        break;
    case QFile::UnspecifiedError:
        errorString = tr( "An unspecified error occurred." );
        break;
    case QFile::RemoveError:
        errorString = tr( "The file could not be removed." );
        break;
    case QFile::RenameError:
        errorString = tr( "The file could not be renamed." );
        break;
    case QFile::PositionError:
        errorString = tr( "The position in the file could not be changed." );
        break;
    case QFile::ResizeError:
        errorString = tr( "The file could not be resized." );
        break;
    case QFile::PermissionsError:
        errorString = tr( "The file could not be accessed." );
        break;
    case QFile::CopyError:
        errorString = tr( "The file could not be copied." );
        break;
    }

    QMessageBox::critical( this, preset, errorString );
}

void TibiaEditor::onDocumentError( QString fileName, QString error, int line )
{
    QString errorString;
    errorString.append( fileName + "\n\n" + tr( "Error: %1" ).arg( error ) );
    if( line != -1 )
        errorString.append( tr( "\nLine: %1" ).arg( line ) );

    QMessageBox::critical( this, tr( "Document Error" ), errorString );
}

/*void TibiaEditor::onDocumentError( QString fileName, QString error, QString details )
{
    QMessageBox messageBox( this );
    messageBox.setWindowTitle( tr( "Document Error" ) );
    messageBox.addButton( QMessageBox::Ok );
    messageBox.setIcon( QMessageBox::Critical );
    messageBox.setText( fileName );
    messageBox.setInformativeText( QString( error ) );
    if( !details.isEmpty() )
        messageBox.setDetailedText( details );
    messageBox.exec();
}*/

void TibiaEditor::onFileNewDat( void )
{
    if( g_tibiaHandler.getItemFile() && g_tibiaHandler.getItemFile()->isLoaded() ) {
        switch( QMessageBox::question( this, tr( "New Dat..." ), tr( "Another Dat file is already open, do you want to close it?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ) {
        case QMessageBox::Yes:
            g_tibiaHandler.unloadItemFile();
            break;
        default:
            return;
            break;
        }
    }
    if( g_tibiaHandler.loadItemFile( QString(), NULL, true, true ) )
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Dat Created." ) );
}

void TibiaEditor::onFileNewSpr( void )
{
    if( g_tibiaHandler.getSpriteFile() && g_tibiaHandler.getSpriteFile()->isLoaded() ) {
        switch( QMessageBox::question( this, tr( "New Spr..." ), tr( "Another Spr file is already open, do you want to close it?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ) {
        case QMessageBox::Yes:
            g_tibiaHandler.unloadSpriteFile();
            break;
        default:
            return;
            break;
        }
    }
    if( g_tibiaHandler.loadSpriteFile( QString(), true, true ) )
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Spr Created." ) );
}

void TibiaEditor::onFileNewPic( void )
{
    if( g_tibiaHandler.getPictureFile() && g_tibiaHandler.getPictureFile()->isLoaded() ) {
        switch( QMessageBox::question( this, tr( "New Pic..." ), tr( "Another Pic file is already open, do you want to close it?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ) {
        case QMessageBox::Yes:
            g_tibiaHandler.unloadPictureFile();
            break;
        default:
            return;
            break;
        }
    }
    if( g_tibiaHandler.loadPictureFile( QString(), true, true ) )
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Pic Created." ) );
}

void TibiaEditor::onFileNewItem( void )
{
    // Open item wizard
}

void TibiaEditor::onFileOpenDat( void )
{
    QStringList filters;
    if( g_formatHandler.isLoaded() ) {
        for( DatFormatMap::const_iterator it = g_formatHandler.getFormatsBegin(); it != g_formatHandler.getFormatsEnd(); it++ )
            filters.push_front( tr( "%1 (*.dat)" ).arg( it.value()->getName() ) );
        filters.push_front( tr( "Auto Detect (*.dat)" ) );
    }

    // Filepath config here
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName ( this, tr( "Open Dat..." ), tr(""), filters.join( ";;" ), &selectedFilter );

    if( !fileName.isEmpty() ) {
        if( g_tibiaHandler.getItemFile() && g_tibiaHandler.getItemFile()->isLoaded() ) {
            switch( QMessageBox::question( this, tr( "Open Dat..." ), tr( "Another Dat file is already open, do you want to close it?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ) {
            case QMessageBox::Yes:
                g_tibiaHandler.unloadItemFile();
                break;
            default:
                return;
                break;
            }
        }

        DatFormat *datFormat = NULL;
        if( selectedFilter.compare( filters.first(), Qt::CaseInsensitive ) == 0 )
            datFormat = g_formatHandler.getFormatBySignature( TibiaFile::peekSignature( fileName ) );
        else
            datFormat = g_formatHandler.getFormatByName( selectedFilter.remove( tr( " (*.dat)" ) ) );

        if( g_tibiaHandler.loadItemFile( fileName, datFormat, true ) )
            g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Dat Loaded." ) );
    }
}

void TibiaEditor::onFileOpenSpr( void )
{
    // Filepath config here
    QString fileName = QFileDialog::getOpenFileName ( this, tr( "Open Spr..." ), tr(""), tr( "Sprite File (*.spr);;All Files (*)" ) );
    if( !fileName.isEmpty() ) {
        if( g_tibiaHandler.getSpriteFile() && g_tibiaHandler.getSpriteFile()->isLoaded() ) {
            switch( QMessageBox::question( this, tr( "Open Spr..." ), tr( "Another Spr file is already open, do you want to close it?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ) {
            case QMessageBox::Yes:
                g_tibiaHandler.unloadSpriteFile();
                break;
            default:
                return;
                break;
            }
        }
        if( g_tibiaHandler.loadSpriteFile( fileName, true ) )
            g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Spr Loaded." ) );
    }
}

void TibiaEditor::onFileOpenPic( void )
{
    // Filepath config here
    QString fileName = QFileDialog::getOpenFileName ( this, tr( "Open Pic..." ), tr(""), tr( "Picture File (*.pic);;All Files (*)" ) );
    if( !fileName.isEmpty() ) {
        if( g_tibiaHandler.getPictureFile() && g_tibiaHandler.getPictureFile()->isLoaded() ) {
            switch( QMessageBox::question( this, tr( "Open Pic..." ), tr( "Another Pic file is already open, do you want to close it?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ) {
            case QMessageBox::Yes:
                g_tibiaHandler.unloadPictureFile();
                break;
            default:
                return;
                break;
            }
        }
        if( g_tibiaHandler.loadPictureFile( fileName, true ) )
            g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Pic Loaded." ) );
    }
}

void TibiaEditor::onFileOpenItem( void )
{
    // Open item
}

void TibiaEditor::onFileCompileDat( void )
{
    ItemFile *itemFile = g_tibiaHandler.getItemFile();
    if( !itemFile ) {
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::red ), tr( "No Dat loaded." ) );
        return;
    }

    QString currentFilter = tr( "%1 (*.dat)" ).arg( itemFile->getFormat() != g_formatHandler.getBaseFormat() ? itemFile->getFormat()->getName() : g_formatHandler.getBaseFormat()->getName() );

    QStringList filters;
    if( g_formatHandler.isLoaded() ) {
        for( DatFormatMap::const_iterator it = g_formatHandler.getFormatsBegin(); it != g_formatHandler.getFormatsEnd(); it++ )
            filters.push_front( tr( "%1 (*.dat)" ).arg( it.value()->getName() ) );
        filters.push_front( currentFilter );
    }

    // Filepath config here
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName ( this, tr( "Save Dat..." ), tr(""), filters.join( ";;" ), &selectedFilter );
    if( fileName.isEmpty() )
        return;

    DatFormat *datFormat = NULL;
    if( selectedFilter.compare( filters.first(), Qt::CaseInsensitive ) == 0 )
        datFormat = itemFile->getFormat();
    else
        datFormat = g_formatHandler.getFormatByName( selectedFilter.remove( tr( " (*.dat)" ) ) );

    if( datFormat == g_formatHandler.getBaseFormat() ) {
        switch( QMessageBox::question( this, tr( "Save Dat..." ), tr( "You are saving an item file with the base format, are you sure you want to continue?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes ) ) {
        case QMessageBox::Yes: {
            QObject::connect( itemFile->getThread(), SIGNAL( started( ) ), this, SLOT( onSaveStarted( ) ), Qt::UniqueConnection );
            QObject::connect( itemFile->getThread(), SIGNAL( finished( ) ), this, SLOT( onSaveComplete( ) ), Qt::UniqueConnection );
            QObject::connect( itemFile->getThread(), SIGNAL( success( int ) ), this, SLOT( onSaveSuccess( int ) ), Qt::UniqueConnection );
            QObject::connect( itemFile->getThread(), SIGNAL( parseError( QString, QFile::FileError ) ), this, SLOT( onFileError( QString, QFile::FileError ) ), Qt::UniqueConnection );
            QObject::connect( itemFile->getThread(), SIGNAL( documentError( QString, QString, int ) ), this, SLOT( onDocumentError( QString, QString, int ) ), Qt::UniqueConnection );
            QProgressDialog progressDialog( this, Qt::Tool );
            progressDialog.setWindowIcon( windowIcon() );
            progressDialog.setAutoClose( false );
            progressDialog.setWindowTitle( tr( "Compile Progress" ) );
            itemFile->getThread()->setup();
            itemFile->getThread()->setName( fileName );
            itemFile->getThread()->execute( &progressDialog );
            return;
        }
        break;
        default:
            return;
            break;
        }
    }

    if( datFormat ) {
        itemFile->setFormat( datFormat );
        QObject::connect( itemFile->getThread(), SIGNAL( started( ) ), this, SLOT( onSaveStarted( ) ), Qt::UniqueConnection );
        QObject::connect( itemFile->getThread(), SIGNAL( finished( ) ), this, SLOT( onSaveComplete( ) ), Qt::UniqueConnection );
        QObject::connect( itemFile->getThread(), SIGNAL( success( int ) ), this, SLOT( onSaveSuccess( int ) ), Qt::UniqueConnection );
        QObject::connect( itemFile->getThread(), SIGNAL( parseError( QString, QFile::FileError ) ), this, SLOT( onFileError( QString, QFile::FileError ) ), Qt::UniqueConnection );
        QObject::connect( itemFile->getThread(), SIGNAL( documentError( QString, QString, int ) ), this, SLOT( onDocumentError( QString, QString, int ) ), Qt::UniqueConnection );
        QProgressDialog progressDialog( this, Qt::Tool );
        progressDialog.setWindowIcon( windowIcon() );
        progressDialog.setAutoClose( false );
        progressDialog.setWindowTitle( tr( "Compile Progress" ) );
        itemFile->getThread()->setup();
        itemFile->getThread()->setName( fileName );
        itemFile->getThread()->execute( &progressDialog );
    } else
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::red ), tr( "Invalid format." ) );
}

void TibiaEditor::onFileCompileSpr( void )
{
    SpriteFile *spriteFile = g_tibiaHandler.getSpriteFile();
    if( !spriteFile ) {
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::red ), tr( "No Spr loaded." ) );
        return;
    }

    // Filepath config here
    QString fileName = QFileDialog::getSaveFileName ( this, tr( "Save Spr..." ), tr(""), tr( "Sprite File (*.spr);;All Files (*)" ) );
    if( !fileName.isEmpty() ) {
        QObject::connect( spriteFile->getThread(), SIGNAL( started( ) ), this, SLOT( onSaveStarted( ) ), Qt::UniqueConnection );
        QObject::connect( spriteFile->getThread(), SIGNAL( finished( ) ), this, SLOT( onSaveComplete( ) ), Qt::UniqueConnection );
        QObject::connect( spriteFile->getThread(), SIGNAL( success( int ) ), this, SLOT( onSaveSuccess( int ) ), Qt::UniqueConnection );
        QObject::connect( spriteFile->getThread(), SIGNAL( parseError( QString, QFile::FileError ) ), this, SLOT( onFileError( QString, QFile::FileError ) ), Qt::UniqueConnection );
        QObject::connect( spriteFile->getThread(), SIGNAL( documentError( QString, QString, int ) ), this, SLOT( onDocumentError( QString, QString, int ) ), Qt::UniqueConnection );
        QProgressDialog progressDialog( this, Qt::Tool );
        progressDialog.setWindowIcon( windowIcon() );
        progressDialog.setAutoClose( false );
        progressDialog.setWindowTitle( tr( "Compile Progress" ) );
        spriteFile->getThread()->setup();
        spriteFile->getThread()->setName( fileName );
        spriteFile->getThread()->execute( &progressDialog );
    }
}

void TibiaEditor::onFileCompilePic( void )
{
    PictureFile *pictureFile = g_tibiaHandler.getPictureFile();
    if( !pictureFile ) {
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::red ), tr( "No Pic loaded." ) );
        return;
    }

    // Filepath config here
    QString fileName = QFileDialog::getSaveFileName ( this, tr( "Save Pic..." ), tr(""), tr( "Picture File (*.pic);;All Files (*)" ) );
    if( !fileName.isEmpty() ) {
        QObject::connect( pictureFile->getThread(), SIGNAL( started( ) ), this, SLOT( onSaveStarted( ) ), Qt::UniqueConnection );
        QObject::connect( pictureFile->getThread(), SIGNAL( finished( ) ), this, SLOT( onSaveComplete( ) ), Qt::UniqueConnection );
        QObject::connect( pictureFile->getThread(), SIGNAL( success( int ) ), this, SLOT( onSaveSuccess( int ) ), Qt::UniqueConnection );
        QObject::connect( pictureFile->getThread(), SIGNAL( parseError( QString, QFile::FileError ) ), this, SLOT( onFileError( QString, QFile::FileError ) ), Qt::UniqueConnection );
        QObject::connect( pictureFile->getThread(), SIGNAL( documentError( QString, QString, int ) ), this, SLOT( onDocumentError( QString, QString, int ) ), Qt::UniqueConnection );
        QProgressDialog progressDialog( this, Qt::Tool );
        progressDialog.setWindowIcon( windowIcon() );
        progressDialog.setAutoClose( false );
        progressDialog.setWindowTitle( tr( "Compile Progress" ) );
        pictureFile->getThread()->setup();
        pictureFile->getThread()->setName( fileName );
        pictureFile->getThread()->execute( &progressDialog );
    }
}

void TibiaEditor::onFileCompileItem( void )
{
    // Save Item
}

void TibiaEditor::onSaveStarted( void )
{
    g_tibiaHandler.getChaseWidget()->addOperation( USER_OPERATION_COMPILE );
}

void TibiaEditor::onSaveComplete( void )
{
    g_tibiaHandler.getChaseWidget()->removeOperation( USER_OPERATION_COMPILE );
}

void TibiaEditor::onSaveSuccess( int elapsed )
{
    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkCyan ), tr( "Compile Completed: ( %1 )" ).arg( QTime().addMSecs( elapsed ).toString( tr( "hh:mm:ss.zzz" ) ) ) );
}

void TibiaEditor::onFileImport( void )
{
    ImportHandler importHandler( this );

    QObject::connect( importHandler.getThread(), SIGNAL( importSprites( quint8, quint32, ResourceList ) ), this, SLOT( onImportSprites( quint8, quint32, ResourceList ) ) );
    QObject::connect( importHandler.getThread(), SIGNAL( importItems( quint8, ItemList ) ), this, SLOT( onImportItems( quint8, ItemList ) ) );

    QAction *action = qobject_cast<QAction *>( sender() );
    if( action ) {
        importHandler.setWindowIcon( action->icon() );
        importHandler.setWindowTitle( action->text() );
    }

    // Check current widget
    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( this->focusWidget() );
    if( itemView ) {
        if( itemView == ui->itemView )
            importHandler.selectItems();
        else if( itemView == ui->outfitView )
            importHandler.selectOutfits();
        else if( itemView == ui->effectView )
            importHandler.selectEffects();
        else if( itemView == ui->projectileView )
            importHandler.selectProjectiles();
        else if( itemView == ui->spriteView )
            importHandler.selectSprites();
        else if( itemView == ui->pictureView )
            importHandler.selectPictures();
    }

    importHandler.exec();
}

void TibiaEditor::onFileExport( void )
{
    OutputWidget *outputWidget = qobject_cast<OutputWidget *>( this->focusWidget() );
    if( outputWidget ) {
        QString filePath = QFileDialog::getSaveFileName ( this, tr( "Export Output" ), tr( "" ), tr( "Hyper Text Markup Language (*.html);;All Files (*)" ) );
        if( !filePath.isEmpty() ) {
            QFile file( filePath );
            file.open( QIODevice::WriteOnly );
            file.write( outputWidget->toHtml().toUtf8() );
            file.close();
        }
    } else {
        ExportHandler exportHandler( this );
        QAction *action = qobject_cast<QAction *>( sender() );
        if( action ) {
            exportHandler.setWindowIcon( action->icon() );
            exportHandler.setWindowTitle( action->text() );
        }

        exportHandler.setItems( itemSelection( ui->itemView->selectionModel() ) );
        exportHandler.setOutfits( itemSelection( ui->outfitView->selectionModel() ) );
        exportHandler.setEffects( itemSelection( ui->effectView->selectionModel() ) );
        exportHandler.setProjectiles( itemSelection( ui->projectileView->selectionModel() ) );
        exportHandler.setParameters();
        exportHandler.updateItems();
        exportHandler.setSprites( spriteSelection( ui->spriteView->selectionModel() ) );
        exportHandler.updateSprites();
        exportHandler.setPictures( spriteSelection( ui->pictureView->selectionModel() ) );
        exportHandler.updatePictures();

        // Check current widget
        QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( this->focusWidget() );
        if( itemView ) {
            if( itemView == ui->itemView )
                exportHandler.selectItems( true );
            else if( itemView == ui->outfitView )
                exportHandler.selectOutfits( true );
            else if( itemView == ui->effectView )
                exportHandler.selectEffects( true );
            else if( itemView == ui->projectileView )
                exportHandler.selectProjectiles( true );
            else if( itemView == ui->spriteView )
                exportHandler.selectSprites();
            else if( itemView == ui->pictureView )
                exportHandler.selectPictures();
        }

        exportHandler.exec();
    }
}


void TibiaEditor::onEditInsert( void )
{
    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( this->focusWidget() );
    if( itemView ) {
        ItemFile *itemFile = g_tibiaHandler.getItemFile();
        if( itemFile ) {
            QModelIndex index = itemView->currentIndex();
            if( index.isValid() ) {
                TibiaItem *reference = index.data( Qt::UserRole ).value<TibiaItem *>();
                if( reference ) {
                    itemUndoStack->push( new CommandRelocateItem( itemFile, NULL, reference, ITEM_ACTION_INSERT ) );
                    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkGreen ), tr( "Item Added" ) );
                }
            } else {
                quint8 parent = ITEM_PARENT_INTERNAL;
                if( itemView == ui->itemView )
                    parent = ITEM_PARENT_ITEMS;
                else if( itemView == ui->outfitView )
                    parent = ITEM_PARENT_OUTFITS;
                else if( itemView == ui->effectView )
                    parent = ITEM_PARENT_EFFECTS;
                else if( itemView == ui->projectileView )
                    parent = ITEM_PARENT_PROJECTILES;

                if( parent != ITEM_PARENT_INTERNAL ) {
                    itemUndoStack->push( new CommandRelocateItem( itemFile, NULL, NULL, ITEM_ACTION_INSERT, parent ) );
                    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkGreen ), tr( "Item Added" ) );
                }
            }
        }
    }
}

void TibiaEditor::onEditDelete( void )
{
    ItemFile *itemFile = g_tibiaHandler.getItemFile();
    if( itemFile ) {
        QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( this->focusWidget() );
        if( itemView ) {
            if( isItemView( itemView ) ) { // Multiple items being deleted
                ItemList items = itemSelection( itemView->selectionModel() );
                if( items.size() > 1 )
                    itemUndoStack->push( new CommandRemoveItems( itemFile, items ) );
                else if( items.size() == 1 ) // Only one item deleted
                    itemUndoStack->push( new CommandRemoveItem( itemFile, items.first() ) );

                if( items.size() >= 1 )
                    g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkGreen ), tr( "%n Item(s) Removed", 0, items.size() ) );
            }
            /*else if( itemView == ui->spriteView )
                // Nothing
            else if( itemView == ui->pictureView )
                // Nothing Yet
            */
        }
    }
}

void TibiaEditor::onEditCut( void )
{
    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( this->focusWidget() );
    if( isItemView( itemView ) ) {
        ItemFile *itemFile = g_tibiaHandler.getItemFile();
        if( itemFile ) {
            ItemList items = itemSelection( itemView->selectionModel() );
            itemUndoStack->push( new CommandCutItems( itemFile, items ) );

            QClipboard *clipboard = QApplication::clipboard();
            if( clipboard ) {
                QMimeData *mimeData = new QMimeData();
                QByteArray encodedData;
                QDataStream stream( &encodedData, QIODevice::WriteOnly );
                stream << (quint32)items.size();
                foreach( TibiaItem* item, items )
                stream << *item;
                mimeData->setData( QString( CLIPBOARD_ITEMS ), encodedData );
                clipboard->setMimeData( mimeData );
            }

            onUpdateActions( itemView );
            g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkGreen ), tr( "Cut %n Item(s)", 0, items.size() ) );
        }
    }
}

void TibiaEditor::onEditCopy( void )
{
    OutputWidget *outputWidget = qobject_cast<OutputWidget *>( this->focusWidget() );
    if( outputWidget )
        outputWidget->copy();

    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( this->focusWidget() );
    if( isItemView( itemView ) ) {
        ItemFile *itemFile = g_tibiaHandler.getItemFile();
        if( itemFile ) {
            ItemList items = itemSelection( itemView->selectionModel() );
            if( items.size() > 0 ) {
                QClipboard *clipboard = QApplication::clipboard();
                if( clipboard ) {
                    QMimeData *mimeData = new QMimeData();
                    QByteArray encodedData;
                    QDataStream stream( &encodedData, QIODevice::WriteOnly );
                    stream << (quint32)items.size();
                    foreach( TibiaItem* item, items )
                    stream << *item;
                    mimeData->setData( QString( CLIPBOARD_ITEMS ), encodedData );
                    clipboard->setMimeData( mimeData );
                }

                onUpdateActions( itemView );
                g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkGreen ), tr( "Copied %n Item(s)", 0, items.size() ) );
            }
        }
    }
}

void TibiaEditor::onEditPaste( void )
{
    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( this->focusWidget() );
    if( isItemView( itemView ) ) {
        ItemFile *itemFile = g_tibiaHandler.getItemFile();
        if( itemFile ) {
            ItemList items;
            QClipboard *clipboard = QApplication::clipboard();

            if( clipboard && clipboard->mimeData() && clipboard->mimeData()->hasFormat( CLIPBOARD_ITEMS ) ) {
                QByteArray encodedData = clipboard->mimeData()->data( CLIPBOARD_ITEMS );
                QDataStream stream( &encodedData, QIODevice::ReadOnly );

                quint32 size;
                stream >> size;
                for( quint32 i = 0; i < size; i++ ) {
                    TibiaItem tibiaItem;
                    stream >> tibiaItem;
                    if( tibiaItem.isValid() ) {
                        TibiaItem *newItem = new TibiaItem( tibiaItem );
                        items.push_back( newItem );
                    }
                }

                QModelIndex index = itemView->currentIndex();
                if( index.isValid() ) {
                    TibiaItem *reference = index.data( Qt::UserRole ).value<TibiaItem *>();
                    if( reference ) {
                        if( items.size() > 1 )
                            itemUndoStack->push( new CommandRelocateItems( itemFile, items, reference, ITEM_ACTION_PASTE ) );
                        else if( items.size() == 1 )
                            itemUndoStack->push( new CommandRelocateItem( itemFile, items.first(), reference, ITEM_ACTION_PASTE ) );
                        if( items.size() >= 1 )
                            g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkGreen ), tr( "Pasted %n Item(s)", 0, items.size() ) );
                    }
                } else {
                    quint8 parent = ITEM_PARENT_INTERNAL;
                    if( itemView == ui->itemView )
                        parent = ITEM_PARENT_ITEMS;
                    else if( itemView == ui->outfitView )
                        parent = ITEM_PARENT_OUTFITS;
                    else if( itemView == ui->effectView )
                        parent = ITEM_PARENT_EFFECTS;
                    else if( itemView == ui->projectileView )
                        parent = ITEM_PARENT_PROJECTILES;

                    if( parent != ITEM_PARENT_INTERNAL ) {
                        if( items.size() > 1 )
                            itemUndoStack->push( new CommandRelocateItems( itemFile, items, NULL, ITEM_ACTION_PASTE, parent ) );
                        else if( items.size() == 1 )
                            itemUndoStack->push( new CommandRelocateItem( itemFile, items.first(), NULL, ITEM_ACTION_PASTE, parent ) );

                        if( items.size() >= 1 )
                            g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkGreen ), tr( "Pasted %n Item(s)", 0, items.size() ) );
                    }
                }
            }
        }
    }
}

void TibiaEditor::onEditSelectAll( void )
{
    OutputWidget *outputWidget = qobject_cast<OutputWidget *>( focusWidget() );
    if( outputWidget )
        outputWidget->selectAll();

    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( focusWidget() );
    if( itemView ) {
        itemView->selectAll();
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "All objects selected." ) );
    }
}

void TibiaEditor::onEditDeselect( void )
{
    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( focusWidget() );
    if( itemView ) {
        itemView->clearSelection();
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkBlue ), tr( "Selection cleared." ) );
    }
}

void TibiaEditor::onEditProperties( void )
{
    if( ( ( g_tibiaHandler.getItemFile() && g_tibiaHandler.getItemFile()->isLoaded() ) ||
            ( g_tibiaHandler.getSpriteFile() && g_tibiaHandler.getSpriteFile()->isLoaded() ) ||
            ( g_tibiaHandler.getPictureFile() && g_tibiaHandler.getPictureFile()->isLoaded() ) ) ) {
        FileInformation fileInfo( this );
        QAction *action = qobject_cast<QAction *>( sender() );
        if( action ) {
            fileInfo.setWindowIcon( action->icon() );
            fileInfo.setWindowTitle( action->text() );
        }
        fileInfo.exec();
    } else
        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::red ), tr( "No primary files have been loaded." ) );
}

void TibiaEditor::onEditItemProperties( void )
{
    ItemAttributeEditor attributeEditor( this );
    QObject::connect( &attributeEditor, SIGNAL( changeItem( ItemFile *, TibiaItem *, PropertyList ) ), this, SLOT( onChangeItem( ItemFile *, TibiaItem *, PropertyList ) ) );
    QAction *action = qobject_cast<QAction *>( sender() );
    if( action ) {
        attributeEditor.setWindowIcon( action->icon() );
        attributeEditor.setWindowTitle( action->text() );
    }
    attributeEditor.setItems( itemSelection( ui->itemView->selectionModel() ) );
    attributeEditor.setOutfits( itemSelection( ui->outfitView->selectionModel() ) );
    attributeEditor.setEffects( itemSelection( ui->effectView->selectionModel() ) );
    attributeEditor.setProjectiles( itemSelection( ui->projectileView->selectionModel() ) );
    attributeEditor.updateItems();
    attributeEditor.exec();
}

void TibiaEditor::onEditOpenItem( void )
{
    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( this->focusWidget() );
    if( !itemView )
        return;

    if( isItemView( itemView ) ) {
        QList<ItemEditor *> openEditors = itemEditors();
        ItemList items = itemSelection( itemView->selectionModel() );
        foreach( TibiaItem* item, items ) {
            bool openItem = true;
            if( openEditors.size() > 0 ) {
                foreach( ItemEditor* itemEditor, openEditors ) {
                    if( itemEditor->getTibiaItem() == item ) {
                        ui->mdiArea->setActiveSubWindow( itemEditor );
                        openItem = false;
                    }
                }
            }

            if( openItem )
                onOpenItem( item, item->getItemData() );
        }
    } else if( itemView == ui->spriteView ) {
        //
    } else if( itemView == ui->pictureView ) {
        //
    }

    onUpdateActions( itemView );
}

void TibiaEditor::onEditCloseItem( void )
{
    QAbstractItemView *itemView = qobject_cast<QAbstractItemView *>( this->focusWidget() );
    if( !itemView ) {
        if( ItemEditor *itemEditor = qobject_cast<ItemEditor *>( ui->mdiArea->activeSubWindow() ) )
            itemEditor->close();
        return;
    }

    if( isItemView( itemView ) ) {
        QList<ItemEditor *> openEditors = itemEditors();
        if( openEditors.size() == 0 )
            return;

        ItemList items = itemSelection( itemView->selectionModel() );
        foreach( TibiaItem* item, items ) {
            foreach( ItemEditor* itemEditor, openEditors ) {
                if( itemEditor->getTibiaItem() == item )
                    itemEditor->close();
            }
        }
    }

    onUpdateActions( itemView );
}

void TibiaEditor::onEditFind( void )
{
    ItemSearch itemSearch( this );
    QObject::connect( &itemSearch, SIGNAL( selectItems( const ItemList&, const ItemList&, const ItemList&, const ItemList& ) ), this, SLOT( onSelectItems( const ItemList&, const ItemList&, const ItemList&, const ItemList& ) ) );
    QAction *action = qobject_cast<QAction *>( sender() );
    if( action ) {
        itemSearch.setWindowIcon( action->icon() );
        itemSearch.setWindowTitle( action->text() );
    }
    itemSearch.exec();
}

void TibiaEditor::onFullScreen( bool toggle )
{
    if( toggle )
        setWindowState( windowState() ^ Qt::WindowFullScreen );
    else
        setWindowState( windowState() & ~Qt::WindowFullScreen );
}

void TibiaEditor::onTreeViewContextMenu( const QPoint& point )
{
    Q_UNUSED( point );

    QPoint realPos = QCursor::pos();
    QMenu menu;
    menu.addAction( ui->actionEditOpenItem );
    menu.addAction( ui->actionEditCloseItem );
    menu.addSeparator();
    menu.addAction( ui->actionFileImport );
    menu.addAction( ui->actionFileExport );
    menu.addSeparator();
    menu.addAction( ui->actionEditCut );
    menu.addAction( ui->actionEditCopy );
    menu.addAction( ui->actionEditPaste );
    menu.addSeparator();
    menu.addAction( ui->actionEditSelectAll );
    menu.addAction( ui->actionEditDeselect );
    menu.addSeparator();
    menu.addAction( ui->actionEditInsert );
    menu.addAction( ui->actionEditDelete );
    if( qobject_cast<QAbstractItemView *>( sender() ) == ui->spriteView ) {
        menu.addSeparator();
        menu.addAction( ui->actionToolsItemGenerator );
    }
    menu.exec( realPos );
}

void TibiaEditor::onUpdateActions( const QWidget *widget )
{
    if( widget == focusWidget() ) {
        const OutputWidget *outputWidget = qobject_cast<const OutputWidget *>( widget );
        if( outputWidget ) {
            undoGroup->setActiveStack( NULL );
            ui->actionEditOpenItem->setEnabled( false );
            ui->actionEditCloseItem->setEnabled( false );
            ui->actionEditCut->setEnabled( false );
            ui->actionEditCopy->setEnabled( true );
            ui->actionEditDelete->setEnabled( false );
            ui->actionEditInsert->setEnabled( false );
            ui->actionEditPaste->setEnabled( false );
            return;
        }

        const QUndoView *undoView = qobject_cast<const QUndoView *>( widget );
        if( undoView ) {
            ui->actionEditOpenItem->setEnabled( false );
            ui->actionEditCloseItem->setEnabled( false );
            ui->actionEditCut->setEnabled( false );
            ui->actionEditCopy->setEnabled( false );
            ui->actionEditDelete->setEnabled( false );
            ui->actionEditInsert->setEnabled( false );
            ui->actionEditPaste->setEnabled( false );
            return;
        }

        //const QMdiSubWindow* subWindow = qobject_cast<const QMdiSubWindow*>( widget );
        /*if( const ItemEditor* itemEditor = qobject_cast<const ItemEditor*>( widget ) )
        {
            undoGroup->setActiveStack( itemEditor->getUndoStack() );
            ui->actionEditOpenItem->setEnabled( true );
            ui->actionEditCloseItem->setEnabled( true );
            ui->actionEditCut->setEnabled( true );
            ui->actionEditCopy->setEnabled( true );
            ui->actionEditDelete->setEnabled( true );
            ui->actionEditInsert->setEnabled( true );
            ui->actionEditPaste->setEnabled( true );
            return;
        }*/

        // MDI Windows cannot have a direct focus, but any child of an MDIArea must be a subwindow or subwindow child
        if( ui->mdiArea->isAncestorOf( widget ) ) {
            ItemEditor *itemEditor = qobject_cast<ItemEditor *>( ui->mdiArea->activeSubWindow() );
            if( itemEditor ) {
                undoGroup->setActiveStack( itemEditor->getUndoStack() );
                ui->actionEditOpenItem->setEnabled( false );
                ui->actionEditCloseItem->setEnabled( true );
                ui->actionEditCut->setEnabled( false );
                ui->actionEditCopy->setEnabled( false );
                ui->actionEditDelete->setEnabled( false );
                ui->actionEditInsert->setEnabled( false );
                ui->actionEditPaste->setEnabled( false );
                ui->actionCompileItem->setEnabled( true );
                return;
            }
        }

        const QAbstractItemView *itemView = qobject_cast<const QAbstractItemView *>( widget );
        if( itemView ) {
            if( isItemView( itemView ) || itemView == ui->spriteView || itemView == ui->pictureView ) {
                if( itemView->model() ) {
                    if( QItemSelectionModel *selectionModel = itemView->selectionModel() ) {
                        QPair<bool,bool> openedClosed = isOpenedClosed( widget );
                        ui->actionEditOpenItem->setEnabled( openedClosed.first );
                        ui->actionEditCloseItem->setEnabled( openedClosed.second );
                        ui->actionEditCut->setEnabled( selectionModel->hasSelection() );
                        ui->actionEditCopy->setEnabled( selectionModel->hasSelection() );
                        ui->actionEditDelete->setEnabled( selectionModel->hasSelection() );
                        ui->actionEditDeselect->setEnabled( selectionModel->hasSelection() );
                        ui->actionCompileItem->setEnabled( selectionModel->hasSelection() );
                    }

                    ui->actionEditInsert->setEnabled( isFileOpened( widget ) );

                    if( isItemView( itemView ) )
                        undoGroup->setActiveStack( itemUndoStack );
                    else if( itemView == ui->spriteView )
                        undoGroup->setActiveStack( spriteUndoStack );
                    else if( itemView == ui->pictureView )
                        undoGroup->setActiveStack( pictureUndoStack );

                    QClipboard *clipboard = QApplication::clipboard();
                    if( clipboard && clipboard->mimeData() ) {
                        if( isItemView( itemView ) )
                            ui->actionEditPaste->setEnabled( clipboard->mimeData()->hasFormat( CLIPBOARD_ITEMS ) );
                        else if( itemView == ui->spriteView )
                            ui->actionEditPaste->setEnabled( clipboard->mimeData()->hasFormat( CLIPBOARD_SPRITES ) );
                        else if( itemView == ui->pictureView )
                            ui->actionEditPaste->setEnabled( clipboard->mimeData()->hasFormat( CLIPBOARD_PICTURES ) );
                    }

                    return;
                }
            }
        }

        undoGroup->setActiveStack( NULL );
        ui->actionEditOpenItem->setEnabled( false );
        ui->actionEditCloseItem->setEnabled( false );
        ui->actionEditCut->setEnabled( false );
        ui->actionEditCopy->setEnabled( false );
        ui->actionEditDelete->setEnabled( false );
        ui->actionEditInsert->setEnabled( false );
        ui->actionEditPaste->setEnabled( false );
        ui->actionCompileItem->setEnabled( false );
    }
}

void TibiaEditor::onFocusChanged( QWidget *old, QWidget *current )
{
    Q_UNUSED( old );
    onUpdateActions( current );
}

void TibiaEditor::onLoadItemFile( ItemFile *itemFile )
{
    itemModel = new ItemModel( ui->itemView );
    itemModel->setDragMimeFormat( MIME_TYPE_ITEM );
    itemModel->setDropMimeFormats( QStringList() << MIME_TYPE_ITEM );
    QObject::connect( itemFile, SIGNAL( insertItem( qint32, TibiaItem * ) ), itemModel, SLOT( addItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( itemFile, SIGNAL( removeItem( qint32, TibiaItem * ) ), itemModel, SLOT( removeItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( itemFile, SIGNAL( replaceItem( qint32, TibiaItem * ) ), itemModel, SLOT( replaceItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( itemModel, SIGNAL( decodeMimeDrop( const QMimeData *, int, const QModelIndex& ) ), this, SLOT( onMoveItem( const QMimeData *, int, const QModelIndex& ) ) );
    if( ui->itemView ) {
        if( ui->itemView->selectionModel() )
            ui->itemView->selectionModel()->deleteLater();

        ui->itemView->setModel( itemModel );
        itemModel->setItemList( itemFile->getItems() );

        QObject::connect( ui->itemView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( onSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
    }
    outfitModel = new ItemModel( ui->outfitView );
    outfitModel->setDragMimeFormat( MIME_TYPE_ITEM );
    outfitModel->setDropMimeFormats( QStringList() << MIME_TYPE_ITEM );
    QObject::connect( itemFile, SIGNAL( insertOutfit( qint32, TibiaItem * ) ), outfitModel, SLOT( addItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( itemFile, SIGNAL( removeOutfit( qint32, TibiaItem * ) ), outfitModel, SLOT( removeItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( itemFile, SIGNAL( replaceOutfit( qint32, TibiaItem * ) ), outfitModel, SLOT( replaceItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( outfitModel, SIGNAL( decodeMimeDrop( const QMimeData *, int, const QModelIndex& ) ), this, SLOT( onMoveItem( const QMimeData *, int, const QModelIndex& ) ) );
    if( ui->outfitView ) {
        if( ui->outfitView->selectionModel() )
            ui->outfitView->selectionModel()->deleteLater();

        ui->outfitView->setModel( outfitModel );
        outfitModel->setItemList( itemFile->getOutfits() );

        QObject::connect( ui->outfitView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( onSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
    }
    effectModel = new ItemModel( ui->effectView );
    effectModel->setDragMimeFormat( MIME_TYPE_ITEM );
    effectModel->setDropMimeFormats( QStringList() << MIME_TYPE_ITEM );
    QObject::connect( itemFile, SIGNAL( insertEffect( qint32, TibiaItem * ) ), effectModel, SLOT( addItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( itemFile, SIGNAL( removeEffect( qint32, TibiaItem * ) ), effectModel, SLOT( removeItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( itemFile, SIGNAL( replaceEffect( qint32, TibiaItem * ) ), effectModel, SLOT( replaceItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( effectModel, SIGNAL( decodeMimeDrop( const QMimeData *, int, const QModelIndex& ) ), this, SLOT( onMoveItem( const QMimeData *, int, const QModelIndex& ) ) );
    if( ui->effectView ) {
        if( ui->effectView->selectionModel() )
            ui->effectView->selectionModel()->deleteLater();

        ui->effectView->setModel( effectModel );
        effectModel->setItemList( itemFile->getEffects() );

        QObject::connect( ui->effectView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( onSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
    }
    projectileModel = new ItemModel( ui->projectileView );
    projectileModel->setDragMimeFormat( MIME_TYPE_ITEM );
    projectileModel->setDropMimeFormats( QStringList() << MIME_TYPE_ITEM );
    QObject::connect( itemFile, SIGNAL( insertProjectile( qint32, TibiaItem * ) ), projectileModel, SLOT( addItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( itemFile, SIGNAL( removeProjectile( qint32, TibiaItem * ) ), projectileModel, SLOT( removeItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( itemFile, SIGNAL( replaceProjectile( qint32, TibiaItem * ) ), projectileModel, SLOT( replaceItem( qint32, TibiaItem * ) ), Qt::DirectConnection );
    QObject::connect( projectileModel, SIGNAL( decodeMimeDrop( const QMimeData *, int, const QModelIndex& ) ), this, SLOT( onMoveItem( const QMimeData *, int, const QModelIndex& ) ) );
    if( ui->projectileView ) {
        if( ui->projectileView->selectionModel() )
            ui->projectileView->selectionModel()->deleteLater();

        ui->projectileView->setModel( projectileModel );
        projectileModel->setItemList( itemFile->getProjectiles() );

        QObject::connect( ui->projectileView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( onSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
    }

    ui->actionCompileDat->setEnabled( true );
}

void TibiaEditor::onLoadSpriteFile( SpriteFile *spriteFile )
{
    spriteModel = new SpriteModel( ui->spriteView, RESOURCE_TYPE_SPRITE, spriteFile );
    spriteModel->setDragMimeFormat( MIME_TYPE_SPRITE );
    spriteModel->setDropMimeFormats( QStringList() << MIME_TYPE_SPRITE );
    QObject::connect( &g_resourceHandler, SIGNAL( resourceAdded( quint8, SharedResource&, qint32 ) ), spriteModel, SLOT( addSprite( quint8, SharedResource&, qint32 ) ) );
    QObject::connect( &g_resourceHandler, SIGNAL( resourceUpdated( quint8, qint32, SharedResource& ) ), spriteModel, SLOT( setSprite( quint8, qint32, SharedResource& ) ) );
    QObject::connect( &g_resourceHandler, SIGNAL( resourcesAdded( quint8, ResourceList& ) ), spriteModel, SLOT( addSprites( quint8, ResourceList& ) ) );
    QObject::connect( &g_resourceHandler, SIGNAL( resourcesRemoved( quint8, quint32 ) ), spriteModel, SLOT( removeSprites( quint8, quint32 ) ) );
    QObject::connect( spriteModel, SIGNAL( addedSprites( quint8, quint32 ) ), &g_tibiaHandler, SLOT( onAddSprites( quint8, quint32 ) ) );
    QObject::connect( spriteModel, SIGNAL( removedSprites( quint8, quint32 ) ), &g_tibiaHandler, SLOT( onRemoveSprites( quint8, quint32 ) ) );
    QObject::connect( spriteModel, SIGNAL( decodeMimeDrop( const QMimeData *, int, const QModelIndex& ) ), this, SLOT( onMoveSprite( const QMimeData *, int, const QModelIndex& ) ) );
    if( ui->spriteView ) {
        if( ui->spriteView->selectionModel() )
            ui->spriteView->selectionModel()->deleteLater();

        ui->spriteView->setModel( spriteModel );

        ResourceList resources;
        for( quint32 i = 1; i < spriteFile->getCount(); i++ )
            resources.push_back( g_resourceHandler.loadLocalResource( RESOURCE_TYPE_SPRITE, i, true ) );

        spriteModel->setResourceList( resources );

        QObject::connect( ui->spriteView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( onSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
    }

    ui->actionCompileSpr->setEnabled( true );
}

void TibiaEditor::onLoadPictureFile( PictureFile *pictureFile )
{
    pictureModel = new SpriteModel( ui->pictureView, RESOURCE_TYPE_PICTURE, pictureFile );
    pictureModel->setDragMimeFormat( MIME_TYPE_PICTURE );
    pictureModel->setDropMimeFormats( QStringList() << MIME_TYPE_PICTURE );
    QObject::connect( &g_resourceHandler, SIGNAL( resourceAdded( quint8, SharedResource&, qint32 ) ), pictureModel, SLOT( addSprite( quint8, SharedResource&, qint32 ) ) );
    QObject::connect( &g_resourceHandler, SIGNAL( resourceUpdated( quint8, qint32, SharedResource& ) ), pictureModel, SLOT( setSprite( quint8, qint32, SharedResource& ) ) );
    QObject::connect( &g_resourceHandler, SIGNAL( resourcesAdded( quint8, ResourceList& ) ), pictureModel, SLOT( addSprites( quint8, ResourceList& ) ) );
    QObject::connect( &g_resourceHandler, SIGNAL( resourcesRemoved( quint8, quint32 ) ), pictureModel, SLOT( removeSprites( quint8, quint32 ) ) );
    QObject::connect( pictureModel, SIGNAL( addedSprites( quint8, quint32 ) ), &g_tibiaHandler, SLOT( onAddSprites( quint8, quint32 ) ) );
    QObject::connect( pictureModel, SIGNAL( removedSprites( quint8, quint32 ) ), &g_tibiaHandler, SLOT( onRemoveSprites( quint8, quint32 ) ) );
    QObject::connect( pictureModel, SIGNAL( decodeMimeDrop( const QMimeData *, int, const QModelIndex& ) ), this, SLOT( onMovePicture( const QMimeData *, int, const QModelIndex& ) ) );
    if( ui->pictureView ) {
        if( ui->pictureView->selectionModel() )
            ui->pictureView->selectionModel()->deleteLater();

        ui->pictureView->setModel( pictureModel );

        ResourceList resources;
        for( quint32 i = 0; i < pictureFile->getCount(); i++ )
            resources.push_back( g_resourceHandler.loadLocalResource( RESOURCE_TYPE_PICTURE, i, true ) );

        pictureModel->setResourceList( resources );

        QObject::connect( ui->pictureView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( onSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
    }

    ui->actionCompilePic->setEnabled( true );
}

void TibiaEditor::onUnloadItemFile( void )
{
    if( itemUndoStack )
        itemUndoStack->clear();

    if( itemModel ) {
        itemModel->clear();
        if( ui->itemView ) {
            ui->itemView->setModel( NULL );
            onUpdateActions( ui->itemView );
        }

        delete itemModel;
        itemModel = NULL;
    }

    if( outfitModel ) {
        outfitModel->clear();
        if( ui->outfitView ) {
            ui->outfitView->setModel( NULL );
            onUpdateActions( ui->outfitView );
        }

        delete outfitModel;
        outfitModel = NULL;
    }

    if( effectModel ) {
        effectModel->clear();
        if( ui->effectView ) {
            ui->effectView->setModel( NULL );
            onUpdateActions( ui->effectView );
        }

        delete effectModel;
        effectModel = NULL;
    }

    if( projectileModel ) {
        projectileModel->clear();
        if( ui->projectileView ) {
            ui->projectileView->setModel( NULL );
            onUpdateActions( ui->projectileView );
        }

        delete projectileModel;
        projectileModel = NULL;
    }

    ui->actionCompileDat->setEnabled( false );
}

void TibiaEditor::onUnloadSpriteFile( void )
{
    if( spriteUndoStack )
        spriteUndoStack->clear();

    if( spriteModel ) {
        spriteModel->clear();
        if( ui->spriteView ) {
            ui->spriteView->setModel( NULL );
            onUpdateActions( ui->spriteView );
        }

        delete spriteModel;
        spriteModel = NULL;
    }

    g_resourceHandler.cleanupLocalResources( RESOURCE_TYPE_SPRITE );
    ui->actionCompileSpr->setEnabled( false );
}

void TibiaEditor::onUnloadPictureFile( void )
{
    if( pictureUndoStack )
        pictureUndoStack->clear();

    if( pictureModel ) {
        pictureModel->clear();
        if( ui->pictureView ) {
            ui->pictureView->setModel( NULL );
            onUpdateActions( ui->pictureView );
        }

        delete pictureModel;
        pictureModel = NULL;
    }

    g_resourceHandler.cleanupLocalResources( RESOURCE_TYPE_PICTURE );
    ui->actionCompilePic->setEnabled( false );
}

void TibiaEditor::onInvalidateItems( void )
{
    // File cleanup
    if( g_tibiaHandler.getItemFile() ) {
        if( itemModel )
            itemModel->invalidate();
        if( outfitModel )
            outfitModel->invalidate();
        if( effectModel )
            effectModel->invalidate();
        if( projectileModel )
            projectileModel->invalidate();
    }

    // Item pictures changed
    emit invalidateSubWindows();
}

void TibiaEditor::onSelectionChanged ( const QItemSelection& selected, const QItemSelection& deselected )
{
    Q_UNUSED( selected );
    Q_UNUSED( deselected );

    QItemSelectionModel *selectionModel = qobject_cast<QItemSelectionModel *>( sender() );
    if( selectionModel ) {
        const QAbstractItemModel *model = qobject_cast<const QAbstractItemModel *>( selectionModel->model() );
        if( model ) {
            QWidget *widget = qobject_cast<QWidget *>( model->parent() );
            if( widget ) {
                onUpdateActions( widget );
            }
        }
    }
}

void TibiaEditor::onSelectItems( const ItemList& items, const ItemList& outfits, const ItemList& effects, const ItemList& projectiles )
{
    if( !items.isEmpty() )
        setSelection( ui->itemView, items );
    if( !outfits.isEmpty() )
        setSelection( ui->outfitView, outfits );
    if( !effects.isEmpty() )
        setSelection( ui->effectView, effects );
    if( !projectiles.isEmpty() )
        setSelection( ui->projectileView, projectiles );
}

void TibiaEditor::onChangeItem( ItemFile *_itemFile, TibiaItem *_item, PropertyList _properties )
{
    itemUndoStack->push( new CommandChangeItem( _itemFile, _item, _properties ) );
}

void TibiaEditor::onMoveItem( const QMimeData *data, int toIndex, const QModelIndex& parent )
{
    QByteArray encodedData = data->data( MIME_TYPE_ITEM );
    QDataStream stream( &encodedData, QIODevice::ReadOnly );

    ItemFile *itemFile = g_tibiaHandler.getItemFile();
    if( itemFile ) {
        quint32 size;
        stream >> size;

        for( quint32 i = 0; i < size; i++ ) {
            quint64 fromItemId = 0;
            stream >> fromItemId;
            TibiaItem *fromItem = (TibiaItem *)fromItemId;
            if( fromItem ) {
                TibiaItem *toItem = NULL;
                if( parent.isValid() )
                    toItem = parent.data( Qt::UserRole ).value<TibiaItem *>();

                quint8 toParent = ITEM_PARENT_INTERNAL;
                if( sender() == itemModel )
                    toParent = ITEM_PARENT_ITEMS;
                else if( sender() == outfitModel )
                    toParent = ITEM_PARENT_OUTFITS;
                else if( sender() == effectModel )
                    toParent = ITEM_PARENT_EFFECTS;
                else if( sender() == projectileModel )
                    toParent = ITEM_PARENT_PROJECTILES;

                quint8 fromParent = fromItem->getParentType();
                qint32 fromIndex = itemFile->getItemIndex( fromItem );

                // Moving an item further down in its own list offsets the indexes
                if( fromParent == toParent && fromIndex < toIndex && toIndex > 0 )
                    toIndex--;

                itemUndoStack->push( new CommandRelocateItem( itemFile, fromItem, toItem, ITEM_ACTION_MOVE, toParent, toIndex ) );
            }
        }
    }
}

void TibiaEditor::onOpenItem( TibiaItem *item, const ItemData& itemData )
{
    ItemEditor *itemEditor = new ItemEditor( ui->mdiArea, 0, item, itemData );
    QObject::connect( this, SIGNAL( invalidateSubWindows( ) ), itemEditor, SLOT( onInvalidate( ) ) );
    undoGroup->addStack( itemEditor->getUndoStack() );
    itemEditor->show();
}

void TibiaEditor::onImportSprites( quint8 resourceType, quint32 index, ResourceList resources )
{
    if( resourceType == RESOURCE_TYPE_SPRITE )
        spriteUndoStack->push( new CommandImportSprites( resourceType, index, resources ) );
    else if( resourceType == RESOURCE_TYPE_PICTURE )
        pictureUndoStack->push( new CommandImportSprites( resourceType, index, resources ) );
}

void TibiaEditor::onImportItems( quint8 destination, ItemList items )
{
    ItemFile *itemFile = g_tibiaHandler.getItemFile();
    if( itemFile ) {
        if( items.size() > 1 )
            itemUndoStack->push( new CommandRelocateItems( itemFile, items, NULL, ITEM_ACTION_IMPORT_NEW, destination ) );
        else
            itemUndoStack->push( new CommandRelocateItem( itemFile, items.first(), NULL, ITEM_ACTION_IMPORT_NEW, destination ) );
        if( items.size() >= 1 )
            g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkGreen ), tr( "Imported %n Item(s)", 0, items.size() ) );
    }
}

void TibiaEditor::onMoveSprite( const QMimeData *data, int toIndex, const QModelIndex& parent )
{
    QByteArray encodedData = data->data( MIME_TYPE_SPRITE );
    QDataStream stream( &encodedData, QIODevice::ReadOnly );

    SpriteFile *spriteFile = g_tibiaHandler.getSpriteFile();
    if( spriteFile ) {
        qint32 size;
        stream >> size;

        // Only move one at a time
        SharedResource fromResource;
        stream >> fromResource;

        if( fromResource ) {
            SharedResource toResource;
            if( parent.isValid() ) {
                toResource = parent.data( Qt::UserRole ).value<SharedResource>();
                toIndex = spriteModel->getIndexByResource( toResource );

                // Do sprite movement
                spriteUndoStack->push( new CommandSwapSprite( spriteModel->getIndexByResource( fromResource ), toIndex, fromResource, toResource ) );
            }
        }
    }
}

void TibiaEditor::onMovePicture( const QMimeData *data, int toIndex, const QModelIndex& parent )
{
    QByteArray encodedData = data->data( MIME_TYPE_PICTURE );
    QDataStream stream( &encodedData, QIODevice::ReadOnly );

    PictureFile *pictureFile = g_tibiaHandler.getPictureFile();
    if( pictureFile ) {
        quint32 size;
        stream >> size;

        // Only move one at a time
        SharedResource fromResource;
        stream >> fromResource;

        if( fromResource ) {
            SharedResource toResource;
            if( parent.isValid() ) {
                toResource = parent.data( Qt::UserRole ).value<SharedResource>();
                toIndex = pictureModel->getIndexByResource( toResource );

                // Do sprite movement
                pictureUndoStack->push( new CommandSwapSprite( pictureModel->getIndexByResource( fromResource ), toIndex, fromResource, toResource ) );
            }
        }
    }
}
