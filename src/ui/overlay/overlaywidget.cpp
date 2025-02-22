#include "overlaywidget.h"
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QKeyEvent>

OverlayWidget::OverlayWidget(QWidget *parent)
    : QWidget(parent)
    , m_isDrawing(false)
    , m_sizeLabel(new QLabel(this))
{
    // 添加调试输出
    qDebug() << "OverlayWidget created";
    
    // 设置窗口标志
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    
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
    
    // 智能调整标签位置，避免超出屏幕
    QPoint labelPos;
    const int MARGIN = 5;
    
    // 优先显示在选区下方
    labelPos = QPoint(rect.left(), rect.bottom() + MARGIN);
    
    // 如果标签超出屏幕底部，则显示在选区上方
    if (labelPos.y() + m_sizeLabel->height() > this->height()) {
        labelPos.setY(rect.top() - m_sizeLabel->height() - MARGIN);
    }
    
    // 如果标签超出屏幕右边，则向左偏移
    if (labelPos.x() + m_sizeLabel->width() > this->width()) {
        labelPos.setX(this->width() - m_sizeLabel->width() - MARGIN);
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
            emit captureFinished(); // 新增信号
        }
    }
} 