#ifndef CAPTUREMANAGER_H
#define CAPTUREMANAGER_H

#include <QObject>
#include <QScreen>
#include <QPixmap>
#include <QPoint>
#include <QVector>
#include <QPainter>
#include <QMutex>
#include <QMutexLocker>
#include <QtMath>  // 添加数学函数支持

class CaptureManager : public QObject
{
    Q_OBJECT
public:
    // 定义标注类型
    enum class AnnotationType {
        Rectangle,
        Arrow,
        Text
    };
    
    // 定义标注项结构
    struct Annotation {
        AnnotationType type;
        QRect rect;           // 标注区域
        QString text;         // 文本内容（用于文字标注）
        QColor color;         // 标注颜色
        int thickness;        // 线条粗细
        bool filled;          // 是否填充（用于矩形）
        QPoint startPoint;    // 添加：箭头起点
        QPoint endPoint;      // 添加：箭头终点
    };

public:
    explicit CaptureManager(QObject *parent = nullptr);
    
    void startCapture();
    QPixmap captureScreen();
    QPixmap captureWindow(WId windowId);
    
    // 使用右值引用优化性能
    void handleCapture(QPixmap &&pixmap);
    
    // 添加缓存机制
    QPixmap getCachedScreen() const;
    
    // 新增编辑相关方法
    void addAnnotation(const Annotation& annotation);
    void removeLastAnnotation();
    void clearAnnotations();
    QPixmap getEditedPixmap() const;  // 获取带有标注的图片
    
    void setOriginalPixmap(const QPixmap& pixmap) { m_lastCapture = pixmap; }
    const QPixmap& currentPixmap() const { return m_lastCapture; }
    
    void clearResources()
    {
        QMutexLocker locker(&m_mutex);  // 添加互斥锁保护
        m_lastCapture = QPixmap();
        m_annotations.clear();
    }
    
signals:
    void captureTaken(const QPixmap &pixmap);
    void captureFinished();
    
private:
    QPixmap m_lastCapture;
    QScreen *m_primaryScreen{nullptr}; // 缓存主屏幕指针
    
    // 预分配内存，避免频繁分配
    QVector<QScreen*> m_screens;
    QVector<Annotation> m_annotations;  // 存储所有标注
    void updateScreenCache();
    void drawAnnotation(QPainter& painter, const Annotation& annotation) const;
    QMutex m_mutex;  // 添加互斥锁
};

#endif // CAPTUREMANAGER_H 