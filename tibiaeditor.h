#ifndef TIBIAEDITOR_H
#define TIBIAEDITOR_H

#include <QtGui/QMainWindow>
#include <QtCore/QFile>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QUndoView>
#include <QtGui/QUndoStack>
#include <QtGui/QUndoCommand>
#include <QtGui/QUndoGroup>

#include "undoframework.h"
#include "ui_tibiaeditor.h"
#include "spritemodel.h"
#include "tibiahandler.h"

class ItemFile;
class TibiaItem;
class ItemEditor;

class TibiaEditor : public QMainWindow
{
    Q_OBJECT

public:
    TibiaEditor( QWidget *parent = 0, Qt::WFlags flags = 0 );
    virtual ~TibiaEditor( void );

    static ItemList itemSelection( QItemSelectionModel * );
    static ResourceList spriteSelection( QItemSelectionModel * );

private:
    void setSelection( QAbstractItemView *itemView, const ItemList& items );
    bool isFileOpened( const QWidget * ) const;
    QPair<bool,bool> isOpenedClosed( const QWidget *widget );
    bool isItemView( const QAbstractItemView *itemView ) const {
        return ( itemView == ui->itemView || itemView == ui->outfitView || itemView == ui->effectView || itemView == ui->projectileView );
    };

    QList<ItemEditor *> itemEditors( void ) const;

    ItemModel *itemModel;
    ItemModel *outfitModel;
    ItemModel *effectModel;
    ItemModel *projectileModel;

    SpriteModel *spriteModel;
    SpriteModel *pictureModel;

    QWidget *fillWidget;
    QDockWidget *outputDock;

    QUndoGroup *undoGroup;
    QUndoStack *itemUndoStack;
    QUndoStack *spriteUndoStack;
    QUndoStack *pictureUndoStack;

    Ui::TibiaEditorClass *ui;

protected:
    virtual void closeEvent( QCloseEvent *event );

signals:
    void invalidateSubWindows( void );

public slots:
    // Error Slots
    void onFileError( QString, QFile::FileError );
    void onDocumentError( QString, QString, int );

    // File Slots
    void onFileNewDat( void );
    void onFileNewSpr( void );
    void onFileNewPic( void );
    void onFileNewItem( void );
    void onFileOpenDat( void );
    void onFileOpenSpr( void );
    void onFileOpenPic( void );
    void onFileOpenItem( void );
    void onFileCompileDat( void );
    void onFileCompileSpr( void );
    void onFileCompilePic( void );
    void onFileCompileItem( void );
    void onFileImport( void );
    void onFileExport( void );

    // File Save Slots
    void onSaveStarted( void );
    void onSaveComplete( void );
    void onSaveSuccess( int );

    // Edit Slots
    void onEditInsert( void );
    void onEditDelete( void );
    void onEditCut( void );
    void onEditCopy( void );
    void onEditPaste( void );
    void onEditSelectAll( void );
    void onEditDeselect( void );
    void onEditProperties( void );
    void onEditItemProperties( void );
    void onEditFind( void );
    void onEditOpenItem( void );
    void onEditCloseItem( void );

    // UI Slots
    void onFullScreen( bool );
    void onTreeViewContextMenu( const QPoint& );
    void onUpdateActions( const QWidget *widget );
    void onFocusChanged( QWidget *, QWidget * );

    // Display Slots
    void onLoadItemFile( ItemFile *itemFile );
    void onLoadSpriteFile( SpriteFile *spriteFile );
    void onLoadPictureFile( PictureFile *pictureFile );
    void onUnloadItemFile( void );
    void onUnloadSpriteFile( void );
    void onUnloadPictureFile( void );
    void onInvalidateItems( void );

    // Selection Slots
    void onSelectionChanged ( const QItemSelection&, const QItemSelection& );
    void onSelectItems( const ItemList&, const ItemList&, const ItemList&, const ItemList& );

    // Item Change Slots
    void onChangeItem( ItemFile *, TibiaItem *, PropertyList );
    void onMoveItem( const QMimeData *, int, const QModelIndex& );
    void onOpenItem( TibiaItem *, const ItemData& );

    void onMoveSprite( const QMimeData *, int, const QModelIndex& );
    void onMovePicture( const QMimeData *, int, const QModelIndex& );

    // Import Slots
    void onImportSprites( quint8, quint32, ResourceList );
    void onImportItems( quint8, ItemList );
};

#endif // TIBIAEDITOR_H
