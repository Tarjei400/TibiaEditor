#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtCore/QDebug>

#include "undoframework.h"
#include "itemattributes.h"
#include "itemeditor.h"

extern ResourceHandler g_resourceHandler;

CommandSetItemAppearance::CommandSetItemAppearance( ItemEditor *itemEditor, ItemData *itemData, QUndoCommand *parent ) : QUndoCommand( parent ),
    m_itemEditor( itemEditor ),
    m_itemData( itemData )
{
    m_oldWidth = m_itemData->width;
    m_oldHeight = m_itemData->height;
    m_oldCropsize = m_itemData->cropsize;
    m_oldLayers = m_itemData->blendframes;
    m_oldXDiv = m_itemData->xdiv;
    m_oldYDiv = m_itemData->ydiv;
    m_oldZDiv = m_itemData->zdiv;
    m_oldAnimations = m_itemData->animcount;

    m_newWidth = itemEditor->getWidth()->value();
    m_newHeight = itemEditor->getHeight()->value();
    m_newCropsize = itemEditor->getCropsize()->value();
    m_newLayers = itemEditor->getLayers()->value();
    m_newXDiv = itemEditor->getXDiv()->value();
    m_newYDiv = itemEditor->getYDiv()->value();
    m_newZDiv = itemEditor->getZDiv()->value();
    m_newAnimations = itemEditor->getAnimations()->value();


    setText( QObject::tr( "Set %1 appearance" ).arg( TibiaItem::typeName( itemData->type ) ) );
}

void CommandSetItemAppearance::undo( void )
{
    m_itemData->width = m_oldWidth;
    m_itemData->height = m_oldHeight;
    m_itemData->cropsize = m_oldCropsize;
    m_itemData->blendframes = m_oldLayers;
    m_itemData->xdiv = m_oldXDiv;
    m_itemData->ydiv = m_oldYDiv;
    m_itemData->zdiv = m_oldZDiv;
    m_itemData->animcount = m_oldAnimations;

    m_itemEditor->getWidth()->setValue( m_itemData->width );
    m_itemEditor->getHeight()->setValue( m_itemData->height );
    m_itemEditor->getCropsize()->setValue( m_itemData->cropsize );
    m_itemEditor->getLayers()->setValue( m_itemData->blendframes );
    m_itemEditor->getXDiv()->setValue( m_itemData->xdiv );
    m_itemEditor->getYDiv()->setValue( m_itemData->ydiv );
    m_itemEditor->getZDiv()->setValue( m_itemData->zdiv );
    m_itemEditor->getAnimations()->setValue( m_itemData->animcount );
    m_itemEditor->onInvalidate();
    m_itemEditor->onSetupAnimations();
    m_itemEditor->onSetupLayers();
    m_itemEditor->onSetupAddons();
    m_itemEditor->onSetupFloorPattern();
    m_itemEditor->onSetupDirections();
}

void CommandSetItemAppearance::redo( void )
{
    m_itemData->width = m_newWidth;
    m_itemData->height = m_newHeight;
    m_itemData->cropsize = m_newCropsize;
    m_itemData->blendframes = m_newLayers;
    m_itemData->xdiv = m_newXDiv;
    m_itemData->ydiv = m_newYDiv;
    m_itemData->zdiv = m_newZDiv;
    m_itemData->animcount = m_newAnimations;

    m_itemEditor->getWidth()->setValue( m_itemData->width );
    m_itemEditor->getHeight()->setValue( m_itemData->height );
    m_itemEditor->getCropsize()->setValue( m_itemData->cropsize );
    m_itemEditor->getLayers()->setValue( m_itemData->blendframes );
    m_itemEditor->getXDiv()->setValue( m_itemData->xdiv );
    m_itemEditor->getYDiv()->setValue( m_itemData->ydiv );
    m_itemEditor->getZDiv()->setValue( m_itemData->zdiv );
    m_itemEditor->getAnimations()->setValue( m_itemData->animcount );
    m_itemEditor->onInvalidate();
    m_itemEditor->onSetupAnimations();
    m_itemEditor->onSetupLayers();
    m_itemEditor->onSetupAddons();
    m_itemEditor->onSetupFloorPattern();
    m_itemEditor->onSetupDirections();
}

CommandSetItemProperties::CommandSetItemProperties( ItemEditor *itemEditor, ItemData *itemData, PropertyList properties, QUndoCommand *parent ) : QUndoCommand( parent ),
    m_itemEditor( itemEditor ),
    m_itemData( itemData ),
    m_properties( properties )
{
    m_oldProperties = m_itemData->getProperties();
    setText( QObject::tr( "Set %1 properties" ).arg( TibiaItem::typeName( itemData->type ) ) );
}

