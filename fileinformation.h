#ifndef FILEINFORMATION_H
#define FILEINFORMATION_H

#include <QtGui/QDialog>
#include "ui_fileinformation.h"

class TibiaHandler;

class FileInformation : public QDialog
{
    Q_OBJECT

public:
    FileInformation( QWidget *parent = 0 );
    virtual ~FileInformation( void );

    void updateInformation( void );

private:
    Ui::FileInformationClass *ui;

private slots:
    void onButtonClicked( QAbstractButton * );
};

#endif // FILEINFORMATION_H
