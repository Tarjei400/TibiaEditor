#include <QtCore/QTime>
#include <QtGui/QScrollBar>
#include "outputwidget.h"

OutputWidget::OutputWidget( QWidget *parent ) : QTextEdit( parent )
{
    setWordWrapMode( QTextOption::NoWrap );
    setReadOnly( true );
    setAcceptRichText( true );
}

void OutputWidget::addLine( const QColor& color, const QString& string )
{
    moveCursor( QTextCursor::End );
    insertHtml( QString( "<span style=\"color:%1;\">[%2] %3</span>" ).arg( color.name() ).arg( QTime::currentTime().toString() ).arg( string ) );
    insertPlainText( "\n" );
    verticalScrollBar()->setValue( verticalScrollBar()->maximum() );
}

void OutputWidget::onDocumentError( QString fileName, QString errorType, int line )
{
    addLine( QColor( Qt::red ), QString( "%1: %2" ).arg( fileName ).arg( errorType ) );
}
