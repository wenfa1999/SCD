#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QTimer>
#include <QClipboard>
#include <Windows.h>
#include "../core/capture/capturemanager.h"
#include "../ui/overlay/overlaywidget.h"
#include <QSystemTrayIcon>
#include <QMenu>
#include "../ui/floatimage/floatwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void startCapture();
    void handleCapture(const QPixmap &pixmap);
    void onCaptureFinished();
    void createFloatWindow(const QPixmap& pixmap);
    void closeApplication();

private:
    // 使用智能指针管理资源
    QScopedPointer<CaptureManager> m_captureManager;
    QScopedPointer<OverlayWidget> m_overlay;
    void setupHotkeys();

    // 全局快捷键相关
    static HHOOK keyboardHook;
    static MainWindow* instance;
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    
    void setupTrayIcon();
    void createTrayMenu();
    void handleTrayActivated(QSystemTrayIcon::ActivationReason reason);

    QList<FloatWindow*> m_floatWindows;  // 管理所有贴图窗口
    bool m_isClosing{false};  // 添加标志位
};

#endif // MAINWINDOW_H 