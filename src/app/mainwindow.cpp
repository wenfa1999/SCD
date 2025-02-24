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
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDir>
#include <QStyle>
#include "../ui/floatimage/floatwindow.h"  // 使用相对路径
#include <QIcon>

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
    setupTrayIcon();
    
    // 修改connect的使用方式，使用.data()获取原始指针
    connect(m_overlay.data(), &OverlayWidget::areaSelected, this, [this](const QRect &rect) {
        QPixmap screenshot = m_captureManager->captureScreen();
        QPixmap croppedShot = screenshot.copy(rect);
        handleCapture(croppedShot);
    });
    
    // 连接截图完成信号
    connect(m_overlay.data(), &OverlayWidget::captureFinished, 
            this, &MainWindow::onCaptureFinished);
    
    // 连接贴图信号
    connect(m_overlay.data(), &OverlayWidget::createFloatWindow,
            this, &MainWindow::createFloatWindow);
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

void MainWindow::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    // 使用资源文件中的图标
    m_trayIcon->setIcon(QIcon(":/icons/app.ico"));
    m_trayIcon->setToolTip("截图工具");
    
    createTrayMenu();
    m_trayIcon->setContextMenu(m_trayMenu);
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, 
            this, &MainWindow::handleTrayActivated);
            
    m_trayIcon->show();
}

void MainWindow::createTrayMenu()
{
    m_trayMenu = new QMenu(this);
    
    QAction* captureAction = new QAction("截图", this);
    connect(captureAction, &QAction::triggered, this, &MainWindow::startCapture);
    
    QAction* showAction = new QAction("显示主窗口", this);
    connect(showAction, &QAction::triggered, this, &MainWindow::show);
    
    QAction* quitAction = new QAction("退出", this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::closeApplication);
    
    m_trayMenu->addAction(captureAction);
    m_trayMenu->addAction(showAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(quitAction);
}

void MainWindow::handleTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        startCapture();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_isClosing && m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::createFloatWindow(const QPixmap& pixmap)
{
    FloatWindow* floatWin = new FloatWindow(pixmap);
    
    // 当窗口关闭时自动删除
    floatWin->setAttribute(Qt::WA_DeleteOnClose);
    
    // 连接关闭信号以从列表中移除
    connect(floatWin, &FloatWindow::destroyed, this, [this, floatWin]() {
        m_floatWindows.removeOne(floatWin);
    });
    
    m_floatWindows.append(floatWin);
    
    // 获取当前选区的位置
    QRect selectedRect = QRect(m_overlay->getStartPos(), m_overlay->getEndPos()).normalized();
    
    // 设置贴图窗口的位置为选区位置
    floatWin->move(selectedRect.topLeft());
    
    floatWin->show();
}

void MainWindow::closeApplication()
{
    m_isClosing = true;  // 设置关闭标志
    
    // 先禁用所有事件处理
    setEnabled(false);
    
    // 断开所有信号连接
    disconnect();
    
    // 清理全局钩子
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = nullptr;
    }

    // 断开所有贴图窗口的信号连接并隐藏
    for (auto* window : m_floatWindows) {
        window->disconnect();
        window->hide();
    }

    // 隐藏托盘图标并断开连接
    if (m_trayIcon) {
        m_trayIcon->disconnect();
        m_trayIcon->hide();
    }

    // 隐藏主窗口
    hide();

    // 使用延迟调用进行清理和退出
    QTimer::singleShot(0, this, [this]() {
        // 清理贴图窗口
        qDeleteAll(m_floatWindows);
        m_floatWindows.clear();

        // 清理托盘图标
        delete m_trayIcon;
        m_trayIcon = nullptr;

        // 退出应用
        qApp->quit();
    });
}
