#include "NotificationManager.h"
#include <QAction>
#include <QLabel>

NotificationManager::NotificationManager(QWidget *parent,TcpClient* client)
    : QWidget{parent},
    client(client)
{
    initUi();
}

void NotificationManager::initUi()
{
    QVBoxLayout* vLayout=new QVBoxLayout(this);

    QToolBar* toolBar=topToolBar();



    // 创建QListView实例
    listView=new QListView;
    // 创建数据模型
    model=new QStandardItemModel;
    // 将模型设置给listView
    listView->setModel(model);

    // 设置自定义委托
    delegate=new ButtonDelegate(this,false);
    listView->setItemDelegate(delegate);


    // 连接 client 的 messageReceived 信号到槽函数
    connect(client,&TcpClient::messageReceived,this,&NotificationManager::handleNewFriendItemResponse);
    // 连接 client 的 messageReceived 信号到槽函数
    connect(client,&TcpClient::messageReceived,this,&NotificationManager::handleFriendBarDataResponse);
    // 连接按钮(“同意”)信号到槽函数
    connect(delegate,&ButtonDelegate::buttonClicked,this,&NotificationManager::onAgreeButtonClicked);

    vLayout->addWidget(toolBar);
    vLayout->addWidget(listView);

    this->setLayout(vLayout);
}

QToolBar *NotificationManager::topToolBar()
{
    QToolBar* bar=new QToolBar(this);

    QAction* filterNotif=new QAction(QIcon(""),"",this);
    QAction* clearNotif=new QAction(QIcon(""),"",this);


    // 向tBar中添加组件
    bar->addWidget(new QLabel("好友通知",this));

    // 加弹簧
    QWidget *spring = new QWidget(this);
    spring->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bar->addWidget(spring);

    bar->addAction(filterNotif);
    bar->addAction(clearNotif);

    return bar;
}

void NotificationManager::onRequestData()
{
    Packege send_Pkg;

    send_Pkg.sender = qApp->property("username").toString();
    send_Pkg.type = GET_FRIENDBAR_DATA;

    client->sendMessage(send_Pkg);
}

void NotificationManager::addLocalFriendRequestEntry(const Account_Message& user_Info)
{
    // =================================== 本地立即显示 =====================================
    // 头像加载
    QPixmap avatarPixmap;
    if (!avatarPixmap.loadFromData(user_Info.avatarData))
    {
        // 直接从字节数组加载图像
        qWarning() << "Failed to load pixmap from byte array";
        // 加载失败时设置默认头像
        avatarPixmap = QPixmap(":/resource/image/avatar1.jpg");
    }
    QString account = user_Info.account;
    QString nickname = user_Info.nickname!=""?user_Info.nickname:"~这个用户好懒,没设置用户名";

    //增加项目栏
    QStandardItem* item=new QStandardItem;
    item->setData(avatarPixmap, Qt::DecorationRole);                           // 头像
    item->setData(account, Qt::UserRole);                                      // QQ号
    item->setData(nickname, Qt::DisplayRole);                                  // 昵称


    //清除可编辑标志,保留其他默认标志
    item->setFlags(item->flags()&~Qt::ItemIsEditable);

    model->appendRow(item);

    QModelIndex index=model->indexFromItem(item);
    delegate->setButtonText(index,"等待同意");
    // 设置按钮为可点击状态
    delegate->setButtonEnabled(index,true);
}

void NotificationManager::addRemoteFriendRequestEntry(const Packege &resend_Pkg)
{
    if(resend_Pkg.type != FRIEND_REQUEST_SENT) return;

    // =================================== 对方立即显示 =====================================
    // 新的好友请求
    Account_Message user_Info = resend_Pkg.user_Info;

    // 头像加载
    QPixmap avatarPixmap;
    if (!avatarPixmap.loadFromData(user_Info.avatarData))
    {
        // 直接从字节数组加载图像
        qWarning() << "Failed to load pixmap from byte array";
        // 加载失败时设置默认头像
        avatarPixmap = QPixmap(":/resource/image/avatar1.jpg");
    }
    QString account = user_Info.account;
    QString nickname = user_Info.nickname!=""?user_Info.nickname:"~这个用户好懒,没设置用户名";

    //增加项目栏
    QStandardItem* item=new QStandardItem;
    item->setData(avatarPixmap, Qt::DecorationRole);                           // 头像
    item->setData(account, Qt::UserRole);                                      // QQ号
    item->setData(nickname, Qt::DisplayRole);                                  // 昵称

    //清除可编辑标志,保留其他默认标志
    item->setFlags(item->flags()&~Qt::ItemIsEditable);

    model->appendRow(item);

    QModelIndex index=model->indexFromItem(item);
    delegate->setButtonText(index,"同意");
    // 设置按钮为可点击状态
    delegate->setButtonEnabled(index,true);
}

