#include "floatwindow.h"
#include <QPainter>
#include <QFileDialog>

FloatWindow::FloatWindow(const QPixmap& pixmap, QWidget* parent)
    : QWidget(parent)
    , m_pixmap(pixmap)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(pixmap.size());
    createContextMenu();
    
    // 允许鼠标追踪
    setMouseTracking(true);
}

void FloatWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(rect(), m_pixmap);
}

void FloatWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        m_dragStartPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void FloatWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging) {
        move(pos() + event->pos() - m_dragStartPos);
    }
}

void FloatWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void FloatWindow::contextMenuEvent(QContextMenuEvent* event)
{
    m_contextMenu->exec(event->globalPos());
}

void FloatWindow::createContextMenu()
{
    m_contextMenu = new QMenu(this);
    
    QAction* copyAction = new QAction("复制", this);
    connect(copyAction, &QAction::triggered, [this]() {
        QApplication::clipboard()->setPixmap(m_pixmap);
    });
    
    QAction* saveAction = new QAction("保存", this);
    connect(saveAction, &QAction::triggered, this, &FloatWindow::saveImage);
    
    QAction* closeAction = new QAction("关闭", this);
    connect(closeAction, &QAction::triggered, this, &QWidget::close);
    
    m_contextMenu->addAction(copyAction);
    m_contextMenu->addAction(saveAction);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction(closeAction);
}

void FloatWindow::saveImage()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "保存图片",
        QDir::homePath() + "/screenshot.png",
        "Images (*.png *.jpg *.bmp)"
    );
    
    if (!filePath.isEmpty()) {
        m_pixmap.save(filePath);
    }
} 