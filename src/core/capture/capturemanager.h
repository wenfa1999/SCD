#ifndef CAPTUREMANAGER_H
#define CAPTUREMANAGER_H

#include <QObject>
#include <QScreen>
#include <QPixmap>
#include <QPoint>
#include <QVector>

class CaptureManager : public QObject
{
    Q_OBJECT
public:
    explicit CaptureManager(QObject *parent = nullptr);
    
    void startCapture();
    QPixmap captureScreen();
    QPixmap captureWindow(WId windowId);
    
    // 使用右值引用优化性能
    void handleCapture(QPixmap &&pixmap);
    
    // 添加缓存机制
    QPixmap getCachedScreen() const;
    
signals:
    void captureTaken(const QPixmap &pixmap);
    void captureFinished();
    
private:
    QPixmap m_lastCapture;
    QScreen *m_primaryScreen{nullptr}; // 缓存主屏幕指针
    
    // 预分配内存，避免频繁分配
    QVector<QScreen*> m_screens;
    void updateScreenCache();
};

#endif // CAPTUREMANAGER_H 