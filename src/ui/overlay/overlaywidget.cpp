#include "overlaywidget.h"
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QTimer>

OverlayWidget::OverlayWidget(QWidget *parent)
    : QWidget(parent)
    , m_isDrawing(false)
    , m_sizeLabel(new QLabel(this))
{
    // 添加调试输出
    qDebug() << "OverlayWidget created";
    
    // 修改窗口标志
    setWindowFlags(Qt::FramelessWindowHint | 
                  Qt::WindowStaysOnTopHint | 
                  Qt::Tool |
                  Qt::NoDropShadowWindowHint);  // 添加这个标志
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_ShowWithoutActivating, false);  // 确保窗口可以被激活
    setAttribute(Qt::WA_X11DoNotAcceptFocus, false);   // 确保窗口可以接受焦点
    setFocusPolicy(Qt::StrongFocus);                   // 设置强焦点策略
    
    // 获取所有屏幕的总区域
    QRect screenRect;
    for (QScreen *screen : QGuiApplication::screens()) {
        screenRect = screenRect.united(screen->geometry());
    }
    setGeometry(screenRect);
    
    // 设置尺寸标签样式
    m_sizeLabel->setStyleSheet(
        "QLabel { "
        "   color: white; "
        "   background-color: #1a1a1a; "
        "   padding: 4px 8px; "
        "   border-radius: 4px; "
        "   font-size: 12px; "
        "}"
    );
    m_sizeLabel->hide();
}

void OverlayWidget::updateSizeInfo()
{
    if (!m_isDrawing) {
        m_sizeLabel->hide();
        return;
    }
    
    QRect rect = QRect(m_startPos, m_endPos).normalized();
    m_sizeLabel->setText(QString("%1 × %2").arg(rect.width()).arg(rect.height()));
    m_sizeLabel->adjustSize(); // 确保标签大小正确
    
    // 智能调整标签位置，避免超出屏幕
    QPoint labelPos;
    const int MARGIN = 5;
    
    // 计算各个方向的可用空间
    int spaceAbove = rect.top();
    int spaceBelow = this->height() - rect.bottom();
    int spaceLeft = rect.left();
    int spaceRight = this->width() - rect.right();
    
    // 优先选择空间最大的位置
    if (spaceBelow >= m_sizeLabel->height() + MARGIN) {
        // 下方空间足够
        labelPos.setY(rect.bottom() + MARGIN);
    } else if (spaceAbove >= m_sizeLabel->height() + MARGIN) {
        // 上方空间足够
        labelPos.setY(rect.top() - m_sizeLabel->height() - MARGIN);
    } else {
        // 上下都不够，显示在选区内部底部
        labelPos.setY(rect.bottom() - m_sizeLabel->height() - MARGIN);
    }
    
    if (spaceRight >= m_sizeLabel->width() + MARGIN) {
        // 右边空间足够
        labelPos.setX(rect.left());
    } else if (spaceLeft >= m_sizeLabel->width() + MARGIN) {
        // 左边空间足够
        labelPos.setX(rect.right() - m_sizeLabel->width());
    } else {
        // 左右都不够，显示在选区内部右侧
        labelPos.setX(rect.right() - m_sizeLabel->width() - MARGIN);
    }
    
    m_sizeLabel->move(labelPos);
    m_sizeLabel->show();
    m_sizeLabel->raise();
}

void OverlayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    
    // 绘制半透明背景
    painter.fillRect(rect(), QColor(0, 0, 0, 128));
    
    // 如果正在绘制选区
    if (m_isDrawing) {
        QRect selectedRect = QRect(m_startPos, m_endPos).normalized();
        // 清除选区的半透明背景
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.fillRect(selectedRect, Qt::transparent);
        
        // 绘制选区边框
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRect(selectedRect);
    }
}

void OverlayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDrawing = true;
        m_startPos = event->pos();
        m_endPos = m_startPos;
        updateSizeInfo();
    }
}

void OverlayWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDrawing) {
        m_endPos = event->pos();
        updateSizeInfo();
        update();
    }
}

void OverlayWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDrawing) {
        m_isDrawing = false;
        m_endPos = event->pos();
        QRect selectedRect = QRect(m_startPos, m_endPos).normalized();
        m_sizeLabel->hide();
        emit areaSelected(selectedRect);
        hide();
    }
}

void OverlayWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (m_isDrawing) {
            // 如果正在绘制，先取消当前选区
            m_isDrawing = false;
            m_sizeLabel->hide();
            update();
        } else {
            // 如果没有在绘制，则退出截图
            hide();
            emit captureFinished();
        }
        event->accept();
    }
}

void OverlayWidget::show()
{
    QWidget::show();
    setWindowState(Qt::WindowActive);
    raise();
    activateWindow();
    
    // 短暂延迟后设置焦点
    QTimer::singleShot(100, this, [this]() {
        setFocus(Qt::ActiveWindowFocusReason);
        activateWindow();
    });
} 