#ifndef CHATLIST_H
#define CHATLIST_H

#include <QWidget>
#include <QBoxLayout>
#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>
#include "ChatListItemDelegate.h"
#include "Tcpclient.h"
#include "LocalDatabase.h"
#include "AddFriendOrGroupDialog.h"

class ChatList : public QWidget
{
    Q_OBJECT
public:
    explicit ChatList(QWidget *parent = nullptr,TcpClient* client=nullptr,LocalDatabase* localBase=nullptr);
    void initUi();

public slots:
    //连接列表项点击信号到槽函数
    void onItemClicked(const QModelIndex& index);
    // 弹出添加好友窗口
    void onAddFriendOrGroupButtonClicked();
    void load_Local_chatList_Data();
    void update_TipMessage(const QString& account);

public:
    QVBoxLayout* vLayout;
    QListView* listView;
    QStandardItemModel* model;
private:
    TcpClient* client;
    LocalDatabase* localBase;
    ChatListItemDelegate* delegate;

signals:
    void switchChatPageRequested(int index);
    void switchSpecificPageRequested(const QString& user);
    void onAddNewFriendButtonClickded(const Account_Message& user_Info);
};

#endif // CHATLIST_H
