#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>

#include "mimeconstants.h"

#include "itemeditor.h"
#include "undoframework.h"
#include "formathandler.h"
#include "itemspritemodel.h"

extern TibiaHandler g_tibiaHandler;
extern FormatHandler g_formatHandler;

ItemEditor::ItemEditor( QWidget *parent, Qt::WindowFlags flags, TibiaItem *tibiaItem, const ItemData& itemData ) : QMdiSubWindow( parent, flags ), ui( new Ui::ItemEditorClass )
{
    m_itemData = itemData;
    m_tibiaItem = tibiaItem;
    m_internalWidget = new QWidget;
    m_undoStack = new QUndoStack;
    ui->setupUi( m_internalWidget );
    ui->buttonLayout->setAlignment ( ui->viewDirectionE, Qt::AlignCenter );
    ui->buttonLayout->setAlignment ( ui->viewDirectionN, Qt::AlignCenter );
    ui->buttonLayout->setAlignment ( ui->viewDirectionNE, Qt::AlignCenter );
    ui->buttonLayout->setAlignment ( ui->viewDirectionNW, Qt::AlignCenter );
    ui->buttonLayout->setAlignment ( ui->viewDirectionS, Qt::AlignCenter );
    ui->buttonLayout->setAlignment ( ui->viewDirectionSE, Qt::AlignCenter );
    ui->buttonLayout->setAlignment ( ui->viewDirectionSW, Qt::AlignCenter );
    ui->buttonLayout->setAlignment ( ui->viewDirectionW, Qt::AlignCenter );
    setWidget( m_internalWidget );
    setAttribute( Qt::WA_DeleteOnClose );

    setWindowTitle( ( tibiaItem ? TibiaItem::fullName( g_tibiaHandler.getItemFile(), tibiaItem ) : TibiaItem::typeName( itemData.type ) ) + " [*]" );
    setGeometry( m_internalWidget->geometry() );

    if( g_formatHandler.isLoaded() ) {
        for( DatFormatMap::const_iterator it = g_formatHandler.getFormatsBegin(); it != g_formatHandler.getFormatsEnd(); it++ )
            ui->comboVersions->insertItem( 0, it.value()->getName(), QVariant::fromValue( it.key() ) );
    } else
        ui->comboVersions->insertItem( 0, tr( "No Formats" ) );

    QObject::connect( ui->viewImage, SIGNAL( imageChanged( const QImage& ) ), this, SLOT( onImageChanged( const QImage& ) ) );
    QObject::connect( ui->viewShowNumbers, SIGNAL( toggled( bool ) ), ui->viewImage, SLOT( showNumbers( bool ) ) );
    QObject::connect( ui->comboVersions, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onCurrentIndexChanged( int ) ) );

    dParams.showPattern = true;

    ui->viewImage->setMimeDropType( MIME_TYPE_SPRITE );
    ui->viewImage->setImage( g_tibiaHandler.drawItem( m_itemData, dParams ) );
    ui->itemAttributes->setFormat( g_formatHandler.getBaseFormat() );

    m_spriteModel = new ItemSpriteModel( ui->spriteView, &m_itemData );
    m_spriteModel->setDragMimeFormat( MIME_TYPE_SPRITE );
    m_spriteModel->setDropMimeFormats( QStringList() << MIME_TYPE_SPRITE );
    ui->spriteView->setModel( m_spriteModel );

    onLoadItem();

    QObject::connect( m_undoStack, SIGNAL( canUndoChanged( bool ) ), this, SLOT( setWindowModified( bool ) ) );
    QObject::connect( ui->viewAppearanceBox, SIGNAL( clicked( QAbstractButton * ) ), this, SLOT( onAppearanceButtonClicked( QAbstractButton * ) ) );
    QObject::connect( ui->viewAttributeBox, SIGNAL( clicked( QAbstractButton * ) ), this, SLOT( onAttributeButtonClicked( QAbstractButton * ) ) );
    QObject::connect( ui->viewBlendLayers, SIGNAL( toggled( bool ) ), this, SLOT( onToggleBlend( bool ) ) );
    QObject::connect( ui->viewAnimation, SIGNAL( valueChanged( int ) ), this, SLOT( onSetAnimation( int ) ) );
    QObject::connect( ui->viewLayer, SIGNAL( valueChanged( int ) ), this, SLOT( onSetLayer( int ) ) );
    QObject::connect( ui->viewAddon, SIGNAL( valueChanged( int ) ), this, SLOT( onSetAddon( int ) ) );
    QObject::connect( ui->viewFloorPattern, SIGNAL( valueChanged( int ) ), this, SLOT( onSetFloorPattern( int ) ) );
    QObject::connect( ui->viewDirectionE, SIGNAL( toggled( bool ) ), this, SLOT( onSetDirectionEast( void ) ) );
    QObject::connect( ui->viewDirectionN, SIGNAL( toggled( bool ) ), this, SLOT( onSetDirectionNorth( void ) ) );
    QObject::connect( ui->viewDirectionNE, SIGNAL( toggled( bool ) ), this, SLOT( onSetDirectionNorthEast( void ) ) );
    QObject::connect( ui->viewDirectionNW, SIGNAL( toggled( bool ) ), this, SLOT( onSetDirectionNorthWest( void ) ) );
    QObject::connect( ui->viewDirectionS, SIGNAL( toggled( bool ) ), this, SLOT( onSetDirectionSouth( void ) ) );
    QObject::connect( ui->viewDirectionSE, SIGNAL( toggled( bool ) ), this, SLOT( onSetDirectionSouthEast( void ) ) );
    QObject::connect( ui->viewDirectionSW, SIGNAL( toggled( bool ) ), this, SLOT( onSetDirectionSouthWest( void ) ) );
    QObject::connect( ui->viewDirectionW, SIGNAL( toggled( bool ) ), this, SLOT( onSetDirectionWest( void ) ) );
}

