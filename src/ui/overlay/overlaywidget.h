#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>
#include <QLabel>
#include <QTimer>
#include "../toolbar/editbar.h"
#include "../../core/capture/capturemanager.h"

class OverlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OverlayWidget(QWidget *parent = nullptr, CaptureManager* manager = nullptr);
    ~OverlayWidget();
    void show();
    void hide();
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    
private:
    QPoint m_startPos;
    QPoint m_endPos;
    bool m_isDrawing;
    bool m_isDragging;
    QPoint m_dragStartPos;
    QLabel *m_sizeLabel;
    EditBar *m_editBar;
    QPixmap m_screenSnapshot;
    bool m_isAnnotating{false};  // 是否正在绘制标注
    QPoint m_annotationStart;    // 标注起始点
    QPoint m_annotationEnd;      // 标注结束点
    CaptureManager::AnnotationType m_currentTool{CaptureManager::AnnotationType::Rectangle};
    QColor m_currentColor{Qt::red};  // 当前标注颜色
    int m_currentThickness{2};       // 当前线条粗细
    bool m_currentFilled{false};     // 当前是否填充
    CaptureManager* m_captureManager;  // 添加成员变量
    static QCursor* s_customCursor;  // 添加静态成员声明
    
    void updateSizeInfo();
    void updateEditBarPosition();
    void handleToolChanged(EditBar::Tool tool);
    void updateCursor(const QPoint &pos);
    void takeScreenshot();
    void startAnnotation(const QPoint& pos);
    void updateAnnotation(const QPoint& pos);
    void finishAnnotation();
    
signals:
    void areaSelected(const QRect &rect);
    void captureFinished();
    void createFloatWindow(const QPixmap& pixmap);
};

#endif // OVERLAYWIDGET_H 
