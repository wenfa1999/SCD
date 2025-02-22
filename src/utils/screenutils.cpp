#include "screenutils.h"
#include <QGuiApplication>
#include <QScreen>
#include <QWindow>

QList<QScreen*> ScreenUtils::getAllScreens()
{
    return QGuiApplication::screens();
}

QScreen* ScreenUtils::getScreenAt(const QPoint &pos)
{
    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen->geometry().contains(pos)) {
            return screen;
        }
    }
    return nullptr;
}

QRect ScreenUtils::getVirtualScreenGeometry()
{
    QRect rect;
    for (QScreen *screen : QGuiApplication::screens()) {
        rect = rect.united(screen->geometry());
    }
    return rect;
}

WId ScreenUtils::getWindowAt(const QPoint &pos)
{
    // 注意：这个功能在不同平台上的实现可能不同
    // 这里需要使用平台特定的API来实现
    // Windows可以使用WindowsAPI，Linux可以使用X11/Wayland API
    return 0; // 暂时返回0，需要根据平台实现具体逻辑
} 