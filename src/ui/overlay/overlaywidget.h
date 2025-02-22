#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>
#include <QLabel>
#include <QTimer>
#include "../toolbar/editbar.h"

class OverlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OverlayWidget(QWidget *parent = nullptr);
    void show();
    
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
    
    void updateSizeInfo();
    void updateEditBarPosition();
    void handleToolChanged(EditBar::Tool tool);
    void updateCursor(const QPoint &pos);
    void takeScreenshot();
    
signals:
    void areaSelected(const QRect &rect);
    void captureFinished();
};

#endif // OVERLAYWIDGET_H 
