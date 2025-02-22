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
    
    // 使用 Qt 标准图标
    auto rectBtn = createToolButton("", "矩形标注", Rectangle);
    rectBtn->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));  // 临时图标
    
    auto arrowBtn = createToolButton("", "箭头标注", Arrow);
    arrowBtn->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    
    auto textBtn = createToolButton("", "文字标注", Text);
    textBtn->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    
    // 添加分隔线
    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("background-color: #3D3D3D;");
    
    // 确认和取消按钮使用标准图标
    auto confirmBtn = createToolButton("", "确认", None);
    confirmBtn->setIcon(style()->standardIcon(QStyle::SP_DialogApplyButton));
    
    auto cancelBtn = createToolButton("", "取消", None);
    cancelBtn->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    
    connect(confirmBtn, &QToolButton::clicked, this, &EditBar::confirmClicked);
    connect(cancelBtn, &QToolButton::clicked, this, &EditBar::cancelClicked);
    
    layout->addWidget(rectBtn);
    layout->addWidget(arrowBtn);
    layout->addWidget(textBtn);
    layout->addWidget(line);
    layout->addWidget(confirmBtn);
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
            if (m_currentTool != tool) {
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