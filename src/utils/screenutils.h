#ifndef SCREENUTILS_H
#define SCREENUTILS_H

#include <QScreen>
#include <QWindow>
#include <QApplication>

class ScreenUtils
{
public:
    static QList<QScreen*> getAllScreens();
    static QScreen* getScreenAt(const QPoint &pos);
    static QRect getVirtualScreenGeometry();
    static WId getWindowAt(const QPoint &pos);
};

#endif // SCREENUTILS_H 