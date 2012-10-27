#ifndef EXPORTHANDLER_H
#define EXPORTHANDLER_H

#include <QtCore/QTime>
#include <QtCore/QDir>
#include <QtGui/QDialog>
#include <QtCore/QThread>
#include <QtGui/QCloseEvent>
#include <QtCore/QMutexLocker>
#include "exportthread.h"
#include "spritemodel.h"
#include "ui_exporthandler.h"

class ItemModel;
class SpriteModel;

class ExportHandler : public QDialog
{
    Q_OBJECT

public:
    ExportHandler( QWidget *parent = 0 );
    virtual ~ExportHandler( void );

    void selectItems( bool value );
    void selectOutfits( bool value );
    void selectEffects( bool value );
    void selectProjectiles( bool value );
    void selectSprites( void );
    void selectPictures( void );

    void setItems( const ItemList& vector );
    void setOutfits( const ItemList& vector );
    void setEffects( const ItemList& vector );
    void setProjectiles( const ItemList& vector );
    void updateItems( void );
    void setSprites( const ResourceList& vector );
    void setPictures( const ResourceList& vector );
    void updateSprites( void );
    void updatePictures( void );
    void setParameters( void );

protected:
    virtual void closeEvent( QCloseEvent *event );

private:
    ExportThread *exportThread;
    ItemModel *itemModel;
    SpriteModel *spriteModel;
    SpriteModel *pictureModel;

    ItemList items;
    ItemList outfits;
    ItemList effects;
    ItemList projectiles;

    ResourceList sprites;
    ResourceList pictures;

    DrawParameters dParams;

    QTime time;

    Ui::ExportHandlerClass *ui;

private slots:
    void onExportStarted( void );
    void onExportSuccess( int );
    void onExportComplete( void );
    void onBrowse( bool );
    void onExport( bool );
    void onPatternChanged( bool );
    void onSelectionChanged( bool );
    void onParameterChanged( int );
};

#endif // EXPORTHANDLER_H
