#ifndef QTWIN_H
#define QTWIN_H

#include <QtGui/QColor>
#include <QtGui/QWidget>
/**
  * This is a helper class for using the Desktop Window Manager
  * functionality on Windows 7 and Windows Vista. On other platforms
  * these functions will simply not do anything.
  */

class WindowNotifier;

class QtWin
{
public:
    static bool enableBlurBehindWindow(QWidget *widget, bool enable = true);
    static bool extendFrameIntoClientArea(QWidget *widget,
                                          int left = -1, int top = -1,
                                          int right = -1, int bottom = -1);
    static bool isCompositionEnabled();
    static QColor colorizatinColor();

private:
    static WindowNotifier *windowNotifier();
};

#endif // QTWIN_H