void NotificationManager::handleNewFriendItemResponse(const Packege &resend_Pkg)
{
    if(resend_Pkg.type != FRIEND_REQUEST_SENT) return;

    // 新的好友请求
    Account_Message user_Info = resend_Pkg.user_Info;

    // 头像加载
    QPixmap avatarPixmap;
    if (!avatarPixmap.loadFromData(user_Info.avatarData))
    {
        // 直接从字节数组加载图像
        qWarning() << "Failed to load pixmap from byte array";
        // 加载失败时设置默认头像
        avatarPixmap = QPixmap(":/resource/image/avatar1.jpg");
    }
    QString account = user_Info.account;
    QString nickname = user_Info.nickname!=""?user_Info.nickname:"~这个用户好懒,没设置用户名";

    //增加项目栏
    QStandardItem* item=new QStandardItem(user_Info.nickname);
    item->setData(avatarPixmap, Qt::DecorationRole);                           // 头像
    item->setData(account, Qt::UserRole);                                      // QQ号
    item->setData(nickname, Qt::DisplayRole);                                  // 昵称


    //清除可编辑标志,保留其他默认标志
    item->setFlags(item->flags()&~Qt::ItemIsEditable);

    model->appendRow(item);

    QModelIndex index = model->indexFromItem(item);
    delegate->setButtonText(index,"同意");
    delegate->setButtonEnabled(index,true);
}


void NotificationManager::handleFriendBarDataResponse(const Packege &resend_Pkg)
{
    if(resend_Pkg.type != GET_FRIENDBAR_DATA) return;

    // 清空model
    model->clear();

    // 从服务器端数据库中直接下载数据

    QVector<QString> singleMap={"等待验证","已通过","已同意","同意"};
    for(int i=0;i<4;i++)
    {
        QVector<Account_Message> friendGroups = resend_Pkg.friendListBarItemTextStates[i];
        for(auto friendInfo:friendGroups)
        {
            // 头像加载
            QPixmap avatarPixmap;
            if (!avatarPixmap.loadFromData(friendInfo.avatarData))
            {
                // 直接从字节数组加载图像
                qWarning() << "Failed to load pixmap from byte array";
                // 加载失败时设置默认头像
                avatarPixmap = QPixmap(":/resource/image/avatar1.jpg");
            }
            QString account = friendInfo.account;
            QString nickname = friendInfo.nickname!=""?friendInfo.nickname:"~这个用户好懒,没设置用户名";

            //增加项目栏
            QStandardItem* item=new QStandardItem(friendInfo.nickname);
            item->setData(avatarPixmap, Qt::DecorationRole);                           // 头像
            item->setData(account, Qt::UserRole);                                      // QQ号
            item->setData(nickname, Qt::DisplayRole);                                  // 昵称


            // 清除可编辑标志,保留其他默认标志
            item->setFlags(item->flags()&~Qt::ItemIsEditable);

            model->appendRow(item);

            QModelIndex index=model->indexFromItem(item);
            delegate->setButtonText(index,singleMap[i]);

            // 若状态不为"同意",则按钮不能被点击
            if(singleMap[i]!="同意")
            {
                delegate->setButtonEnabled(index,false);
            }
            else
            {
                delegate->setButtonEnabled(index,true);
            }
        }
    }
}

void NotificationManager::onAgreeButtonClicked(const QModelIndex &index)
{
    Packege send_Pkg;
    send_Pkg.receiver = qApp->property("username").toString();
    send_Pkg.sender = model->itemFromIndex(index)->data(Qt::UserRole).toString();
    send_Pkg.type = HANDLE_FRIEND_REQUEST;

    client->sendMessage(send_Pkg);

    delegate->setButtonText(index,"已同意");

    // 重新获取数据，更新通知栏
    onRequestData();
}
