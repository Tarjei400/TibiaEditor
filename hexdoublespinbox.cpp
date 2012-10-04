#include "hexdoublespinbox.h"

HexDoubleSpinBox::HexDoubleSpinBox( QWidget *parent ) : QDoubleSpinBox( parent )
{
    setRange( 0, 4294967295 );
    validator = new QRegExpValidator( QRegExp( "[0-9A-Fa-f]{1,8}" ), this );
}

QValidator::State HexDoubleSpinBox::validate( QString& text, int& pos ) const
{
    return validator->validate(text, pos);
}

QString HexDoubleSpinBox::textFromValue( double value ) const
{
    return QString::number( ( quint64 )value, 16 ).toUpper();
}

double HexDoubleSpinBox::valueFromText( const QString& text ) const
{
    bool ok;
    return ( double )text.toInt( &ok, 16 );
}
