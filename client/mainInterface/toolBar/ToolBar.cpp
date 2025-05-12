#include "ToolBar.h"
#include <QIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QFont>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <QStandardPaths>

ToolBar::ToolBar(QWidget *parent,TcpClient* client,LocalDatabase* localBase)
    : QWidget{parent},
    client(client),
    localBase(localBase)
{
    initUi();
}

void ToolBar::initUi()
{
    tBar=new QToolBar(this);
    QAction* avatarAction = setUserAvatarBar();
    QAction* chatAction = setChatMessageBar();
    QAction* friendListAction = setFriendListBar();

    //添加操作
    tBar->addAction(avatarAction);
    tBar->addAction(chatAction);
    tBar->addAction(friendListAction);

    //在左侧停靠
    tBar->setAllowedAreas(Qt::LeftToolBarArea);
    //垂直摆放
    tBar->setOrientation(Qt::Vertical);

    //点击头像，弹出图片对话框
    connect(avatarAction,&QAction::triggered,[this](){
        //获取用户信息(异步性，数据返回后，创建对话框)
        load_Local_Account_Message();
    });

    //发送信号给mainPage
    connect(chatAction,&QAction::triggered,[this](){
        emit switchSecondPageRequested(CHATLIST_PAGE);
    });

    connect(friendListAction,&QAction::triggered,[this](){
        emit switchSecondPageRequested(FRIENDLIST_PAGE);
    });

}


QAction* ToolBar::setUserAvatarBar()
{
    QAction* avatarAction = new QAction(this);
    avatarAction->setIcon(QIcon(":/resource/image/contacts.png"));
    avatarAction->setToolTip("图片");
    return avatarAction;
}

QAction* ToolBar::setChatMessageBar()
{
    QAction* chatAction = new QAction(this);
    chatAction->setIcon(QIcon(":/resource/image/chat.png"));
    chatAction->setToolTip("消息");
    return chatAction;
}

QAction* ToolBar::setFriendListBar()
{
    QAction* friendListAction = new QAction(this);
    friendListAction->setIcon(QIcon(":/resource/image/contacts.png"));
    friendListAction->setToolTip("联系人");
    return friendListAction;
}


