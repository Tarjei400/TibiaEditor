#ifndef IMPORTHANDLER_H
#define IMPORTHANDLER_H

#include <QtGui/QDialog>
#include <QtCore/QDir>
#include <QtCore/QFutureWatcher>
#include <QtCore/QTime>
#include <QtGui/QCloseEvent>
#include "ui_importhandler.h"
#include "importthread.h"

#define TAB_INDEX_ITEMS 0
#define TAB_INDEX_SPRITES 1
#define TAB_INDEX_PICTURES 2

#define IMPORT_TYPE_INSERT 0
#define IMPORT_TYPE_APPEND 1
#define IMPORT_TYPE_REPLACE 2

class ItemModel;
class SpriteModel;

class ImportHandler : public QDialog
{
    Q_OBJECT

public:
    ImportHandler( QWidget *parent = 0 );
    virtual ~ImportHandler( void );

    void selectItems( void );
    void selectOutfits( void );
    void selectEffects( void );
    void selectProjectiles( void );
    void selectSprites( void );
    void selectPictures( void );

    void processFiles( const QStringList& selectedFiles );
    bool processFile( const QString& filePath, QObject *destination );

    void pathSearch( QStringList& validFiles, const QString& path );
    void recursiveSearch( QStringList& validFiles, const QDir& dir );

    void performImport( ObjectVector objects, qint8 importDestination );

    void importItems( ItemList& items, qint8 importDestination );
    void importSprites( const ResourceList& resources, quint8 resourceType, quint32 count );

    ImportThread *getThread( void ) const {
        return importThread;
    };
    DropFileThread *getAltThread( void ) const {
        return dropThread;
    };

    void dropFiles(const QStringList& strings, const QUrlList& urls, DropMode_t dropMode );

private:
    ImportThread *importThread;
    DropFileThread *dropThread;

    QAction *actionDelete;
    QAction *destItems;
    QAction *destOutfits;
    QAction *destEffects;
    QAction *destProjectiles;

    ItemModel *itemModel;
    SpriteModel *spriteModel;
    SpriteModel *pictureModel;

    QList<QFile *> fileList;

    Ui::ImportHandlerClass *ui;

private slots:
    void onDropTibiaFile( QFile * );
    void onDropItems( ItemList );
    void onDropSprites( quint8, ResourceList );
    void onDropSuccess( int );
    void onImportStarted( void );
    void onImportComplete( void );
    void onImportSuccess( int );
    void onTreeViewContextMenu( const QPoint& );
    void onDelete( void );
    void onDropFiles( const QMimeData *, int, const QModelIndex& );
    void onButtonClicked( QAbstractButton * );

protected:
    virtual void closeEvent( QCloseEvent *event );
};

#endif // IMPORTHANDLER_H
