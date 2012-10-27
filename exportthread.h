#ifndef EXPORTTHREAD_H
#define EXPORTTHREAD_H

#include <QtCore/QDir>
#include "userthread.h"
#include "tibiaresource.h"
#include "tibiahandler.h"
#include "resourcehandler.h"

class TibiaItem;

enum ExportMode_t {
    EXPORT_NONE = 0,
    EXPORT_ITEM,
    EXPORT_LIBRARY,
    EXPORT_PNG,
    EXPORT_BMP
};

class ExportThread : public UserThread
{
    Q_OBJECT

public:
    ExportThread( QObject *parent = 0, ExportMode_t mode = EXPORT_NONE );
    virtual ~ExportThread( void );

    virtual void setup( void );

    ExportMode_t getMode( void ) const {
        return m_exportMode;
    };

    void exportItems( const QString& name, ItemList& items, quint32& currentItem, QTime& time );
    void exportSprites( const QString& name, ResourceList& sprites, quint32& currentSprite, QTime& time );

    bool exportSprite( const QDir& dir, const SharedResource& );
    bool exportItem( const QDir& dir, TibiaItem * );

    void setFormat( const quint32 _format );
    void setMode( const ExportMode_t mode );
    void setDirectory( const QString& dir );
    void setItems( const ItemList& vector );
    void setOutfits( const ItemList& vector );
    void setEffects( const ItemList& vector );
    void setProjectiles( const ItemList& vector );
    void setSprites( const ResourceList& vector );
    void setPictures( const ResourceList& vector );
    void setDrawParameters( const DrawParameters& params );

protected:
    virtual void run( void );

private:
    bool m_abort;
    mutable QMutex mutex;
    quint32 m_format;
    QString m_directory;
    ExportMode_t m_exportMode;
    DrawParameters m_dParams;
    ItemList m_items;
    ItemList m_outfits;
    ItemList m_effects;
    ItemList m_projectiles;
    ResourceList m_sprites;
    ResourceList m_pictures;
};

#endif // EXPORTTHREAD_H
