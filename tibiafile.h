#ifndef TIBIAFILE_H
#define TIBIAFILE_H

#include <QtCore/QFile>
#include <QtCore/QDataStream>

class TibiaItem;
class TibiaSprite;

class ItemFile;
class SpriteFile;
class PictureFile;
class TibiaModule;

class TibiaFile : public QFile
{
    Q_OBJECT

public:
    TibiaFile( QObject *parent );
    virtual ~TibiaFile() {};

    virtual bool createNew( void ) {
        return false;
    };
    virtual bool isLoaded( void ) const {
        return false;
    };
    virtual void unload( void );
    virtual bool load( const QString& ) = 0;
    virtual bool save( const QString& ) = 0;
    virtual bool idle( const QString&, bool ) = 0;

    virtual ItemFile *getItemFile() {
        return NULL;
    };
    virtual const ItemFile *getItemFile() const {
        return NULL;
    };

    virtual SpriteFile *getSpriteFile() {
        return NULL;
    };
    virtual const SpriteFile *getSpriteFile() const {
        return NULL;
    };

    virtual PictureFile *getPictureFile() {
        return NULL;
    };
    virtual const PictureFile *getPictureFile() const {
        return NULL;
    };

    virtual TibiaModule *getTibiaModule() {
        return NULL;
    };
    virtual const TibiaModule *getTibiaModule() const {
        return NULL;
    };

    static quint32 peekSignature( const QString& fileName );
    static void readSprite( QDataStream& in, TibiaSprite& sprite, quint32 offset_x, quint32 offset_y, bool read_rgb = true );
    static void writeSprite( QDataStream& out, const TibiaSprite& sprite, quint32 offset_x, quint32 offset_y, quint32& dumpOffset, bool write_rgb = true );

    quint32 getSignature( void ) const {
        return m_signature;
    };
    void setSignature( quint32 signature ) {
        m_signature = signature;
    };
    bool isIdle( void ) const {
        return m_idle;
    };

signals:
    void parseError( QString, QFile::FileError );
    void documentError( QString, QString, int );

private:
    quint32 m_signature;
    bool m_idle;

    friend class SpriteFile;
    friend class ItemFile;
    friend class PictureFile;
    friend class ExternalFile;
    friend class LibraryFile;
};

#endif
