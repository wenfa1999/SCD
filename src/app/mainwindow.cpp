#include "mainwindow.h"
#include <QShortcut>
#include <QKeySequence>
#include <QApplication>
#include <QTimer>
#include <QLabel>
#include <QScreen>
#include <QWindow>
#include <Windows.h> // Windows平台

// 初始化静态成员
HHOOK MainWindow::keyboardHook = nullptr;
MainWindow* MainWindow::instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_captureManager(new CaptureManager(this))
    , m_overlay(new OverlayWidget(nullptr)) // 设置为nullptr使其成为顶级窗口
{
    // 添加这行，设置一个合适的初始大小
    resize(800, 600);
    
    // 添加一个标签显示使用说明
    QLabel *label = new QLabel(this);
    label->setText("按 Ctrl+Alt+A 开始截图");
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
    
    setupHotkeys();
    
    // 修改connect的使用方式，使用.data()获取原始指针
    connect(m_overlay.data(), &OverlayWidget::areaSelected, this, [this](const QRect &rect) {
        QPixmap screenshot = m_captureManager->captureScreen();
        QPixmap croppedShot = screenshot.copy(rect);
        handleCapture(croppedShot);
    });
    
    // 连接截图完成信号
    connect(m_overlay.data(), &OverlayWidget::captureFinished, 
            this, &MainWindow::onCaptureFinished);
}

MainWindow::~MainWindow()
{
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
    }
}

void MainWindow::setupHotkeys()
{
    instance = this;
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 
                                   GetModuleHandle(nullptr), 0);
}

LRESULT CALLBACK MainWindow::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        bool ctrlPressed = GetAsyncKeyState(VK_CONTROL) & 0x8000;
        bool altPressed = GetAsyncKeyState(VK_MENU) & 0x8000;
        
        if (ctrlPressed && altPressed && kbStruct->vkCode == 'A') {
            if (instance) {
                QMetaObject::invokeMethod(instance, "startCapture", 
                                        Qt::QueuedConnection);
                return 1;
            }
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

void MainWindow::startCapture()
{
    // 隐藏主窗口
    hide();
    
    // 短暂延迟以确保窗口完全隐藏
    QTimer::singleShot(200, this, [this]() {
        m_overlay->show();
    });
}

void MainWindow::handleCapture(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        return;
    }
    
    // 这里可以添加后续处理逻辑
    // 例如：保存到剪贴板、显示编辑界面等
    QApplication::clipboard()->setPixmap(pixmap);
    
    // 显示主窗口
    show();
}

void MainWindow::onCaptureFinished()
{
    show();
}
