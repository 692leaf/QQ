#include "FriendList.h"
#include <QMenuBar>
#include <QLineEdit>
#include <QPushButton>

FriendList::FriendList(QWidget *parent,TcpClient* client,LocalDatabase* localBase)
    : QWidget{parent},
    client(client),
    localBase(localBase)
{
    initUi();
}

void FriendList::initUi()
{
    // 创建布局
    vLayout = new QVBoxLayout;


    // 创建水平布局
    QHBoxLayout* hLayout=new QHBoxLayout;

    // 创建搜索栏
    QLineEdit* searchLineEdit=new QLineEdit;
    searchLineEdit->setPlaceholderText("搜索好友");

    QPushButton* menuButton = new QPushButton("+");
    // 创建添加菜单
    QMenu *popupMenu=new QMenu;        // 弹出菜单
    // 加菜单添加到按钮中
    menuButton->setMenu(popupMenu);
    // 创建菜单选项
    QAction *createGroupAction = new QAction("创建群聊", this);
    QAction *addFriendGroupAction = new QAction("加好友/群", this);
    // 将选项添加到菜单
    popupMenu->addAction(createGroupAction);
    popupMenu->addAction(addFriendGroupAction);

    // 将 searchLineEdit 和 menuButton 加入
    hLayout->addWidget(searchLineEdit);
    hLayout->addWidget(menuButton);

    // 创建好友管理器
    QPushButton* friendManager=new QPushButton("好友管理器");

    // 创建用于显示好友通知和群通知的按钮
    QPushButton* friendButton=new QPushButton("好友通知");
    QPushButton* groupButton=new QPushButton("群通知");


    // 创建QListView实例
    listView=new QListView;
    // 创建数据模型
    model=new QStandardItemModel;
    // 将模型设置给listView
    listView->setModel(model);

    // 设置自定义委托
    delegate=new FriendListItemDelegate(this);
    listView->setItemDelegate(delegate);


    /*加入垂直布局*/
    vLayout->addLayout(hLayout);
    vLayout->addWidget(friendManager);
    vLayout->addWidget(friendButton);
    vLayout->addWidget(groupButton);
    vLayout->addWidget(listView);
    setLayout(vLayout);

    // 接收信号,增加新的好友项
    connect(client,&TcpClient::messageReceived,this,&FriendList::addFriendItemOnApproval);
    // 点击添加好友或群弹出窗口
    connect(addFriendGroupAction,&QAction::triggered,this,&FriendList::onAddFriendOrGroupButtonClicked);
    // 好友通知和群通知
    connect(friendButton,&QPushButton::clicked,[this](){
        // 给mainPage的信号
        emit switchFriendPageRequested(NOTIFICATION_PAGE);
    });
    connect(groupButton,&QPushButton::clicked,[this](){
        // 给mainPage的信号
        emit switchFriendPageRequested(DEFAULT_PAGE);
    });
    // 点击切换栏目
    connect(listView,&QListView::clicked,this,&FriendList::onItemClicked);
}

void FriendList::onItemClicked(const QModelIndex& index)
{
    QStandardItem* item=model->itemFromIndex(index);
    if(item)
    {
        // 给mainPage的信号
        emit switchFriendPageRequested(FRIEND_PAGE);

        // 传给frienddetailwidget
        QString account = item->data(Qt::UserRole).toString();
        emit sendFriendInfo(account);
    }
}

// 增加新好友
void FriendList::onAddFriendOrGroupButtonClicked()
{
    AddFriendOrGroupDialog* addFrdDialog=new AddFriendOrGroupDialog(this,client);
    addFrdDialog->show();
    // 添加好友按钮点击
    connect(addFrdDialog,&AddFriendOrGroupDialog::onAddNewFriendButtonClickded,[this](const Account_Message& user_Info){
        emit onAddNewFriendButtonClickded(user_Info);
    });
}

// ============================== 加载好友列表数据 ================================
void FriendList::load_Local_FriendList_Data()
{
    // 清空model
    model->clear();

    // 从本地数据库中获取好友信息
    QString account = qApp->property("username").toString();
    QVector<Account_Message> friend_List_Data = localBase->userProfile_Table_Load_FriendListInfo(account);

    if(friend_List_Data.empty()) return;


    // 设置列表项
    for(auto user_Info:friend_List_Data)
    {
        QString avatarPath = user_Info.avatar_Path!="" ? user_Info.avatar_Path : ":/resource/image/avatar1.jpg";
        QString account = user_Info.account;
        QString nickname = user_Info.nickname!="" ? user_Info.nickname : "~这个用户好懒,没设置用户名";
        bool isOnline = user_Info.status;

        QStandardItem* item = new QStandardItem;
        item->setData(QPixmap(avatarPath), Qt::DecorationRole);                             // 头像
        item->setData(account, Qt::UserRole);                                               // QQ号
        item->setData(nickname, Qt::DisplayRole);                                           // 昵称
        item->setData(isOnline, StatusRole);                                                // 状态（在线，离线）

        // 清除可编辑标志,保留其他默认标志
        item->setFlags(item->flags()&~Qt::ItemIsEditable);

        model->appendRow(item);
    }
}

