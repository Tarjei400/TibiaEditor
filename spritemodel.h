#ifndef SPRITEMODEL_H
#define SPRITEMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include "spritefile.h"
#include "picturefile.h"
#include "resourcehandler.h"

class SpriteModel : public QAbstractListModel
{
    Q_OBJECT

public:
    SpriteModel( QObject *parent = 0, quint8 resourceType = 0, TibiaFile *tibiaFile = 0 );
    ~SpriteModel( void );

    void setDragMimeFormat( const QString& mimeType );
    void setDropMimeFormats( const QStringList& mimeTypes );

    void setItemFlags( Qt::ItemFlags flags );
    void clear( void );

    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const;
    virtual int rowCount( const QModelIndex& parent ) const;

    QMimeData *mimeData( const QModelIndexList& indexes ) const;

    ResourceList getResourceList( void ) const {
        return m_resourceList;
    }
    void setResourceList( const ResourceList& sprites );

    void setSupportedDropActions( Qt::DropActions actions );
    virtual Qt::DropActions supportedDropActions( void ) const;

    virtual QStringList mimeTypes( void ) const;
    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent );

    qint32 getIndexByResource( SharedResource& obj ) const;

public slots:
    void addSprite( quint8, SharedResource& obj, qint32 index = -1 );
    bool removeSprite( quint8, qint32 index );
    void setSprite( quint8, qint32 index, SharedResource& obj );
    void addSprites( quint8, ResourceList& resources );
    void removeSprites( quint8, quint32 );

signals:
    void addedSprites( quint8, quint32 );
    void removedSprites( quint8, quint32 );
    void changedSprite( quint8, quint32 );
    void decodeMimeDrop( const QMimeData *data, int row, const QModelIndex& parent );

private:
    quint8 m_resourceType;
    TibiaFile *m_tibiaFile;
    QString m_dragMimeType;
    QStringList m_dropMimeTypes;
    Qt::DropActions m_dropActions;
    Qt::ItemFlags m_itemFlags;
    ResourceList m_resourceList;
};

Q_DECLARE_METATYPE( SharedResource )

#endif // SPRITEMODEL_H
