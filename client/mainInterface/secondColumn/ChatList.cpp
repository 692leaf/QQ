#include "ChatList.h"
#include <QApplication>
#include <QMenuBar>
#include <QLineEdit>
#include <QPushButton>

ChatList::ChatList(QWidget *parent,TcpClient* client,LocalDatabase* localBase)
    : QWidget{parent},
    client(client),
    localBase(localBase)
{
    initUi();
}

void ChatList::initUi()
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

    // 创建QListView实例
    listView=new QListView(this);
    // 创建数据模型
    model=new QStandardItemModel;
    // 将模型设置给listView
    listView->setModel(model);

    // 设置自定义委托
    delegate=new ChatListItemDelegate(this);
    listView->setItemDelegate(delegate);

    /*加入垂直布局*/
    vLayout->addLayout(hLayout);
    vLayout->addWidget(listView);
    setLayout(vLayout);

    // 点击添加好友或群弹出窗口
    connect(addFriendGroupAction,&QAction::triggered,this,&ChatList::onAddFriendOrGroupButtonClicked);
    // 点击切换栏目
    connect(listView,&QListView::clicked,this,&ChatList::onItemClicked);
}


void ChatList::onItemClicked(const QModelIndex& index)
{
    QStandardItem* item=model->itemFromIndex(index);
    //点击栏目发出信号,聊天窗口接收信号实现切换
    if(item)
    {
        emit switchChatPageRequested(CHAT_PAGE);
        emit switchSpecificPageRequested(item->data(Qt::UserRole).toString());
    }
}

void ChatList::onAddFriendOrGroupButtonClicked()
{
    AddFriendOrGroupDialog* addFrdDialog=new AddFriendOrGroupDialog(this,client);
    addFrdDialog->show();
    // 添加好友按钮点击
    connect(addFrdDialog,&AddFriendOrGroupDialog::onAddNewFriendButtonClickded,[this](const Account_Message& user_Info){
        emit onAddNewFriendButtonClickded(user_Info);
    });
}

// ==================================== 加载数据 ========================================
void ChatList::load_Local_chatList_Data()
{
    // 清空model
    model->clear();


    // 从本地数据库中获取好友信息
    QString account = qApp->property("username").toString();
    QVector<Account_Message> chat_List_Data = localBase->userProfile_Table_Load_ChatListInfo(account);

    if(chat_List_Data.empty()) return;

    for(auto user_Info:chat_List_Data)
    {
        QString avatarPath = user_Info.avatar_Path!="" ? user_Info.avatar_Path : ":/resource/image/avatar1.jpg";
        QString account = user_Info.account;
        QString nickname = user_Info.nickname!="" ? user_Info.nickname:"~这个用户好懒,没设置用户名";
        QString tipMessage = user_Info.tipMessage;

        QStandardItem* item = new QStandardItem;
        item->setData(QPixmap(avatarPath), Qt::DecorationRole);                             // 头像
        item->setData(account, Qt::UserRole);                                               // QQ号
        item->setData(nickname, Qt::DisplayRole);                                           // 昵称
        item->setData(tipMessage, TipMessageRole);                                          // 最新消息

        // 清除可编辑标志,保留其他默认标志
        item->setFlags(item->flags()&~Qt::ItemIsEditable);

        model->appendRow(item);
    }
}

// ====================================== 更新最新消息提示词 ===========================================

void ChatList::update_TipMessage(const QString& account)
{
    // 从本地数据库中获取好友信息
    QString tipMessage = localBase->local_ChrTable_Load_TipMessage(account);

    // 遍历模型中的现有项
    for (int row = 0; row < model->rowCount(); ++row)
    {
        QStandardItem* item = model->item(row);
        if (!item) continue;

        // 检查 UserRole 是否匹配目标 account
        QString itemAccount = item->data(Qt::UserRole).toString();
        if (itemAccount != account) continue;

        // 更新最新消息
        item->setData(tipMessage, TipMessageRole);

        // 触发视图更新
        QModelIndex index = model->indexFromItem(item);
        listView->update(index);
    }
}
