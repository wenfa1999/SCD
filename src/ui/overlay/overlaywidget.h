#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QTimer>

class OverlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OverlayWidget(QWidget *parent = nullptr);
    void show(); // 添加 show 函数声明
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    
private:
    QPoint m_startPos;
    QPoint m_endPos;
    bool m_isDrawing;
    QLabel *m_sizeLabel;  // 尺寸信息标签
    
    // 使用QImage代替QPixmap进行绘制操作，因为QImage是CPU端数据
    // 而QPixmap是GPU端数据，频繁的CPU-GPU数据传输会影响性能
    QImage m_backgroundCache;
    void updateBackgroundCache();
    
    // 使用双缓冲绘制
    QPixmap m_doubleBuffer;
    
    // 更新尺寸信息显示
    void updateSizeInfo();
    
signals:
    void areaSelected(const QRect &rect);
    void captureFinished();  // 添加新信号
};

#endif // OVERLAYWIDGET_H 