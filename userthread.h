#ifndef USERTHREAD_H
#define USERTHREAD_H

#include <QtCore/QThread>
#include <QtCore/QFile>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

class QProgressDialog;

class UserThread : public QThread
{
    Q_OBJECT

public:
    UserThread( QObject *parent ) : QThread( parent ) {
        setup();
    };
    ~UserThread( void ) {
        mutex.lock();
        m_abort = true;
        mutex.unlock();
        wait();
    };

    virtual void setup( void );
    void execute( QProgressDialog * );

    bool isCanceled() const;
    void setName( const QString& name );
    void setLabel( const QString& label );
    void setWindowTitle( const QString& title );
    const QString& getLabel( void ) const;
    const QString& getWindowTitle( void ) const;
    void setMinimum( int min );
    void setMaximum( int max );
    int getMinimum( void ) const;
    int getMaximum( void ) const;

signals:
    void minimum( int );
    void maximum( int );
    void windowTitle( const QString& );
    void labelText( const QString& );
    void valueChanged( int );
    void success( int );
    void parseError( QString, QFile::FileError );
    void documentError( QString, QString, int );
    //void rateChanged( int, int, int );

public slots:
    void cancel( void );

protected:
    virtual void run( void ) {};

private:
    bool m_abort;
    mutable QMutex mutex;

    int m_minimum;
    int m_maximum;

    QString m_name;
    QString m_labelText;
    QString m_windowTitle;

    friend class ItemThread;
    friend class SpriteThread;
    friend class PictureThread;
    friend class LibraryThread;
    friend class ExportThread;
    friend class ImportThread;
    friend class DropFileThread;
};

class ItemThread : public UserThread
{
    Q_OBJECT

public:
    ItemThread( QObject *parent ) : UserThread( parent ) {};

protected:
    virtual void run( void );
};

class SpriteThread : public UserThread
{
    Q_OBJECT

public:
    SpriteThread( QObject *parent ) : UserThread( parent ) {};

protected:
    virtual void run( void );
};

class PictureThread : public UserThread
{
    Q_OBJECT

public:
    PictureThread( QObject *parent ) : UserThread( parent ) {};

protected:
    virtual void run( void );
};

class LibraryThread : public UserThread
{
    Q_OBJECT

public:
    LibraryThread( QObject *parent ) : UserThread( parent ) {};

protected:
    virtual void run( void );
};

#endif // SAVETHREAD_H