void CommandSetItemProperties::undo( void )
{
    m_itemData->setProperties( m_oldProperties );
    m_itemEditor->getItemAttributes()->setAttributes( m_oldProperties );
}

void CommandSetItemProperties::redo( void )
{
    m_itemData->setProperties( m_properties );
    m_itemEditor->getItemAttributes()->setAttributes( m_properties );
}

CommandImportSprites::CommandImportSprites( quint8 resourceType, quint32 indexStart, ResourceList& resources, QUndoCommand *parent ) : QUndoCommand( parent ),
    m_resourceType( resourceType ),
    m_resources( resources ),
    m_resourceOwner( false ),
    m_indexStart( indexStart )
{
    switch( resourceType ) {
    case RESOURCE_TYPE_SPRITE:
        setText( QObject::tr( "Imported %n Sprite(s)", "", resources.size() ) );
        break;
    case RESOURCE_TYPE_PICTURE:
        setText( QObject::tr( "Imported %n Picture(s)", "", resources.size() ) );
        break;
    }
}

CommandImportSprites::~CommandImportSprites( void )
{
    if( m_resourceOwner )
        m_resources.clear();
}

void CommandImportSprites::undo( void )
{
    g_resourceHandler.removeLocalResources( m_resourceType, m_resources );
    //g_resourceHandler.addForeignResources( m_resourceType, m_resources ); // They do not need to be passed back to foreign, they are now stored in this undo command
    emit g_resourceHandler.resourcesRemoved( m_resourceType, m_resources.size() );
    m_resourceOwner = true;
}

void CommandImportSprites::redo( void )
{
    g_resourceHandler.removeForeignResources( m_resourceType, m_resources );
    g_resourceHandler.addLocalResources( m_resourceType, m_indexStart, m_resources );
    emit g_resourceHandler.resourcesAdded( m_resourceType, m_resources );
    m_resourceOwner = false;
}

CommandSwapSprite::CommandSwapSprite( qint32 fromIndex, qint32 toIndex, SharedResource& fromResource, SharedResource& toResource, QUndoCommand *parent ) : QUndoCommand( parent ),
    m_fromIndex( fromIndex ),
    m_toIndex( toIndex ),
    m_fromResource( fromResource ),
    m_toResource( toResource )
{
    if( fromResource->getType() == RESOURCE_TYPE_SPRITE && toResource->getType() == RESOURCE_TYPE_SPRITE )
        setText( QObject::tr( "Swapped Sprite %1 with Sprite %2" ).arg( g_resourceHandler.getDisplayIdentifier( fromResource ) ).arg( g_resourceHandler.getDisplayIdentifier( toResource ) ) );
    else if( fromResource->getType() == RESOURCE_TYPE_PICTURE && toResource->getType() == RESOURCE_TYPE_PICTURE )
        setText( QObject::tr( "Swapped Picture %1 with Picture %2" ).arg( g_resourceHandler.getDisplayIdentifier( fromResource ) ).arg( g_resourceHandler.getDisplayIdentifier( toResource ) ) );
}

CommandSwapSprite::~CommandSwapSprite( void )
{

}

void CommandSwapSprite::undo( void )
{
    g_resourceHandler.swapResources( m_toResource, m_fromResource );
    emit g_resourceHandler.resourceUpdated( m_toResource->getType(), m_toIndex, m_toResource );
    emit g_resourceHandler.resourceUpdated( m_fromResource->getType(), m_fromIndex, m_fromResource );
}

void CommandSwapSprite::redo( void )
{
    g_resourceHandler.swapResources( m_fromResource, m_toResource );
    emit g_resourceHandler.resourceUpdated( m_toResource->getType(), m_toIndex, m_toResource );
    emit g_resourceHandler.resourceUpdated( m_fromResource->getType(), m_fromIndex, m_fromResource );
}

CommandCutItems::CommandCutItems( ItemFile *_itemFile, const ItemList& _items, QUndoCommand *parent ) : QUndoCommand( parent ),
    m_itemFile( _itemFile ),
    m_items( _items ),
    m_itemsOwner( false )
{
    m_parent = m_items.first()->getParentType();

    QString itemsString;
    foreach( TibiaItem* item, m_items ) {
        itemsString += QObject::tr( "%1" ).arg( m_itemFile->getItemType( item ) );
        m_cutItems.push_back( QPair<qint32, TibiaItem *>( m_itemFile->getItemIndex( item ), item ) );
        if( m_items.size() > 1 && item != m_items.last() )
            itemsString += QString( ", " );
    }

    setText( QObject::tr( "Cut %n %1(s)(%2)", "", m_cutItems.size() ).arg( TibiaItem::typeName( m_items.first()->getType() ) ).arg( itemsString ) );
}

