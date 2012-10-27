#ifndef ITEMATTRIBUTES_H
#define ITEMATTRIBUTES_H

#include <QtCore/QMultiHash>
/*#include <QPair>
#include <QList>
#include <QHash>*/
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>

#include <QtGui/QWidget>
#include "tibiaitem.h"
#include "ui_itemattributes.h"

class DatFormat;

typedef QMultiHash<quint8, QWidget *> WidgetHash;

/*typedef QPair<QLabel*, QSpinBox*> ValuePair;
typedef QList<ValuePair> ValueList;
typedef QPair<QCheckBox*, ValueList> AttributePair;
typedef QHash<quint8, AttributePair> AttributeHash;*/

class ItemAttributes : public QWidget
{
    Q_OBJECT

public:
    ItemAttributes( QWidget *parent = 0 );
    virtual ~ItemAttributes( void );

    void showValues( bool shown ) {
        ui->valueArea->setShown( shown );
    };
    void setFormat( DatFormat *datFormat );

    void setHeaders( const HeaderList& headerList );
    void getHeaders( HeaderList& headerList );
    void setAttributes( const PropertyList& properties );
    void getAttributes( PropertyList& properties );

    QCheckBox *getHeader( quint8 header ) const;
    QLabel *getLabel( quint8 header, quint8 index, quint8 size ) const;
    QSpinBox *getValue( quint8 header, quint8 index, quint8 size ) const;

public slots:
    void onSetupContents( void );
    void onResetContents( void );
    void onUnloadContents( void );

private:
    Ui::ItemAttributesClass *ui;
    //AttributeHash attributeHash;
    WidgetHash widgetHash;
    DatFormat *m_datFormat;
};

#endif // ITEMATTRIBUTES_H
