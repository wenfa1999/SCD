#include "overlaywidget.h"
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QTimer>
#include <QApplication>
#include "../toolbar/editbar.h"
#include "../../core/capture/capturemanager.h"

// 在文件开头，类定义之前添加静态成员初始化
QCursor* OverlayWidget::s_customCursor = nullptr;

OverlayWidget::OverlayWidget(QWidget *parent, CaptureManager* manager)
    : QWidget(parent)
    , m_isDrawing(false)
    , m_isDragging(false)
    , m_dragStartPos(QPoint())
    , m_sizeLabel(new QLabel(this))
    , m_editBar(new EditBar(this))
    , m_isAnnotating(false)
    , m_annotationStart(QPoint())
    , m_annotationEnd(QPoint())
    , m_currentTool(CaptureManager::AnnotationType::Rectangle)
    , m_currentColor(Qt::red)
    , m_currentThickness(2)
    , m_currentFilled(false)
    , m_captureManager(manager)
{
    // 添加调试输出
    qDebug() << "OverlayWidget created";
    
    // 修改窗口标志，确保窗口总是在顶层且能接收所有事件
    setWindowFlags(Qt::FramelessWindowHint | 
                  Qt::WindowStaysOnTopHint | 
                  Qt::Tool |
                  Qt::NoDropShadowWindowHint |
                  Qt::BypassWindowManagerHint);  // 使用这个替代 X11BypassWindowManagerHint
    
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    
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
    
    m_editBar->hide();  // 初始时隐藏工具栏
    
    // 连接工具栏信号
    connect(m_editBar, &EditBar::toolChanged, this, &OverlayWidget::handleToolChanged);
    connect(m_editBar, &EditBar::confirmClicked, this, [this]() {
        QRect selectedRect = QRect(m_startPos, m_endPos).normalized();
        emit areaSelected(selectedRect);
        hide();
    });
    connect(m_editBar, &EditBar::cancelClicked, this, [this]() {
        hide();
        emit captureFinished();
    });
    
    // 创建事件过滤器来处理工具栏的鼠标事件
    m_editBar->installEventFilter(this);
    
    // 防止窗口失去焦点
    connect(qApp, &QApplication::applicationStateChanged,
            this, [this](Qt::ApplicationState state) {
        if (state == Qt::ApplicationActive && isVisible()) {
            activateWindow();
            raise();
        }
    });
}

