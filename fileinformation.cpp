#include <QtGui/QPushButton>
#include "fileinformation.h"
#include "tibiahandler.h"

extern TibiaHandler g_tibiaHandler;

FileInformation::FileInformation( QWidget *parent ) : QDialog( parent ), ui( new Ui::FileInformationClass )
{
    ui->setupUi(this);
    ui->buttonBox->button( QDialogButtonBox::Apply )->setIcon( QIcon( ":/TibiaEditor/Resources/apply.png" ) );

    updateInformation();

    QObject::connect( ui->buttonBox, SIGNAL( clicked( QAbstractButton * ) ), this, SLOT( onButtonClicked( QAbstractButton * ) ) );
}

FileInformation::~FileInformation( void )
{
    delete ui;
}

void FileInformation::updateInformation( void )
{
    ItemFile *itemFile = g_tibiaHandler.getItemFile();
    if( itemFile && itemFile->isLoaded() ) {
        ui->editItemSignature->setValue( (double)itemFile->getSignature() );
        ui->editItems->setText( QString::number( itemFile->getItemCount() ) );
        ui->editOutfits->setText( QString::number( itemFile->getOutfitCount() ) );
        ui->editEffects->setText( QString::number( itemFile->getEffectCount() ) );
        ui->editProjectiles->setText( QString::number( itemFile->getProjectileCount() ) );
    } else
        ui->itemGroup->setShown( false );

    SpriteFile *spriteFile = g_tibiaHandler.getSpriteFile();
    if( spriteFile && spriteFile->isLoaded() ) {
        ui->editSpriteSignature->setValue( (double)spriteFile->getSignature() );
        ui->editSprites->setText( QString::number( spriteFile->getCount()-1 ) );
    } else
        ui->spriteGroup->setShown( false );

    PictureFile *pictureFile = g_tibiaHandler.getPictureFile();
    if( pictureFile && pictureFile->isLoaded() ) {
        ui->editPictureSignature->setValue( (double)pictureFile->getSignature() );
        ui->editPictures->setText( QString::number( pictureFile->getCount() ) );
    } else
        ui->pictureGroup->setShown( false );
}

void FileInformation::onButtonClicked( QAbstractButton *button )
{
    switch( ui->buttonBox->buttonRole( button ) ) {
    case QDialogButtonBox::ApplyRole: {
        ItemFile *itemFile = g_tibiaHandler.getItemFile();
        if( itemFile && itemFile->isLoaded() )
            itemFile->setSignature( (quint32)ui->editItemSignature->value() );

        SpriteFile *spriteFile = g_tibiaHandler.getSpriteFile();
        if( spriteFile && spriteFile->isLoaded() )
            spriteFile->setSignature( (quint32)ui->editSpriteSignature->value() );

        PictureFile *pictureFile = g_tibiaHandler.getPictureFile();
        if( pictureFile && pictureFile->isLoaded() )
            pictureFile->setSignature( (quint32)ui->editPictureSignature->value() );

        g_tibiaHandler.getOutputWidget()->addLine( QColor( Qt::darkGreen ), tr( "File Properties Changed." ) );
        close();
    }
    break;
    case QDialogButtonBox::RejectRole:
        close();
        break;
    case QDialogButtonBox::ResetRole:
        updateInformation();
        break;
    default:
        break;
    }
}
