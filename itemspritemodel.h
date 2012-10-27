#ifndef SPRITEMODEL_H
#define SPRITEMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include "resourcehandler.h"

class ItemSpriteModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ItemSpriteModel( QObject *parent = 0, ItemData *itemData = NULL );

    void setDragMimeFormat( const QString& mimeType );
    void setDropMimeFormats( const QStringList& mimeTypes );

    void setItemFlags( Qt::ItemFlags flags );
    void clear( void );

    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
    virtual int rowCount( const QModelIndex& parent ) const;

    QMimeData *mimeData( const QModelIndexList& indexes ) const;

    void setSupportedDropActions( Qt::DropActions actions );
    virtual Qt::DropActions supportedDropActions( void ) const;

    virtual QStringList mimeTypes( void ) const;
    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent );

signals:
    void decodeMimeDrop( const QMimeData *data, int row, const QModelIndex& parent );

public slots:
    void invalidate( void );

private:
    ItemData *m_itemData;
    QString m_dragMimeType;
    QStringList m_dropMimeTypes;
    Qt::DropActions m_dropActions;
    Qt::ItemFlags m_itemFlags;
};

Q_DECLARE_METATYPE( SharedResource )

#endif // SPRITEMODEL_H
