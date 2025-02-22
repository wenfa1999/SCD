#include "editbar.h"
#include <QHBoxLayout>
#include <QIcon>
#include <QStyle>
#include <QFrame>

EditBar::EditBar(QWidget *parent)
    : QWidget(parent)
    , m_currentTool(None)
{
    setObjectName("EditBar");
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet(
        "QWidget#EditBar {"
        "   background-color: #2D2D2D;"
        "   border-radius: 4px;"
        "   padding: 4px;"
        "}"
        "QToolButton {"
        "   border: none;"
        "   border-radius: 3px;"
        "   padding: 4px;"
        "   background-color: transparent;"
        "}"
        "QToolButton:hover {"
        "   background-color: #3D3D3D;"
        "}"
        "QToolButton:checked {"
        "   background-color: #4D4D4D;"
        "}"
    );
    
    setupUI();
}

void EditBar::setupUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(4, 4, 4, 4);
    
    // 矩形工具
    layout->addWidget(createToolButton(":/icons/rect.png", "矩形标注", Rectangle));
    
    // 箭头工具
    layout->addWidget(createToolButton(":/icons/arrow.png", "箭头标注", Arrow));
    
    // 文字工具
    layout->addWidget(createToolButton(":/icons/text.png", "文字标注", Text));
    
    // 贴图工具
    layout->addWidget(createToolButton(":/icons/pin.png", "贴图", Pin));
    
    // 添加分隔线
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    
    // 确认按钮
    QToolButton* confirmBtn = createToolButton(":/icons/confirm.png", "确认", None);
    connect(confirmBtn, &QToolButton::clicked, this, &EditBar::confirmClicked);
    layout->addWidget(confirmBtn);
    
    // 取消按钮
    QToolButton* cancelBtn = createToolButton(":/icons/cancel.png", "取消", None);
    connect(cancelBtn, &QToolButton::clicked, this, &EditBar::cancelClicked);
    layout->addWidget(cancelBtn);
}

QToolButton* EditBar::createToolButton(const QString &iconPath, 
                                     const QString &tooltip,
                                     Tool tool)
{
    QToolButton *btn = new QToolButton(this);
    btn->setIcon(QIcon(iconPath));
    btn->setIconSize(QSize(20, 20));
    btn->setToolTip(tooltip);
    btn->setFixedSize(32, 32);
    
    if (tool != None) {
        btn->setCheckable(true);
        connect(btn, &QToolButton::clicked, this, [this, tool, btn]() {
            // 如果当前工具已经被选中，则取消选中
            if (m_currentTool == tool) {
                m_currentTool = None;
                btn->setChecked(false);
                emit toolChanged(None);
            } else {
                m_currentTool = tool;
                // 取消其他按钮的选中状态
                for (auto child : children()) {
                    if (auto toolBtn = qobject_cast<QToolButton*>(child)) {
                        if (toolBtn != btn) {
                            toolBtn->setChecked(false);
                        }
                    }
                }
                emit toolChanged(tool);
            }
        });
    }
    
    return btn;
}

void EditBar::show()
{
    // 重置工具状态
    m_currentTool = None;
    for (auto child : children()) {
        if (auto toolBtn = qobject_cast<QToolButton*>(child)) {
            toolBtn->setChecked(false);
        }
    }
    QWidget::show();
}

void EditBar::resetTool()
{
    if (m_currentTool != None) {  // 只有在工具被选中时才发送信号
        m_currentTool = None;
        // 取消所有工具按钮的选中状态
        for (auto child : children()) {
            if (auto toolBtn = qobject_cast<QToolButton*>(child)) {
                toolBtn->setChecked(false);
            }
        }
        emit toolChanged(None);
    }
} 