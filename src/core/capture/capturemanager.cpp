#include "capturemanager.h"
#include <QScreen>
#include <QGuiApplication>
#include <QWindow>
#include <QDebug>
#include <QPainter>
#include <cmath>

CaptureManager::CaptureManager(QObject *parent)
    : QObject(parent)
    , m_lastCapture()
{
}

void CaptureManager::startCapture()
{
    // 获取主屏幕
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return;
    }

    // 捕获整个屏幕
    m_lastCapture = screen->grabWindow(0);
    emit captureTaken(m_lastCapture);
}

QPixmap CaptureManager::captureScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        qDebug() << "Failed to get primary screen";
        return QPixmap();
    }
    QPixmap screenshot = screen->grabWindow(0);
    qDebug() << "Screenshot taken, size:" << screenshot.size();
    return screenshot;
}

QPixmap CaptureManager::captureWindow(WId windowId)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return QPixmap();
    }
    return screen->grabWindow(windowId);
}

void CaptureManager::addAnnotation(const Annotation& annotation)
{
    m_annotations.append(annotation);
}

void CaptureManager::removeLastAnnotation()
{
    if (!m_annotations.isEmpty()) {
        m_annotations.removeLast();
    }
}

void CaptureManager::clearAnnotations()
{
    m_annotations.clear();
}

QPixmap CaptureManager::getEditedPixmap() const
{
    if (m_lastCapture.isNull()) {
        return QPixmap();
    }

    // 创建副本以保持原始截图不变
    QPixmap result = m_lastCapture;
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制所有标注
    for (const auto& annotation : m_annotations) {
        drawAnnotation(painter, annotation);
    }

    return result;
}

void CaptureManager::drawAnnotation(QPainter& painter, const Annotation& annotation) const
{
    QPen pen(annotation.color);
    pen.setWidth(annotation.thickness);
    painter.setPen(pen);

    switch (annotation.type) {
        case AnnotationType::Rectangle:
            if (annotation.filled) {
                QColor fillColor = annotation.color;
                fillColor.setAlpha(40);  // 半透明填充
                painter.setBrush(fillColor);
            } else {
                painter.setBrush(Qt::NoBrush);
            }
            painter.drawRect(annotation.rect);
            break;
            
        case AnnotationType::Arrow: {
            QLineF line(annotation.startPoint, annotation.endPoint);
            
            // 箭头参数
            double arrowLength = 20.0;
            double arrowWidth = 8.0;  // 箭头宽度的一半
            
            // 计算箭头方向向量并归一化
            QPointF direction = line.p2() - line.p1();
            double length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());
            if (length < 1e-6) return;  // 避免除以零
            direction /= length;
            
            // 计算垂直向量
            QPointF normal(-direction.y(), direction.x());
            
            // 计算箭头底部中心点（从箭头尖端往回移动）
            QPointF arrowBase = line.p2() - direction * arrowLength;
            
            // 计算箭头的三个点
            QPointF arrowTip = line.p2();  // 箭头尖端
            QPointF arrowLeft = arrowBase + normal * arrowWidth;
            QPointF arrowRight = arrowBase - normal * arrowWidth;
            
            // 绘制箭头主体（从起点到箭头底部中心）
            painter.drawLine(line.p1(), arrowBase);
            
            // 绘制箭头头部
            QPolygonF arrowHead;
            arrowHead << arrowTip << arrowLeft << arrowRight;
            painter.setBrush(annotation.color);
            painter.drawPolygon(arrowHead);
            break;
        }
            
        case AnnotationType::Text:
            if (!annotation.text.isEmpty()) {
                painter.drawText(annotation.rect, 
                               Qt::AlignLeft | Qt::AlignTop, 
                               annotation.text);
            }
            break;
    }
} 