bool OverlayWidget::eventFilter(QObject *watched, QEvent *event)
{
    // 处理工具栏的鼠标事件
    if (watched == m_editBar) {
        switch (event->type()) {
            case QEvent::Enter:
            case QEvent::HoverEnter:
            case QEvent::HoverMove:
                setCursor(Qt::ArrowCursor);
                return true;
            case QEvent::Leave:
            case QEvent::HoverLeave:
                updateCursor(mapFromGlobal(QCursor::pos()));
                return true;
            default:
                break;
        }
    }
    
    // 处理主窗口的事件
    if (watched == this) {
        switch (event->type()) {
            case QEvent::WindowDeactivate:
                if (isVisible()) {
                    activateWindow();
                    raise();
                    return true;
                }
                break;
            default:
                break;
        }
    }
    return QWidget::eventFilter(watched, event);
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

void OverlayWidget::takeScreenshot()
{
    // 获取所有屏幕的总区域
    QRect totalRect;
    for (QScreen *screen : QGuiApplication::screens()) {
        totalRect = totalRect.united(screen->geometry());
    }
    
    // 创建一个完整的截图
    QPixmap fullScreenshot(totalRect.size());
    fullScreenshot.fill(Qt::transparent);
    QPainter painter(&fullScreenshot);
    
    // 截取每个屏幕
    for (QScreen *screen : QGuiApplication::screens()) {
        QPixmap screenShot = screen->grabWindow(0);
        QRect screenRect = screen->geometry();
        painter.drawPixmap(screenRect.translated(-totalRect.topLeft()), screenShot);
    }
    
    m_screenSnapshot = fullScreenshot;
    // 设置到 CaptureManager
    m_captureManager->setOriginalPixmap(fullScreenshot);
}

void OverlayWidget::show()
{
    // 先清理所有资源
    m_screenSnapshot = QPixmap();
    m_captureManager->clearResources();
    
    // 重置所有状态
    m_isDrawing = false;
    m_isDragging = false;
    m_isAnnotating = false;
    m_editBar->hide();
    m_sizeLabel->hide();
    m_startPos = QPoint(-1, -1);
    m_endPos = QPoint(-1, -1);
    
    // 隐藏窗口
    hide();
    
    // 延迟执行新截图
    QTimer::singleShot(100, this, [this]() {
        // 截取新的全屏
        takeScreenshot();
        
        // 显示窗口
        QWidget::show();
        setWindowState(Qt::WindowActive);
        raise();
        activateWindow();
        setFocus(Qt::ActiveWindowFocusReason);
    });
}

void OverlayWidget::hide()
{
    // 清理资源
    m_screenSnapshot = QPixmap();
    m_captureManager->clearResources();
    QWidget::hide();
}

void OverlayWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    
    // 使用高质量渲染
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // 绘制初始截图
    painter.drawPixmap(rect(), m_screenSnapshot);
    
    // 绘制选区外的半透明遮罩
    QRect selectedRect = QRect(m_startPos, m_endPos).normalized();
    if (selectedRect.isValid() && selectedRect.width() > 0 && selectedRect.height() > 0) {
        // 绘制四个遮罩区域
        painter.fillRect(QRect(0, 0, width(), selectedRect.top()), QColor(0, 0, 0, 128));  // 上
        painter.fillRect(QRect(0, selectedRect.bottom() + 1, width(), height() - selectedRect.bottom() - 1), QColor(0, 0, 0, 128));  // 下
        painter.fillRect(QRect(0, selectedRect.top(), selectedRect.left(), selectedRect.height()), QColor(0, 0, 0, 128));  // 左
        painter.fillRect(QRect(selectedRect.right() + 1, selectedRect.top(), width() - selectedRect.right() - 1, selectedRect.height()), QColor(0, 0, 0, 128));  // 右
        
        // 绘制选区边框
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRect(selectedRect);
    } else {
        // 如果没有选区，整个屏幕都是半透明的
        painter.fillRect(rect(), QColor(0, 0, 0, 128));
    }

    // 绘制已保存的标注
    if (!m_isDrawing && !m_isDragging) {
        QRect selectedRect = QRect(m_startPos, m_endPos).normalized();
        if (selectedRect.isValid()) {
            // 获取并绘制带标注的图片
            QPixmap editedPixmap = m_captureManager->getEditedPixmap();
            if (!editedPixmap.isNull()) {
                painter.drawPixmap(selectedRect, editedPixmap, selectedRect);
            }
        }
    }

    // 绘制正在创建的标注预览
    if (m_isAnnotating) {
        QPen pen(m_currentColor);
        pen.setWidth(m_currentThickness);
        painter.setPen(pen);
        
        QRect annotationRect = QRect(m_annotationStart, m_annotationEnd).normalized();
        
        switch (m_currentTool) {
            case CaptureManager::AnnotationType::Rectangle:
                if (m_currentFilled) {
                    QColor fillColor = m_currentColor;
                    fillColor.setAlpha(40);
                    painter.setBrush(fillColor);
                } else {
                    painter.setBrush(Qt::NoBrush);
                }
                painter.drawRect(annotationRect);
                break;
                
            // 其他工具类型将在后续添加
        }
    }
}

void OverlayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QRect currentRect = QRect(m_startPos, m_endPos).normalized();
        
        if (!m_editBar->isVisible()) {
            // 还没有选区，开始选区
            m_isDrawing = true;
            m_startPos = m_endPos = event->pos();
            updateSizeInfo();
        } else if (m_editBar->currentTool() != EditBar::None && !m_isAnnotating) {
            // 工具栏可见且选择了工具，且不在标注状态时
            if (currentRect.contains(event->pos())) {
                startAnnotation(event->pos());
            } else {
                // 在选区外点击时，重置工具并开始新选区
                m_editBar->resetTool();
                m_isDrawing = true;
                m_startPos = m_endPos = event->pos();
                m_editBar->hide();
                updateSizeInfo();
            }
        } else if (currentRect.contains(event->pos())) {
            // 在选区内点击，且没有选择工具时，开始拖动
            m_isDragging = true;
            m_dragStartPos = event->pos();
            setCursor(Qt::ClosedHandCursor);
        } else {
            // 在选区外点击时开始新选区
            m_isDrawing = true;
            m_startPos = m_endPos = event->pos();
            m_editBar->hide();
            updateSizeInfo();
        }
        update();
    }
}

void OverlayWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isAnnotating && m_editBar->currentTool() != EditBar::None) {
        // 只有在工具被选中时才更新标注
        updateAnnotation(event->pos());
    } else if (m_isDrawing) {
        // 正在绘制新选区
        m_endPos = event->pos();
        updateSizeInfo();
        update();
    } else if (m_isDragging) {
        // 检查选区是否已经占满整个屏幕
        QRect currentRect = QRect(m_startPos, m_endPos).normalized();
        if (currentRect.size() == size()) {
            return;  // 如果选区等于屏幕大小，直接返回不处理拖动
        }

        // 正在拖动选区
        QPoint delta = event->pos() - m_dragStartPos;
        QRect newRect = currentRect.translated(delta);
        
        // 优化边界检查逻辑
        bool canMove = true;
        if (newRect.left() < 0) {
            delta.setX(delta.x() - newRect.left());
            canMove = false;
        }
        if (newRect.top() < 0) {
            delta.setY(delta.y() - newRect.top());
            canMove = false;
        }
        if (newRect.right() > width()) {
            delta.setX(delta.x() - (newRect.right() - width()));
            canMove = false;
        }
        if (newRect.bottom() > height()) {
            delta.setY(delta.y() - (newRect.bottom() - height()));
            canMove = false;
        }
        
        // 应用移动
        m_startPos = m_startPos + delta;
        m_endPos = m_endPos + delta;
        m_dragStartPos = event->pos();
        
        updateSizeInfo();
        updateEditBarPosition();
        update();
    } else {
        // 更新鼠标样式
        QRect currentRect = QRect(m_startPos, m_endPos).normalized();
        if (currentRect.isValid() && currentRect.width() > 0 && currentRect.height() > 0 
            && currentRect.contains(event->pos())) {
            if (m_editBar->currentTool() != EditBar::None) {
                setCursor(Qt::CrossCursor);
            } else if (currentRect.size() == size()) {
                updateCursor(event->pos());
            } else {
                setCursor(Qt::OpenHandCursor);
            }
        } else {
            updateCursor(event->pos());
        }
    }
}

void OverlayWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isAnnotating) {
            finishAnnotation();
        } else if (m_isDrawing) {
            // 完成绘制新选区
            m_isDrawing = false;
            m_endPos = event->pos();
            updateEditBarPosition();
            m_editBar->show();
            m_editBar->raise();
        } else if (m_isDragging) {
            // 完成拖动
            m_isDragging = false;
            updateCursor(event->pos());
        }
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

void OverlayWidget::updateEditBarPosition()
{
    QRect rect = QRect(m_startPos, m_endPos).normalized();
    
    // 计算各个方向的可用空间
    int spaceAbove = rect.top();
    int spaceBelow = this->height() - rect.bottom();
    
    // 默认显示在下方
    QPoint barPos(rect.center().x() - m_editBar->width() / 2,
                 rect.bottom() + 10);
    
    // 如果上下都不够空间，显示在选区内部底部
    if (spaceBelow < m_editBar->height() + 10 && 
        spaceAbove < m_editBar->height() + 10) {
        barPos.setY(rect.bottom() - m_editBar->height() - 10);
    }
    // 如果下方空间不够但上方够，显示在上方
    else if (spaceBelow < m_editBar->height() + 10) {
        barPos.setY(rect.top() - m_editBar->height() - 10);
    }
    
    // 水平方向的调整保持不变...
    if (barPos.x() + m_editBar->width() > width()) {
        barPos.setX(width() - m_editBar->width() - 10);
    }
    if (barPos.x() < 10) {
        barPos.setX(10);
    }
    
    m_editBar->move(barPos);
}