// ============================== 更新好友信息 ===============================
void FriendList::updateFriendInfo(const Packege &friendUpdatePackage)
{
    if(friendUpdatePackage.type != UPDATE_ACCOUNT_INFO) return;

    // 更新本地数据库
    localBase->userProfile_Table_Update_UserInfo(friendUpdatePackage.user_Info);

    // 获取更新后的完整信息
    QString account = friendUpdatePackage.user_Info.account;
    Account_Message user_Info = localBase->userProfile_Table_Load_localAccountInfo(account);

    // 遍历模型查找对应项
    for(int row = 0; row < model->rowCount(); ++row)
    {
        QStandardItem* item = model->item(row);
        if (item && item->data(Qt::UserRole).toString() == account)
        {
            // 更新所有显示相关数据
            QString avatarPath = user_Info.avatar_Path.isEmpty() ?
                                     ":/resource/image/avatar1.jpg" :
                                     user_Info.avatar_Path;

            // 更新数据
            item->setData(QPixmap(avatarPath), Qt::DecorationRole);  // 头像
            item->setData(user_Info.nickname, Qt::DisplayRole);      // 昵称
            item->setData(user_Info.status, StatusRole);             // 状态

            // 触发视图更新
            QModelIndex index = model->indexFromItem(item);
            listView->update(index);
        }
    }
}

// ============================== 更新好友在线状态 =============================
void FriendList::updateFriendOnlineStatus(const Packege &onlineStatusPackage)
{
    if(onlineStatusPackage.type != UPDATE_ONLINE_STATUS) return;

    Account_Message user_Info = onlineStatusPackage.user_Info;
    // 更新本地数据库
    localBase->userProfile_Table_Update_FriendStatus(user_Info);

    // 遍历模型查找对应项
    for(int row = 0; row < model->rowCount(); ++row)
    {
        QStandardItem* item = model->item(row);
        if (item && item->data(Qt::UserRole).toString() == user_Info.account)
        {
            // 更新在线状态数据
            item->setData(user_Info.status, StatusRole);

            // 触发视图更新
            QModelIndex index = model->indexFromItem(item);
            listView->update(index);
        }
    }
}

// ================================== 同步好友信息 =====================================
void FriendList::handleServerFriendListAsync(const Packege& friendInfoSyncPackage)
{
    if(friendInfoSyncPackage.type != ASYNC_FETCH_FRIEND_LIST_INFOS) return;

    localBase->userProfile_Table_Compare_Version(friendInfoSyncPackage.friend_List_Data);
}

// =============================== 对方同意好友申请 ==================================
void FriendList::addFriendItemOnApproval(const Packege& resend_Pkg)
{
    if(resend_Pkg.type != HANDLE_FRIEND_REQUEST) return;

    localBase->userProfile_Table_Create_UserInfo(resend_Pkg.user_Info);

    Account_Message user_Info = localBase->userProfile_Table_Load_localAccountInfo(resend_Pkg.user_Info.account);
    QString avatarPath = user_Info.avatar_Path!="" ? user_Info.avatar_Path : ":/resource/image/avatar1.jpg";
    QString account = user_Info.account;
    QString nickname = user_Info.nickname!="" ? user_Info.nickname : "~这个用户好懒,没设置用户名";
    bool isOnline = user_Info.status;

    QStandardItem* item = new QStandardItem;
    item->setData(QPixmap(avatarPath), Qt::DecorationRole);                             // 头像
    item->setData(account, Qt::UserRole);                                               // QQ号
    item->setData(nickname, Qt::DisplayRole);                                           // 昵称
    item->setData(isOnline, StatusRole);                                                // 状态（在线，离线）

    // 清除可编辑标志,保留其他默认标志
    item->setFlags(item->flags()&~Qt::ItemIsEditable);

    model->appendRow(item);
    // 手动更新列表
    listView->viewport()->update();
}