void ToolBar::avatarDialog()
{
    dlog = new QDialog(this);
    dlog->setWindowTitle("查看个人信息");
    dlog->setFixedSize(200,150);

    //============================= 布局设置 =============================

    // 创建主布局：对话框整体垂直布局
    QVBoxLayout* mainVerticalLayout = new QVBoxLayout(dlog);

    // 头部布局：包含头像和用户信息的水平布局
    QHBoxLayout* headerHorizontalLayout = new QHBoxLayout;

    // 信息布局：用户ID和昵称的垂直布局
    QVBoxLayout* infoVerticalLayout = new QVBoxLayout;
    QLabel* nameLabel=new QLabel(displayNickname,dlog);
    QLabel* idLabel=new QLabel("QQ "+qqNumber,dlog);
    infoVerticalLayout->addWidget(nameLabel);
    infoVerticalLayout->addWidget(idLabel);


    QLabel* avatarLabel = new QLabel;
    // 加载图片并设置到标签
    QPixmap image(avatar_Path);
    if (image.isNull())
    {
        qWarning() << "Failed to load avatar image";
        return;
    }

    headerHorizontalLayout->addWidget(avatarLabel);
    headerHorizontalLayout->addLayout(infoVerticalLayout);



    // 操作布局：按钮区域的水平布局
    QHBoxLayout* actionHorizontalLayout = new QHBoxLayout;
    QPushButton* editButton = new QPushButton("编辑资料",dlog);
    QPushButton* sendMessageButton = new QPushButton("发消息",dlog);
    actionHorizontalLayout->addWidget(editButton);
    actionHorizontalLayout->addWidget(sendMessageButton);


    mainVerticalLayout->addLayout(headerHorizontalLayout); // 先添加头部布局
    mainVerticalLayout->addLayout(actionHorizontalLayout); // 再添加操作布局

    dlog->setLayout(mainVerticalLayout);

    // ================================= 设置样式 =====================================

    // 创建一个 QFont 对象
    QFont font = nameLabel->font();
    // 设置字体大小
    font.setPointSize(20);
    // 将修改后的字体应用到 QLabel 上
    nameLabel->setFont(font);

    // 调整图像大小
    int side = 100; // 设置固定尺寸
    QPixmap scaledImage = image.scaled(side, side, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    avatarLabel->setPixmap(scaledImage);
    avatarLabel->setFixedSize(side, side);

    connect(editButton,&QPushButton::clicked,this,&ToolBar::editDialog);
    dlog->show();


    // 标签，签名等，，，待添加
}

void ToolBar::editDialog()
{
    dlog->accept();

    editDlog = new QDialog(this);
    editDlog->setWindowTitle("编辑资料");

    QVBoxLayout* vLayout=new QVBoxLayout(editDlog);

    // 创建控件
    QPushButton* selectAvatarBtn = new QPushButton(QIcon(avatar_Path),"选择图片",editDlog);
    QLabel* nicknameLabel = new QLabel("昵称",editDlog);
    QLineEdit* nicknameEdit = new QLineEdit(editDlog);
    QLabel* nicknameCountLabel = new QLabel("0/36",editDlog);//创建字符计数标签
    QPushButton* saveButton = new QPushButton("保存",editDlog);
    QPushButton* cancelButton = new QPushButton("取消",editDlog);

    // 创建水平布局放置昵称标签和输入框
    QHBoxLayout* nicknameLayout = new QHBoxLayout(editDlog);
    nicknameLayout->addWidget(nicknameLabel);
    nicknameLayout->addWidget(nicknameEdit);
    nicknameLayout->addWidget(nicknameCountLabel);

    // 创建水平布局放置昵称保存和取消按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout(editDlog);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);


    // 总体垂直布局
    vLayout->addWidget(selectAvatarBtn);
    vLayout->addLayout(nicknameLayout);
    vLayout->addLayout(buttonLayout);

    editDlog->setLayout(vLayout);

    // 图片设置
    connect(selectAvatarBtn,&QPushButton::clicked,[this](){
        // ================================= 图片路径默认使用上一次的 ====================================

        // 获取图片路径
        QString imagePath=QFileDialog::getOpenFileName(
            this,"选择图片作为头像框","D:/","图片(*gif;*png;*jpg;*jpeg;*.webp;*avif;*.bmp;*sharpp;*.apng)");
        if(!imagePath.isEmpty())
        {
            avatar_Path = imagePath;
        }
        else
        {
            qDebug()<<"未选择图片";
        }
    });

    // 文本设置
    connect(nicknameEdit,&QLineEdit::textChanged,[nicknameEdit,nicknameCountLabel](const QString& text){
        int length=text.length();
        nicknameCountLabel->setText(QString::number(length)+"/36");
        // 判断长度是否超过36，如果超过则恢复到之前的文本（即不允许继续输入）
        if (length > 36)
        {
            nicknameEdit->setText(nicknameEdit->text().left(36));
        }
    });

    connect(saveButton,&QPushButton::clicked,[this,nicknameEdit,saveButton](){
        if (isLocalDataUpdatePending)
        {
            qDebug() << "请求已发送，请等待响应";
            return;
        }
        isLocalDataUpdatePending=true;

        // ================= 发送信号给服务器，通知好友更新我的本地账户信息 =================
        // 获取账号和昵称
        Packege send_Pkg;
        send_Pkg.user_Info.account = qApp->property("username").toString();
        send_Pkg.user_Info.nickname = nicknameEdit->text();
        getImageBinaryDataByPath(avatar_Path,send_Pkg.user_Info);
        send_Pkg.user_Info.status = 1;

        // 生成版本号
        QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
        send_Pkg.user_Info.version = uuid;
        send_Pkg.type = UPDATE_ACCOUNT_INFO;

        client->sendMessage(send_Pkg);

        // 本地数据库更新本地用户信息
        bool onSave = localBase->userProfile_Table_Update_UserInfo(send_Pkg.user_Info);
        // 保存成功退出窗口
        if(onSave)
        {
            editDlog->accept();
        }

        isLocalDataUpdatePending = false;
    });

    // 点击叉号退出窗口
    connect(cancelButton,&QPushButton::clicked,editDlog,&QDialog::reject);
    // exec使得主线程被阻塞,信号无法接收,改成show
    editDlog->show();
}

// ========================= 本地数据库加载数据，获取本地账户数据 ======================
void ToolBar::load_Local_Account_Message()
{
    QString account=qApp->property("username").toString();
    Account_Message user_Info=localBase->userProfile_Table_Load_localAccountInfo(account);

    qqNumber = account;
    displayNickname = user_Info.nickname;
    avatar_Path = user_Info.avatar_Path;

    //创建对话框
    avatarDialog();
}


void ToolBar::getImageBinaryDataByPath(const QString &localPath,Account_Message& user_Info)
{
    QFileInfo imageInfo(localPath);

    user_Info.imageType = imageInfo.suffix().toLower(); // 图片类型（png等）

    // 安全读取图片数据
    QFile image(localPath);
    if (image.open(QIODevice::ReadOnly))
    {
        user_Info.avatarData = image.readAll();
        image.close();
    }
    else
    {
        qWarning() << "无法打开图片文件:" << localPath << "错误原因:" << image.errorString();
        user_Info.avatarData.clear(); // 清空无效数据
    }
}



