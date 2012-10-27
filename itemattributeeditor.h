#ifndef ITEMATTRIBUTEEDITOR_H
#define ITEMATTRIBUTEEDITOR_H

#include <QtGui/QDialog>
#include "ui_itemattributeeditor.h"

class TibiaHandler;
class ItemModel;

class ItemAttributeEditor : public QDialog
{
    Q_OBJECT

public:
    ItemAttributeEditor( QWidget *parent = 0 );
    ~ItemAttributeEditor( void );

    void setItems( const ItemList& items );
    void setOutfits( const ItemList& items );
    void setEffects( const ItemList& items );
    void setProjectiles( const ItemList& items );
    void updateItems( void );
    void setProperties( TibiaItem *item, const PropertyList& properties );

private:
    ItemList items;
    ItemList outfits;
    ItemList effects;
    ItemList projectiles;
    ItemModel *itemModel;
    Ui::ItemAttributeEditorClass *ui;

signals:
    void changeItem( ItemFile *, TibiaItem *, PropertyList );

private slots:
    void onSelectionChanged( bool );
    void onButtonClicked( QAbstractButton *button );
    void on_comboVersions_currentIndexChanged( int );
};

#endif // ITEMATTRIBUTEEDITOR_H
