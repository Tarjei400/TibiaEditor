#ifndef ITEMSEARCH_H
#define ITEMSEARCH_H

#include <QtGui/QDialog>
#include <QtCore/QTime>
#include <QtCore/QFutureWatcher>
#include "ui_itemsearch.h"

class ItemModel;
class QItemSelectionModel;

class ItemSearch : public QDialog
{
    Q_OBJECT

public:
    ItemSearch( QWidget *parent = 0 );
    virtual ~ItemSearch( void );

    void toggleAdvanced( void );
    quint32 colorDominance( const QImage& image, quint8 colorIntensity = 30 ) const;
    ItemList itemSelection( QItemSelectionModel *selectionModel );

    void performSearch( const HeaderList& headerSearch, const PropertyList& propertySearch, ItemFile *itemFile );
    bool verifyItem( const HeaderList& headerSearch, const PropertyList& propertySearch, const TibiaItem *item ) const;

private:
    QFutureWatcher<void> searchWatcher;
    QTime time;

    ItemList results;
    ItemModel *itemModel;
    Ui::ItemSearchClass *ui;

signals:
    void selectItems( const ItemList&, const ItemList&, const ItemList&, const ItemList& );

protected:
    virtual void closeEvent( QCloseEvent *event );

private slots:
    void onSearchComplete( void );
    void on_buttonSelect_clicked( void );
    void on_buttonFind_clicked( void );
    void on_comboVersions_currentIndexChanged( int );
    void on_checkAdvanced_stateChanged( int );
};

#endif // ITEMSEARCH_H