ItemEditor::~ItemEditor( void )
{
    delete m_undoStack;
    delete m_internalWidget;
    delete ui;
}

void ItemEditor::closeEvent( QCloseEvent *event )
{
    if( isWindowModified() ) {
        switch( QMessageBox::question( this, tr( "Modified" ), tr( "Changes were made to this item, do you wish to save?" ), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save ) ) {
        case QMessageBox::Save:
            emit savedItem( m_tibiaItem, m_itemData );
            break;
        case QMessageBox::Discard:
            event->accept();
            return;
            break;
        case QMessageBox::Cancel:
            event->ignore();
            return;
            break;
        }
    }

    event->accept();
}

void ItemEditor::repaint( void )
{
    ui->viewImage->setImage( g_tibiaHandler.drawItem( m_itemData, dParams ) );
}

void ItemEditor::onInvalidate( void )
{
    repaint();
    if( m_spriteModel )
        m_spriteModel->invalidate();
}

void ItemEditor::onLoadItem( void )
{
    ui->viewAttributeWidth->setValue( m_itemData.width );
    ui->viewAttributeHeight->setValue( m_itemData.height );
    ui->viewAttributeCropsize->setValue( m_itemData.cropsize );
    ui->viewAttributeLayers->setValue( m_itemData.blendframes );
    ui->viewAttributeXDiv->setValue( m_itemData.xdiv );
    ui->viewAttributeYDiv->setValue( m_itemData.ydiv );
    ui->viewAttributeZDiv->setValue( m_itemData.zdiv );
    ui->viewAttributeAnimations->setValue( m_itemData.animcount );
    ui->itemAttributes->setAttributes( m_itemData.getProperties() );

    onSetupAnimations();
    onSetupLayers();
    onSetupAddons();
    onSetupFloorPattern();
    onSetupDirections();
}

void ItemEditor::onSetupAnimations( void )
{
    ui->viewAnimation->setRange( 0, m_itemData.animcount - 1 );
    bool hasAnimation = ( m_itemData.animcount - 1 > 0 );
    ui->viewLabelAnimation->setShown( hasAnimation );
    ui->viewAnimationValue->setShown( hasAnimation );
    ui->viewAnimation->setShown( hasAnimation );
}

void ItemEditor::onSetupLayers( void )
{
    ui->viewLayer->setRange( 0, m_itemData.blendframes - 1 );
    bool hasLayers = ( m_itemData.blendframes - 1 > 0 );
    ui->viewLabelLayer->setShown( hasLayers );
    ui->viewLayerValue->setShown( hasLayers );
    ui->viewLayer->setShown( hasLayers );
}

void ItemEditor::onSetupAddons( void )
{
    ui->viewAddon->setRange( 0, m_itemData.ydiv );
    bool hasAddons = ( m_itemData.ydiv == 3 && m_itemData.type == ITEM_TYPE_OUTFIT );
    ui->viewLabelAddon->setShown( hasAddons );
    ui->viewAddonValue->setShown( hasAddons );
    ui->viewAddon->setShown( hasAddons );
}

void ItemEditor::onSetupFloorPattern( void )
{
    ui->viewFloorPattern->setRange( 0, m_itemData.zdiv - 1 );
    bool hasZPattern = ( m_itemData.zdiv - 1 > 0 );
    ui->viewLabelFloorPattern->setShown( hasZPattern );
    ui->viewFloorPatternValue->setShown( hasZPattern );
    ui->viewFloorPattern->setShown( hasZPattern );
}