CommandCutItems::~CommandCutItems( void )
{
    if( m_itemsOwner ) { // We undid our changes and overwrote this command
        for( QList<QPair<qint32, TibiaItem *> >::iterator it = m_cutItems.begin(); it != m_cutItems.end(); it++ ) {
            if( (*it).second ) {
                delete (*it).second;
                (*it).second = NULL;
            }
        }

        m_cutItems.clear();
    }
}

void CommandCutItems::undo( void )
{
    if( !m_itemFile || !m_items.first() )
        return;

    for( QList<QPair<qint32, TibiaItem *> >::iterator it = m_cutItems.begin(); it != m_cutItems.end(); it++ ) {
        if( (*it).second )
            m_itemFile->insert( m_parent, (*it).first, (*it).second );
    }

    m_itemsOwner = false;
}

void CommandCutItems::redo( void )
{
    if( !m_itemFile || !m_items.first() )
        return;

    foreach( TibiaItem* item, m_items )
    m_itemFile->remove( item );

    m_itemsOwner = true;
}

CommandChangeItem::CommandChangeItem( ItemFile *itemFile, TibiaItem *item, PropertyList properties, QUndoCommand *parent ) : QUndoCommand( parent ),
    m_itemFile( itemFile ),
    m_item( item ),
    m_afterProperties( properties )
{

    setText( QObject::tr( "Changed %1(%2)" ).arg( TibiaItem::typeName( item->getType() ) ).arg( itemFile->getItemType( item ) ) );
}

void CommandChangeItem::undo( void )
{
    m_item->setProperties( m_beforeProperties );
}

void CommandChangeItem::redo( void )
{
    m_beforeProperties = m_item->getProperties();
    m_item->setProperties( m_afterProperties );
}

CommandRemoveItems::CommandRemoveItems( ItemFile *_itemFile, const ItemList& _items, QUndoCommand *parent ) : QUndoCommand( parent )
{
    foreach( TibiaItem* item, _items )
    new CommandRemoveItem( _itemFile, item, this );

    setText( QObject::tr( "Removed %n Item(s)", 0, childCount() ) );
}


CommandRemoveItem::CommandRemoveItem( ItemFile *itemFile, TibiaItem *item, QUndoCommand *parent ) : QUndoCommand( parent ),
    m_itemFile( itemFile ),
    m_item( item ),
    m_itemOwner( false )
{
    setText( QObject::tr( "Removed %1" ).arg( TibiaItem::fullName( itemFile, item ) ) );
}

CommandRemoveItem::~CommandRemoveItem( void )
{
    if( m_itemOwner ) { // We undid our changes and overwrote this command
        delete m_item;
        m_item = NULL;
    }
}

void CommandRemoveItem::undo( void )
{
    if( !m_itemFile || !m_item )
        return;

    if( m_index != -1 ) {
        m_itemFile->insert( m_parent, m_index, m_item );
        m_itemOwner = false;
    }
}

void CommandRemoveItem::redo( void )
{
    if( !m_itemFile || !m_item )
        return;

    m_parent = m_item->getParentType();
    m_index = m_itemFile->getItemIndex( m_item );

    m_itemFile->remove( m_item );
    m_itemOwner = true;
}

