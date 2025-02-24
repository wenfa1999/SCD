#ifndef FLOATWINDOW_H
#define FLOATWINDOW_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QContextMenuEvent>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>

class FloatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FloatWindow(const QPixmap& pixmap, QWidget* parent = nullptr);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    
private:
    QPixmap m_pixmap;
    bool m_isDragging{false};
    QPoint m_dragStartPos;
    QMenu* m_contextMenu;
    
    void createContextMenu();
    void saveImage();
};

#endif // FLOATWINDOW_H 