void ItemEditor::onSetupDirections( void )
{
    if( m_itemData.type == ITEM_TYPE_OUTFIT ) {
        ui->viewDirectionE->setShown( true );
        ui->viewDirectionN->setShown( true );
        ui->viewDirectionNE->setShown( false );
        ui->viewDirectionNW->setShown( false );
        ui->viewDirectionS->setShown( true );
        ui->viewDirectionSE->setShown( false );
        ui->viewDirectionSW->setShown( false );
        ui->viewDirectionW->setShown( true );
    } else if( m_itemData.type == ITEM_TYPE_PROJECTILE ) {
        ui->viewDirectionE->setShown( true );
        ui->viewDirectionN->setShown( true );
        ui->viewDirectionNE->setShown( true );
        ui->viewDirectionNW->setShown( true );
        ui->viewDirectionS->setShown( true );
        ui->viewDirectionSE->setShown( true );
        ui->viewDirectionSW->setShown( true );
        ui->viewDirectionW->setShown( true );
    } else {
        ui->viewDirectionE->setShown( false );
        ui->viewDirectionN->setShown( false );
        ui->viewDirectionNE->setShown( false );
        ui->viewDirectionNW->setShown( false );
        ui->viewDirectionS->setShown( false );
        ui->viewDirectionSE->setShown( false );
        ui->viewDirectionSW->setShown( false );
        ui->viewDirectionW->setShown( false );
    }
}

void ItemEditor::onToggleBlend( bool blend )
{
    if( blend ) {
        dParams.blendOverlay = -1;
        dParams.blendFrame = -1;
    } else {
        dParams.blendOverlay = ui->viewLayer->value();
        dParams.blendFrame = ui->viewLayer->value();
    }
    repaint();
}

void ItemEditor::onSetAnimation( int value )
{
    dParams.animation = value;
    repaint();
}

void ItemEditor::onSetLayer( int value )
{
    if( !ui->viewBlendLayers->isChecked() ) {
        dParams.blendOverlay = ( qint16 )value;
        dParams.blendFrame = ( qint16 )value;
        repaint();
    }
}

void ItemEditor::onSetAddon( int value )
{
    dParams.addons = value;
    repaint();
}

void ItemEditor::onSetFloorPattern( int value )
{
    dParams.floor = value;
    repaint();
}

void ItemEditor::onSetDirectionEast( void )
{
    dParams.outfitDirection = OUTFIT_DIRECTION_EAST;
    dParams.projectileDirection = PROJECTILE_DIRECTION_EAST;
    repaint();
}

void ItemEditor::onSetDirectionNorth( void )
{
    dParams.outfitDirection = OUTFIT_DIRECTION_NORTH;
    dParams.projectileDirection = PROJECTILE_DIRECTION_NORTH;
    repaint();
}

void ItemEditor::onSetDirectionNorthEast( void )
{
    dParams.outfitDirection = OUTFIT_DIRECTION_DEFAULT;
    dParams.projectileDirection = PROJECTILE_DIRECTION_NORTHEAST;
    repaint();
}

void ItemEditor::onSetDirectionNorthWest( void )
{
    dParams.outfitDirection = OUTFIT_DIRECTION_DEFAULT;
    dParams.projectileDirection = PROJECTILE_DIRECTION_NORTHWEST;
    repaint();
}

void ItemEditor::onSetDirectionSouth( void )
{
    dParams.outfitDirection = OUTFIT_DIRECTION_SOUTH;
    dParams.projectileDirection = PROJECTILE_DIRECTION_SOUTH;
    repaint();
}

void ItemEditor::onSetDirectionSouthEast( void )
{
    dParams.outfitDirection = OUTFIT_DIRECTION_DEFAULT;
    dParams.projectileDirection = PROJECTILE_DIRECTION_SOUTHEAST;
    repaint();
}

void ItemEditor::onSetDirectionSouthWest( void )
{
    dParams.outfitDirection = OUTFIT_DIRECTION_DEFAULT;
    dParams.projectileDirection = PROJECTILE_DIRECTION_SOUTHWEST;
    repaint();
}

void ItemEditor::onSetDirectionWest( void )
{
    dParams.outfitDirection = OUTFIT_DIRECTION_WEST;
    dParams.projectileDirection = PROJECTILE_DIRECTION_WEST;
    repaint();
}

void ItemEditor::onAttributeButtonClicked( QAbstractButton *button )
{
    switch( ui->viewAttributeBox->buttonRole( button ) ) {
    case QDialogButtonBox::ApplyRole: {
        PropertyList properties;
        ui->itemAttributes->getAttributes( properties );
        m_undoStack->push( new CommandSetItemProperties( this, &m_itemData, properties ) );
    }
    break;
    }
}

void ItemEditor::onAppearanceButtonClicked( QAbstractButton *button )
{
    switch( ui->viewAppearanceBox->buttonRole( button ) ) {
    case QDialogButtonBox::ApplyRole:
        m_undoStack->push( new CommandSetItemAppearance( this, &m_itemData ) );
        break;
    }
}

void ItemEditor::onImageChanged( const QImage& image )
{
    setWindowIcon( QIcon( QPixmap::fromImage( image ) ) );
}

void ItemEditor::onCurrentIndexChanged( int index )
{
    qint32 version = ui->comboVersions->itemData( index, Qt::UserRole ).toInt();
    DatFormat *datFormat = g_formatHandler.getFormatByClient( version );
    if( datFormat ) {
        ui->itemAttributes->setFormat( datFormat );
    }
}
