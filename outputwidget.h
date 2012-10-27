#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <QtGui/QTextEdit>

class OutputWidget : public QTextEdit
{
    Q_OBJECT

public:
    OutputWidget( QWidget *parent = 0 );
    void addLine( const QColor& color, const QString& string );

public slots:
    void onDocumentError( QString, QString, int );
};

#endif // OUTPUTWIDGET_H
