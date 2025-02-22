#include "capturemanager.h"
#include <QScreen>
#include <QGuiApplication>
#include <QWindow>
#include <QDebug>

CaptureManager::CaptureManager(QObject *parent)
    : QObject(parent)
    , m_lastCapture()
{
}

void CaptureManager::startCapture()
{
    // 获取主屏幕
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return;
    }

    // 捕获整个屏幕
    m_lastCapture = screen->grabWindow(0);
    emit captureTaken(m_lastCapture);
}

QPixmap CaptureManager::captureScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qDebug() << "Failed to get primary screen";
        return QPixmap();
    }
    QPixmap screenshot = screen->grabWindow(0);
    qDebug() << "Screenshot taken, size:" << screenshot.size();
    return screenshot;
}

QPixmap CaptureManager::captureWindow(WId windowId)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return QPixmap();
    }
    return screen->grabWindow(windowId);
} 