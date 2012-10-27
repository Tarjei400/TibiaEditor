#ifndef HEXDOUBLESPINBOX_H
#define HEXDOUBLESPINBOX_H

#include <QtGui/QDoubleSpinBox>

class HexDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    HexDoubleSpinBox( QWidget *parent );

protected:
    QValidator::State validate( QString& text, int& pos ) const;
    double valueFromText( const QString& text ) const;
    QString textFromValue( double value ) const;

private:
    QRegExpValidator *validator;
};

#endif // HEXDOUBLESPINBOX_H
