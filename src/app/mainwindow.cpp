#include "mainwindow.h"
#include <QShortcut>
#include <QKeySequence>
#include <QApplication>
#include <QTimer>
#include <QLabel>
#include <QScreen>
#include <QWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <Windows.h> // Windows平台

// 初始化静态成员
HHOOK MainWindow::keyboardHook = nullptr;
MainWindow* MainWindow::instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_captureManager(new CaptureManager(this))
    , m_overlay(new OverlayWidget(nullptr, m_captureManager.data())) // 传入 CaptureManager
{
    // 添加这行，设置一个合适的初始大小
    resize(800, 600);
    
    // 创建一个中央部件
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    
    // 添加说明标签
    QLabel *label = new QLabel("按 Ctrl+Alt+A 开始截图", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    
    // 添加测试按钮
    QPushButton *captureButton = new QPushButton("开始截图", this);
    connect(captureButton, &QPushButton::clicked, this, &MainWindow::startCapture);
    layout->addWidget(captureButton);
    
    setCentralWidget(centralWidget);
    
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
    // 先清理资源
    m_captureManager->clearResources();
    
    // 隐藏主窗口
    hide();
    
    // 延迟显示截图界面
    QTimer::singleShot(200, this, [this]() {
        m_overlay->show();
    });
}

void MainWindow::handleCapture(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        return;
    }
    
    QApplication::clipboard()->setPixmap(pixmap);
    m_captureManager->clearResources();
    show();
}

void MainWindow::onCaptureFinished()
{
    m_captureManager->clearResources();
    show();
}
