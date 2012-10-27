#ifndef ITEMEDITOR_H
#define ITEMEDITOR_H

#include <QtGui/QMdiSubWindow>
#include <QtGui/QUndoStack>
#include <QtGui/QWidget>
#include "tibiahandler.h"
#include "ui_itemeditor.h"

class ItemSpriteModel;

class ItemEditor : public QMdiSubWindow
{
    Q_OBJECT

public:
    ItemEditor( QWidget *parent = 0, Qt::WindowFlags flags = 0, TibiaItem *tibiaItem = NULL, const ItemData& itemData = ItemData() );
    ~ItemEditor( void );

    TibiaItem *getTibiaItem( void ) const {
        return m_tibiaItem;
    };
    QUndoStack *getUndoStack( void ) const {
        return m_undoStack;
    };

    QSpinBox *getWidth( void ) const {
        return ui->viewAttributeWidth;
    };
    QSpinBox *getHeight( void ) const {
        return ui->viewAttributeHeight;
    };
    QSpinBox *getCropsize( void ) const {
        return ui->viewAttributeCropsize;
    };
    QSpinBox *getLayers( void ) const {
        return ui->viewAttributeLayers;
    };
    QSpinBox *getXDiv( void ) const {
        return ui->viewAttributeXDiv;
    };
    QSpinBox *getYDiv( void ) const {
        return ui->viewAttributeYDiv;
    };
    QSpinBox *getZDiv( void ) const {
        return ui->viewAttributeZDiv;
    };
    QSpinBox *getAnimations( void ) const {
        return ui->viewAttributeAnimations;
    };
    ItemAttributes *getItemAttributes( void ) const {
        return ui->itemAttributes;
    };

protected:
    virtual void closeEvent( QCloseEvent *event );

private:
    DrawParameters dParams;

    ItemSpriteModel *m_spriteModel;
    QUndoStack *m_undoStack;
    TibiaItem *m_tibiaItem;
    ItemData m_itemData;
    QWidget *m_internalWidget;
    Ui::ItemEditorClass *ui;

signals:
    void savedItem( TibiaItem *, const ItemData& );

public slots:
    void repaint( void );
    void onInvalidate( void );

    void onLoadItem( void );
    void onSetupAnimations( void );
    void onSetupLayers( void );
    void onSetupAddons( void );
    void onSetupFloorPattern( void );
    void onSetupDirections( void );

    void onToggleBlend( bool );
    void onSetAnimation( int );
    void onSetLayer( int );
    void onSetAddon( int );
    void onSetFloorPattern( int value );
    void onSetDirectionEast( void );
    void onSetDirectionNorth( void );
    void onSetDirectionNorthEast( void );
    void onSetDirectionNorthWest( void );
    void onSetDirectionSouth( void );
    void onSetDirectionSouthEast( void );
    void onSetDirectionSouthWest( void );
    void onSetDirectionWest( void );

    void onAppearanceButtonClicked( QAbstractButton * );
    void onAttributeButtonClicked( QAbstractButton * );
    void onImageChanged( const QImage& );
    void onCurrentIndexChanged( int );

    friend class CommandSetItemProperties;
    friend class CommandSetItemAppearance;
};

#endif // ITEMEDITOR_H