void OverlayWidget::handleToolChanged(EditBar::Tool tool)
{
    switch (tool) {
        case EditBar::Rectangle:
            m_currentTool = CaptureManager::AnnotationType::Rectangle;
            break;
        case EditBar::Arrow:
            m_currentTool = CaptureManager::AnnotationType::Arrow;
            break;
        case EditBar::Text:
            m_currentTool = CaptureManager::AnnotationType::Text;
            break;
        default:
            m_currentTool = CaptureManager::AnnotationType::Rectangle;
            break;
    }
}

// 新增：更新鼠标样式的辅助函数
void OverlayWidget::updateCursor(const QPoint &pos)
{
    if (!isVisible()) return;
    
    // 检查是否在工具栏区域
    if (m_editBar->isVisible()) {
        QPoint localPos = m_editBar->mapFromParent(pos);
        if (m_editBar->rect().contains(localPos)) {
            setCursor(Qt::ArrowCursor);
            return;
        }
    }
    
    // 检查是否在选区内
    QRect currentRect = QRect(m_startPos, m_endPos).normalized();
    if (currentRect.isValid() && currentRect.width() > 0 && currentRect.height() > 0 
        && currentRect.contains(pos)) {
        // 根据当前工具状态设置光标
        if (m_editBar->currentTool() != EditBar::None) {
            setCursor(Qt::CrossCursor);  // 绘制工具时使用十字光标
        } else {
            setCursor(Qt::OpenHandCursor);  // 无工具时使用手形光标
        }
        return;
    }
    
    // 在其他区域显示自定义十字光标
    if (!s_customCursor) {
        QPixmap pixmap(32, 32);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        
        // 绘制黑色外边
        painter.setPen(QPen(QColor(0, 0, 0), 3));
        painter.drawLine(16, 0, 16, 32);
        painter.drawLine(0, 16, 32, 16);
        
        // 绘制白色内边
        painter.setPen(QPen(QColor(255, 255, 255), 1));
        painter.drawLine(16, 0, 16, 32);
        painter.drawLine(0, 16, 32, 16);
        
        s_customCursor = new QCursor(pixmap, 16, 16);
    }
    setCursor(*s_customCursor);
}

void OverlayWidget::startAnnotation(const QPoint& pos)
{
    m_isAnnotating = true;
    m_annotationStart = m_annotationEnd = pos;
}

void OverlayWidget::updateAnnotation(const QPoint& pos)
{
    if (m_isAnnotating) {
        m_annotationEnd = pos;
        update();
    }
}

void OverlayWidget::finishAnnotation()
{
    if (m_isAnnotating) {
        QRect annotationRect = QRect(m_annotationStart, m_annotationEnd).normalized();
        QRect selectedRect = QRect(m_startPos, m_endPos).normalized();
        
        if (selectedRect.contains(annotationRect)) {
            // 只有当标注区域有效时才添加标注
            if (annotationRect.width() > 0 && annotationRect.height() > 0) {
                CaptureManager::Annotation annotation;
                annotation.type = m_currentTool;
                annotation.rect = annotationRect;
                annotation.color = m_currentColor;
                annotation.thickness = m_currentThickness;
                annotation.filled = m_currentFilled;
                
                m_captureManager->addAnnotation(annotation);
                // 直接更新背景截图
                m_screenSnapshot = m_captureManager->getEditedPixmap();
            }
        }
        
        m_isAnnotating = false;
        m_editBar->resetTool();
        update();
    }
}

OverlayWidget::~OverlayWidget()
{
    delete s_customCursor;
    s_customCursor = nullptr;
} 
