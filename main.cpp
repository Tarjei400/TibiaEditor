#include <QtGui/QApplication>
//#include <QWindowsMime>
#include "tibiaeditor.h"
#include "resourcehandler.h"
#include "formathandler.h"

/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

#ifndef NO_STARTUP
#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#endif

TibiaHandler g_tibiaHandler;
ResourceHandler g_resourceHandler;
FormatHandler g_formatHandler;

Q_DECLARE_METATYPE( ResourceList )
Q_DECLARE_METATYPE( ItemList )

int main( int argc, char *argv[] )
{
    //_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

    QApplication application( argc, argv );
    QCoreApplication::setApplicationVersion( "6.7" );
    QCoreApplication::setApplicationName( "Tibia Editor" );
    QCoreApplication::setOrganizationName( "Demonic Applications" );

    qRegisterMetaType<ResourceList>( "ResourceList" );
    qRegisterMetaType<ItemList>( "ItemList" );

#ifndef NO_STARTUP
    QDialog *dialog = new QDialog( NULL );
    dialog->setWindowFlags( Qt::Tool );
    if( dialog->objectName().isEmpty() )
        dialog->setObjectName( QString::fromUtf8( "Dialog" ) );
    dialog->resize( 250, 250 );
    dialog->setMinimumSize( QSize( 250, 250 ) );
    dialog->setMaximumSize( QSize( 250, 250 ) );
    dialog->setStyleSheet(QString::fromUtf8("QDialog { background-image: url(:/TibiaEditor/Resources/SplashScreen.png); }"));
    QVBoxLayout *verticalLayout = new QVBoxLayout( dialog );
    verticalLayout->setSpacing( 5 );
    verticalLayout->setMargin( 5 );
    verticalLayout->setObjectName( QString::fromUtf8( "verticalLayout" ) );
    QLabel *label = new QLabel( dialog );
    label->setObjectName( QString::fromUtf8( "label" ) );
    label->setAlignment( Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter );
    label->setWordWrap( true );
    label->setOpenExternalLinks( true );
    verticalLayout->addWidget( label );
    dialog->setWindowTitle(QApplication::translate("Dialog", "TE6.7A", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("Dialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                                           "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                                           "p, li { white-space: pre-wrap; }\n"
                                           "</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
                                           "<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"></p>\n"
                                           "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt; color:#00000;\">You are using an Alpha version of TibiaEditor V6.7, functions are limited.</span></p>\n"
                                           "<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt; color:#ffffff;\"></p>\n"
                                           "<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt; color:#ffffff;\"></p>\n"
                                           "<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt; color:#ffffff;\"></p>\n"
                                           "<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt; color:#ffffff;\"></p>\n"
                                           "<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt; color:#ffffff;\"></p>\n"
                                           "<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt; color:#ffffff;\"></p>\n"
                                           "<p align=\"center\" style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt; color:#ffffff;\"></p>\n"
                                           "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt; color:#999999;\">All credits go to BlackDemon. Im going to keep this project alive for some amount of time. Visit forum thread, PM me CONTRIBUTE!</span></p>\n"
                                            "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:10pt; color:#999999;\">Usefull links:</span></p>\n"
                                           "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href=\"http://otland.net/f251/tibia-editor-9-6-discussion-thread-171429/#post1660387\"><span style=\" font-size:18pt; text-decoration: underline; color:#ff0004;\">FORUM</span></a><a href=\"https://github.com/otfallen/TibiaEditor\"><span style=\" font-size:18pt; text-decoration: underline; color:#ff00ff;\"> CONTRIBUTE</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    dialog->exec();
#endif

    TibiaEditor window;
    QObject::connect( &application, SIGNAL( focusChanged( QWidget *, QWidget * ) ), &window, SLOT( onFocusChanged( QWidget *, QWidget * ) ) );
    window.show();
    return application.exec();
}
