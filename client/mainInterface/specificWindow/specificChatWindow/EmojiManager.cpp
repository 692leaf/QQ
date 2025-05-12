#include "EmojiManager.h"
#include <QPushButton>
#include <QBoxLayout>
#include <QScrollArea>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>

EmojiManager::EmojiManager(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint); // 设置为无边框弹出窗口
    setupUI();
}

void EmojiManager::setupUI()
{
    // --- 添加滚动区域 ---
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); // 允许内容自适应大小

    // 创建承载表情按钮的内部容器
    QWidget *contentWidget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(contentWidget); // 布局绑定到 contentWidget
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft); // 顶部对齐

    // --- 动态获取表情路径 ---
    QStringList emojiPaths;
    QDirIterator it(":/resource/image/emojis",  // 资源路径
                    QDir::Files);
    while (it.hasNext())
    {
        emojiPaths.append(it.next()); // 获取完整资源路径，如 ":/resource/image/emojis/smile.png"
    }

    // 每行10个按钮
    const int BUTTONS_PER_ROW = 10; // 改为每行10个
    const QSize BUTTON_SIZE(40, 40); // 固定按钮尺寸

    for (int rowIndex = 0; rowIndex < emojiPaths.size(); rowIndex += BUTTONS_PER_ROW) // 每次步进 5
    {
        QHBoxLayout* rowLayout = new QHBoxLayout;
        rowLayout->setAlignment(Qt::AlignLeft); // 行内左对齐
        rowLayout->setSpacing(0); // 按钮间距为0
        for(int colIndex =rowIndex;colIndex <rowIndex+BUTTONS_PER_ROW  && colIndex <emojiPaths.size();++colIndex )
        {
            QPushButton* button = new QPushButton(this);

            // 设置图标
            QString emojiPath = emojiPaths[colIndex];
            button->setIcon(QIcon(emojiPath));

            // 设置悬停提示
            QFileInfo fileInfo(emojiPath);
            QString displayName = fileInfo.fileName();
            displayName.replace(QRegularExpression("\\.\\w+$"), ""); // 正则匹配后缀
            button->setToolTip(displayName);  // 设置工具提示

            // 设置按钮属性
            button->setIconSize(BUTTON_SIZE - QSize(8,8)); // 图标略小于按钮
            button->setFixedSize(BUTTON_SIZE); // 固定按钮尺寸
            button->setFlat(true); // 去掉按钮边框
            connect(button, &QPushButton::clicked,[this, emojiPath](){
                // 处理表情点击事件
                emit emojiClicked(emojiPath);
            });
            rowLayout->addWidget(button);
        }
        mainLayout->addLayout(rowLayout);
    }

    // 将内容容器设置到滚动区域
    scrollArea->setWidget(contentWidget);

    // 设置主窗口布局（将滚动区域作为唯一内容）
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0); // 移除边距
    outerLayout->addWidget(scrollArea);

    this->setLayout(outerLayout);
    // --- 固定窗口大小 ---
    this->setFixedSize(420, 300); // 宽度 = 40*10 + 20（滚动条空间）
}
