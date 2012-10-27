#ifndef UNDOFRAMEWORK_H
#define UNDOFRAMEWORK_H

#include <QtGui/QUndoCommand>
#include "itemfile.h"
#include "resourcehandler.h"

class SpriteModel;
class QSpinBox;
class ItemEditor;

enum ItemAction {
    ITEM_ACTION_NONE,
    ITEM_ACTION_INSERT_NEW,
    ITEM_ACTION_INSERT,
    ITEM_ACTION_MOVE_NEW,
    ITEM_ACTION_MOVE,
    ITEM_ACTION_PASTE_NEW,
    ITEM_ACTION_PASTE,
    ITEM_ACTION_IMPORT_NEW,
    ITEM_ACTION_IMPORT,
    ITEM_ACTION_REPLACE
};

class CommandSetItemAppearance : public QUndoCommand
{
public:
    CommandSetItemAppearance( ItemEditor *itemEditor, ItemData *itemData, QUndoCommand *parent = NULL );
    ~CommandSetItemAppearance( void ) {};
    virtual void undo( void );
    virtual void redo( void );

private:
    ItemEditor *m_itemEditor;
    ItemData *m_itemData;
    quint8 m_oldWidth, m_oldHeight, m_oldCropsize, m_oldLayers, m_oldXDiv, m_oldYDiv, m_oldZDiv, m_oldAnimations;
    quint8 m_newWidth, m_newHeight, m_newCropsize, m_newLayers, m_newXDiv, m_newYDiv, m_newZDiv, m_newAnimations;
};

class CommandSetItemProperties : public QUndoCommand
{
public:
    CommandSetItemProperties( ItemEditor *itemEditor, ItemData *itemData, PropertyList properties, QUndoCommand *parent = NULL );
    ~CommandSetItemProperties( void ) {};
    virtual void undo( void );
    virtual void redo( void );

private:
    ItemEditor *m_itemEditor;
    ItemData *m_itemData;
    PropertyList m_oldProperties;
    PropertyList m_properties;
};

class CommandImportSprites : public QUndoCommand
{
public:
    CommandImportSprites( quint8 resourceType, quint32 indexStart, ResourceList& resources, QUndoCommand *parent = NULL );
    ~CommandImportSprites( void );
    virtual void undo( void );
    virtual void redo( void );

private:
    quint8 m_resourceType;
    quint32 m_indexStart;
    ResourceList m_resources;
    bool m_resourceOwner;
};

class CommandSwapSprite : public QUndoCommand
{
public:
    CommandSwapSprite( qint32 fromIndex, qint32 toIndex, SharedResource& fromResource, SharedResource& toResource, QUndoCommand *parent = NULL );
    ~CommandSwapSprite( void );
    virtual void undo( void );
    virtual void redo( void );

private:
    qint32 m_fromIndex;
    qint32 m_toIndex;
    SharedResource m_fromResource;
    SharedResource m_toResource;
};

class CommandCutItems : public QUndoCommand
{
public:
    CommandCutItems( ItemFile *_itemFile, const ItemList& _items, QUndoCommand *parent = NULL );
    ~CommandCutItems( void );
    virtual void undo( void );
    virtual void redo( void );

private:
    ItemFile *m_itemFile;
    ItemList m_items;
    QList<QPair<qint32, TibiaItem *> > m_cutItems;
    quint8 m_parent;
    bool m_itemsOwner;
};

class CommandChangeItem : public QUndoCommand
{
public:
    CommandChangeItem( ItemFile *_itemFile, TibiaItem *_item, PropertyList _properties, QUndoCommand *parent = NULL );
    ~CommandChangeItem( void ) {};
    virtual void undo( void );
    virtual void redo( void );

private:
    ItemFile *m_itemFile;
    TibiaItem *m_item;
    PropertyList m_beforeProperties;
    PropertyList m_afterProperties;
};

class CommandRemoveItems : public QUndoCommand
{
public:
    CommandRemoveItems( ItemFile *_itemFile, const ItemList& _items, QUndoCommand *parent = NULL );
    virtual void undo( void ) {
        QUndoCommand::undo();
    };
    virtual void redo( void ) {
        QUndoCommand::redo();
    };
};

class CommandRemoveItem : public QUndoCommand
{
public:
    CommandRemoveItem( ItemFile *itemFile, TibiaItem *item, QUndoCommand *parent = NULL );
    ~CommandRemoveItem( void );
    virtual void undo( void );
    virtual void redo( void );

private:
    ItemFile *m_itemFile;
    TibiaItem *m_item;
    qint32 m_index;
    quint8 m_parent;
    bool m_itemOwner;
};

class CommandRelocateItems : public QUndoCommand
{
public:
    CommandRelocateItems( ItemFile *itemFile, const ItemList& items, TibiaItem *toItem, ItemAction action, quint8 toParent = ITEM_PARENT_INTERNAL, qint32 toIndex = -1, QUndoCommand *parent = NULL );
    virtual void undo( void ) {
        QUndoCommand::undo();
    };
    virtual void redo( void ) {
        QUndoCommand::redo();
    };
};

class CommandRelocateItem : public QUndoCommand
{
public:
    CommandRelocateItem( ItemFile *itemFile, TibiaItem *fromItem, TibiaItem *toItem, ItemAction action, quint8 toParent = ITEM_PARENT_INTERNAL, qint32 toIndex = -1, QUndoCommand *parent = NULL );
    ~CommandRelocateItem( void );
    virtual void undo( void );
    virtual void redo( void );

private:
    ItemFile *m_itemFile;
    TibiaItem *m_fromItem;
    TibiaItem *m_toItem;
    qint32 m_fromIndex;
    qint32 m_toIndex;
    quint8 m_toParent;
    quint8 m_fromParent;
    ItemAction m_action;
    bool m_toItemOwner;
    bool m_fromItemOwner;
};

inline bool isItemContainer( quint8 parent )
{
    return ( parent == ITEM_PARENT_ITEMS || parent == ITEM_PARENT_OUTFITS || parent == ITEM_PARENT_EFFECTS || parent == ITEM_PARENT_PROJECTILES );
};

#endif // UNDOFRAMEWORK_H
