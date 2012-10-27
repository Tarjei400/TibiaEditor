#ifndef SPRITEVIEW_H
#define SPRITEVIEW_H

#include <QtGui/QTreeView>

class SpriteView : public QTreeView
{
    Q_OBJECT

public:
    SpriteView( QWidget *parent = NULL ) : QTreeView( parent ) {};

protected:
    virtual void startDrag( Qt::DropActions supportedActions );
};

#endif // ITEMVIEW_H