CommandRelocateItems::CommandRelocateItems( ItemFile *itemFile, const ItemList& items, TibiaItem *toItem, ItemAction action, quint8 toParent, qint32 toIndex, QUndoCommand *parent ) : QUndoCommand( parent )
{
    if( toItem && toParent == ITEM_PARENT_INTERNAL )
        toParent = toItem->getParentType();

    QString toItemText;
    TibiaItem *referenceItem = itemFile->getItemByIndex( toParent, toIndex - 1 );
    if( referenceItem )
        toItemText = TibiaItem::fullName( itemFile, referenceItem );

    if( toItem && action == ITEM_ACTION_REPLACE ) // Adding an item with a reference
        toItemText = TibiaItem::fullName( itemFile, toItem );

    if( !toItem && !referenceItem ) { // Added to list without a reference
        if( action == ITEM_ACTION_MOVE )
            action = ITEM_ACTION_MOVE_NEW;
        if( action == ITEM_ACTION_INSERT )
            action = ITEM_ACTION_INSERT_NEW;
        if( action == ITEM_ACTION_IMPORT )
            action = ITEM_ACTION_IMPORT_NEW;
        if( action == ITEM_ACTION_PASTE )
            action = ITEM_ACTION_PASTE_NEW;

        toItemText = TibiaItem::parentName( toParent );
    }

    foreach( TibiaItem* item, items )
    new CommandRelocateItem( itemFile, item, toItem, action, toParent, toIndex, this );

    QString commandName;
    switch( action ) {
    case ITEM_ACTION_MOVE_NEW:
        commandName = QObject::tr( "Moved %n Item(s) into %1", 0, childCount() ).arg( toItemText );
        break;
    case ITEM_ACTION_MOVE:
        commandName = QObject::tr( "Moved %n Item(s) after %1", 0, childCount() ).arg( toItemText );
        break;
    case ITEM_ACTION_IMPORT_NEW:
        commandName = QObject::tr( "Imported %n Item(s) into %1", 0, childCount() ).arg( toItemText );
        break;
    case ITEM_ACTION_IMPORT:
        commandName = QObject::tr( "Imported %n Item(s) after %1", 0, childCount() ).arg( toItemText );
        break;
    case ITEM_ACTION_INSERT_NEW:
        commandName = QObject::tr( "Added %n Item(s) into %1", 0, childCount() ).arg( toItemText );
        break;
    case ITEM_ACTION_INSERT:
        commandName = QObject::tr( "Added %n Item(s) after %1", 0, childCount() ).arg( toItemText );
        break;
    case ITEM_ACTION_PASTE_NEW:
        commandName = QObject::tr( "Pasted %n Item(s) into %1", 0, childCount() ).arg( toItemText );
        break;
    case ITEM_ACTION_PASTE:
        commandName = QObject::tr( "Pasted %n Item(s) after %1", 0, childCount() ).arg( toItemText );
        break;
    case ITEM_ACTION_REPLACE:
        commandName = QObject::tr( "Replaced %n Item(s) with %1", 0, childCount() ).arg( toItemText );
        break;
    }

    setText( commandName );
}

CommandRelocateItem::CommandRelocateItem( ItemFile *itemFile, TibiaItem *fromItem, TibiaItem *toItem, ItemAction action, quint8 toParent, qint32 toIndex, QUndoCommand *parent ) : QUndoCommand( parent ),
    m_itemFile( itemFile ),
    m_fromItem( fromItem ),
    m_toItem( toItem ),
    m_action( action ),
    m_toParent( toParent ),
    m_toIndex( toIndex ),
    m_fromParent( ITEM_PARENT_INTERNAL ),
    m_fromIndex( -1 ),
    m_toItemOwner( false ),
    m_fromItemOwner( false )
{

    if( !m_fromItem && ( m_action == ITEM_ACTION_INSERT || m_action == ITEM_ACTION_INSERT_NEW ) ) { // Create the item we dont have
        m_fromItem = new TibiaItem;
        m_fromItemOwner = true;
    }
    if( m_toItem && m_action != ITEM_ACTION_INSERT && m_action != ITEM_ACTION_INSERT_NEW && m_action != ITEM_ACTION_PASTE && m_action != ITEM_ACTION_PASTE_NEW ) {
        m_action = ITEM_ACTION_REPLACE;
        m_toParent = m_toItem->getParentType();
    }
    if( m_toItem && ( m_action == ITEM_ACTION_INSERT || m_action == ITEM_ACTION_INSERT_NEW || m_action == ITEM_ACTION_PASTE || m_action == ITEM_ACTION_PASTE_NEW || m_action == ITEM_ACTION_IMPORT || m_action == ITEM_ACTION_IMPORT_NEW ) ) {
        m_toParent = m_toItem->getParentType();
        m_toIndex = m_itemFile->getItemIndex( m_toItem ) + 1;
    }

    m_fromParent = m_fromItem->getParentType();
    m_fromIndex = m_itemFile->getItemIndex( m_fromItem );

    QString fromItemText = TibiaItem::fullName( m_itemFile, m_fromItem );
    QString toItemText;
    TibiaItem *referenceItem = m_itemFile->getItemByIndex( m_toParent, m_toIndex - 1 );
    if( referenceItem )
        toItemText = TibiaItem::fullName( m_itemFile, referenceItem );

    if( m_toItem && m_action == ITEM_ACTION_REPLACE ) // Adding an item with a reference
        toItemText = TibiaItem::fullName( m_itemFile, m_toItem );

    if( !m_toItem && !referenceItem ) { // Added to list without a reference
        if( m_action == ITEM_ACTION_MOVE )
            m_action = ITEM_ACTION_MOVE_NEW;
        if( m_action == ITEM_ACTION_INSERT )
            m_action = ITEM_ACTION_INSERT_NEW;
        if( m_action == ITEM_ACTION_IMPORT )
            m_action = ITEM_ACTION_IMPORT_NEW;
        if( m_action == ITEM_ACTION_PASTE )
            m_action = ITEM_ACTION_PASTE_NEW;

        toItemText = TibiaItem::parentName( m_toParent );
    }

    QString commandName;
    switch( m_action ) {
    case ITEM_ACTION_MOVE_NEW:
        commandName = QObject::tr( "Moved %1 into %2" ).arg( fromItemText ).arg( toItemText );
        break;
    case ITEM_ACTION_MOVE:
        commandName = QObject::tr( "Moved %1 after %2" ).arg( fromItemText ).arg( toItemText );
        break;
    case ITEM_ACTION_IMPORT_NEW:
        commandName = QObject::tr( "Imported %1 into %2" ).arg( fromItemText ).arg( toItemText );
        break;
    case ITEM_ACTION_IMPORT:
        commandName = QObject::tr( "Imported %1 after %2" ).arg( fromItemText ).arg( toItemText );
        break;
    case ITEM_ACTION_INSERT_NEW:
        commandName = QObject::tr( "Added %1 into %2" ).arg( fromItemText ).arg( toItemText );
        break;
    case ITEM_ACTION_INSERT:
        commandName = QObject::tr( "Added %1 after %2" ).arg( fromItemText ).arg( toItemText );
        break;
    case ITEM_ACTION_PASTE_NEW:
        commandName = QObject::tr( "Pasted %1 into %2" ).arg( fromItemText ).arg( toItemText );
        break;
    case ITEM_ACTION_PASTE:
        commandName = QObject::tr( "Pasted %1 after %2" ).arg( fromItemText ).arg( toItemText );
        break;
    case ITEM_ACTION_REPLACE:
        commandName = QObject::tr( "Replaced %1 with %2" ).arg( toItemText ).arg( fromItemText );
        break;
    }

    setText( commandName );
}

