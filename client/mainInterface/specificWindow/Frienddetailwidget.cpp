#include "Frienddetailwidget.h"

FriendDetailWidget::FriendDetailWidget(QWidget *parent, LocalDatabase *localBase)
    : QWidget{parent},
      localBase(localBase)
{
    // 创建主垂直布局
    QVBoxLayout *mainVerticalLayout = new QVBoxLayout;

    // 创建头像标签
    avatarLabel = new QLabel("");
    // 固定头像标签大小为 400x400
    avatarLabel->setFixedSize(400, 400);
    // 创建昵称和QQ号标签
    nicknameLabel = new QLabel("");
    qqNumberLabel = new QLabel("");
    // 创建备注、签名和 QQ 空间标签
    remarkLabel = new QLabel("备注: 鲁迅");
    signatureLabel = new QLabel("签名: 这是美好的一天");
    qqSpaceLabel = new QLabel("<a href='https://www.baidu.com'>进入 QQ 空间</a>", nullptr); // user.qzone.qq.com/123456
    qqSpaceLabel->setTextFormat(Qt::RichText);
    qqSpaceLabel->setOpenExternalLinks(true);

    // 设置昵称,QQ号,备注,签名和qq空间标签的字体
    QFont font;
    font.setPointSize(14);   // 设置字体大小为14磅
    font.setFamily("Arial"); // 设置字体为Arial，你可以根据需要修改
    nicknameLabel->setFont(font);
    qqNumberLabel->setFont(font);
    remarkLabel->setFont(font);
    signatureLabel->setFont(font);
    qqSpaceLabel->setFont(font);

    // 创建操作按钮
    QPushButton *shareButton = new QPushButton("分享");
    QPushButton *callButton = new QPushButton("音视频通话");
    QPushButton *messageButton = new QPushButton("发消息");

    // 创建水平布局用于放置头像和信息
    QHBoxLayout *avatarInfoHorizontalLayout = new QHBoxLayout;

    // 创建垂直布局用于放置昵称和 QQ 号
    QVBoxLayout *nicknameQQNumberVerticalLayout = new QVBoxLayout;
    nicknameQQNumberVerticalLayout->addWidget(nicknameLabel);
    nicknameQQNumberVerticalLayout->addWidget(qqNumberLabel);

    // 将头像标签和昵称 QQ 号布局添加到头像信息水平布局
    avatarInfoHorizontalLayout->addWidget(avatarLabel);
    avatarInfoHorizontalLayout->addLayout(nicknameQQNumberVerticalLayout);

    QVBoxLayout *centralVerticalLayout = new QVBoxLayout;
    centralVerticalLayout->addLayout(avatarInfoHorizontalLayout);
    centralVerticalLayout->addWidget(remarkLabel);
    centralVerticalLayout->addWidget(signatureLabel);
    centralVerticalLayout->addWidget(qqSpaceLabel);

    // 创建新的水平布局用于在右侧添加间距
    QHBoxLayout *leftSpacedLayout = new QHBoxLayout;
    QSpacerItem *leftMarginSpacer = new QSpacerItem(10, 20, QSizePolicy::Maximum, QSizePolicy::Minimum);
    QSpacerItem *rightMarginSpacer = new QSpacerItem(10, 20, QSizePolicy::Maximum, QSizePolicy::Minimum);
    leftSpacedLayout->addSpacerItem(leftMarginSpacer);
    leftSpacedLayout->addLayout(centralVerticalLayout);
    leftSpacedLayout->addSpacerItem(rightMarginSpacer);

    // 创建水平布局用于放置操作按钮
    QHBoxLayout *operationButtonsHorizontalLayout = new QHBoxLayout;

    // 在按钮左侧添加弹簧
    QSpacerItem *leftButtonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    operationButtonsHorizontalLayout->addSpacerItem(leftButtonSpacer);

    operationButtonsHorizontalLayout->addWidget(shareButton);
    operationButtonsHorizontalLayout->addWidget(callButton);
    operationButtonsHorizontalLayout->addWidget(messageButton);

    // 在按钮右侧添加弹簧
    QSpacerItem *rightButtonSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    operationButtonsHorizontalLayout->addSpacerItem(rightButtonSpacer);

    // 将头像信息水平布局和操作按钮水平布局添加到主垂直布局
    mainVerticalLayout->addLayout(leftSpacedLayout);
    mainVerticalLayout->addLayout(operationButtonsHorizontalLayout);

    // 设置主垂直布局为当前窗口的布局
    setLayout(mainVerticalLayout);
}

void FriendDetailWidget::receiveFriendInfo(const QString &account)
{
    // 处理接收到的 account 信息
    Account_Message user_Info = localBase->userProfile_Table_Load_localAccountInfo(account);
    QPixmap pixmap(user_Info.avatar_Path);
    pixmap = pixmap.scaled(avatarLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 保持比例缩放，适应标签大小
    avatarLabel->setPixmap(pixmap);
    nicknameLabel->setText("昵称: " + user_Info.nickname);
    qqNumberLabel->setText("QQ: " + account);
}
