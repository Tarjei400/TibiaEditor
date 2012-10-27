#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QVector>
#include "tibiahandler.h"
#include "tibiaitem.h"

Q_DECLARE_METATYPE( TibiaItem * )

class ItemModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ItemModel( QObject *_parent = 0 );

    void setDragMimeFormat( const QString& mimeType );
    void setDropMimeFormats( const QStringList& mimeTypes );

    void setItemFlags( Qt::ItemFlags flags );
    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() );
    void clear( void );
    void deleteAll( void );

    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    virtual QMimeData *mimeData( const QModelIndexList& indexes ) const;

    void setSupportedDropActions( Qt::DropActions actions );
    virtual Qt::DropActions supportedDropActions( void ) const;

    virtual QStringList mimeTypes( void ) const;
    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent );

    void setItemList( const ItemList& items );
    void addItemList( const ItemList& items );

    ItemList& getItemList( void ) {
        return m_itemList;
    };

    void invalidate( void );

    void setDrawParameters( const DrawParameters& _dParams );
    const DrawParameters& getDrawParameters( void ) {
        return m_dParams;
    };

    QModelIndex indexByItem( TibiaItem * );

public slots:
    void addItem( qint32 index = -1, TibiaItem *tibiaItem = NULL );
    bool removeItem( qint32 index = -1, TibiaItem *tibiaItem = NULL );
    void replaceItem( qint32 index = -1, TibiaItem *tibiaItem = NULL );
    void updateItem( TibiaItem *item );

signals:
    void decodeMimeDrop( const QMimeData *, int, const QModelIndex& );

private:
    DrawParameters m_dParams;
    QString m_dragMimeType;
    QStringList m_dropMimeTypes;
    Qt::DropActions m_dropActions;
    Qt::ItemFlags m_itemFlags;
    ItemList m_itemList;
};

#endif // SPRITEMODEL_H