CommandRelocateItem::~CommandRelocateItem( void )
{
    if( m_toItemOwner ) {
        delete m_toItem;
        m_toItem = NULL;
    }
    if( m_fromItemOwner ) {
        delete m_fromItem;
        m_fromItem = NULL;
    }
}

void CommandRelocateItem::undo( void )
{
    switch( m_action ) {
    case ITEM_ACTION_MOVE_NEW:
    case ITEM_ACTION_MOVE: {
        // Move the item back to where it was (We own neither items)
        m_itemFile->remove( m_fromItem );
        if( isItemContainer( m_fromParent ) && m_fromIndex != -1 )
            m_itemFile->insert( m_fromParent, m_fromIndex, m_fromItem );
    }
    break;
    case ITEM_ACTION_INSERT_NEW:
    case ITEM_ACTION_INSERT:
    case ITEM_ACTION_PASTE_NEW:
    case ITEM_ACTION_PASTE:
    case ITEM_ACTION_IMPORT_NEW:
    case ITEM_ACTION_IMPORT: {
        // Get rid of the item we created
        m_itemFile->remove( m_fromItem );
        m_fromItemOwner = true;
    }
    break;
    case ITEM_ACTION_REPLACE: {
        if( m_toItem ) { // Bring back the item we replaced
            m_itemFile->replace( m_toItem, m_fromItem );
            if( isItemContainer( m_fromParent ) && m_fromIndex != -1 ) // Replacing item with a new item
                m_itemFile->insert( m_fromParent, m_fromIndex, m_fromItem );
            m_toItemOwner = false;
        }
    }
    break;
    }
}

void CommandRelocateItem::redo( void )
{
    if( m_toItem && m_action == ITEM_ACTION_REPLACE ) { // Adding an item with a reference
        m_itemFile->remove( m_fromItem ); // Remove the item were we go it from
        m_itemFile->replace( m_fromItem, m_toItem );
        m_toItemOwner = true; // We item we replace is no longer in the list
    } else if( isItemContainer( m_toParent ) ) { // Adding an item with no reference
        if( m_action == ITEM_ACTION_MOVE || m_action == ITEM_ACTION_MOVE_NEW )
            m_itemFile->remove( m_fromItem ); // Remove the item were we go it from

        m_itemFile->insert( m_toParent, m_toIndex, m_fromItem );
        m_fromItemOwner = false; // The item we added is no longer in ours
    }
}
