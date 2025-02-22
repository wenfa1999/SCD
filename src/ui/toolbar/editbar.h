#ifndef EDITBAR_H
#define EDITBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QToolButton>

class EditBar : public QWidget
{
    Q_OBJECT
public:
    enum Tool {
        Rectangle,   // 矩形标注
        Arrow,      // 箭头标注
        Text,       // 文字标注
        None        // 无工具选中
    };
    Q_ENUM(Tool)

    explicit EditBar(QWidget *parent = nullptr);
    Tool currentTool() const { return m_currentTool; }
    void show();

signals:
    void toolChanged(Tool tool);
    void confirmClicked();
    void cancelClicked();

private:
    Tool m_currentTool{None};
    QToolButton* createToolButton(const QString &iconPath, 
                                const QString &tooltip,
                                Tool tool);
    void setupUI();
};

#endif // EDITBAR_H 