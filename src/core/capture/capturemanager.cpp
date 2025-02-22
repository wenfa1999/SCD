#include "capturemanager.h"
#include <QScreen>
#include <QGuiApplication>
#include <QWindow>
#include <QDebug>
#include <QPainter>

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
            
        case AnnotationType::Arrow:
            // 箭头实现将在后续添加
            break;
            
        case AnnotationType::Text:
            if (!annotation.text.isEmpty()) {
                painter.drawText(annotation.rect, 
                               Qt::AlignLeft | Qt::AlignTop, 
                               annotation.text);
            }
            break;
    }
} 