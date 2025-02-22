#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QTimer>
#include <QClipboard>
#include "../core/capture/capturemanager.h"
#include "../ui/overlay/overlaywidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startCapture();
    void handleCapture(const QPixmap &pixmap);
    void onCaptureFinished();

private:
    // 使用智能指针管理资源
    QScopedPointer<CaptureManager> m_captureManager;
    QScopedPointer<OverlayWidget> m_overlay;
    void setupHotkeys();
};

#endif // MAINWINDOW_H 