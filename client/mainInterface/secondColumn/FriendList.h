#ifndef FRIENDLIST_H
#define FRIENDLIST_H

#include <QWidget>
#include <QBoxLayout>
#include <QListView>
#include <QStandardItem>
#include <QStandardItemModel>
#include "MessageType.h"
#include "FriendListItemDelegate.h"
#include "Tcpclient.h"
#include "LocalDatabase.h"
#include "AddFriendOrGroupDialog.h"

class FriendList : public QWidget
{
    Q_OBJECT
public:
    explicit FriendList(QWidget *parent = nullptr,TcpClient* client=nullptr,LocalDatabase* localBase=nullptr);
    void initUi();
public:
    QVBoxLayout* vLayout;
    QListView* listView;
    QStandardItemModel* model;
private:
    TcpClient* client;
    LocalDatabase* localBase;
    FriendListItemDelegate* delegate;
public slots:
    //连接列表项点击信号到槽函数
    void onItemClicked(const QModelIndex& index);
    void onAddFriendOrGroupButtonClicked();                                    // 弹出添加好友窗口
    void load_Local_FriendList_Data();                                         // 加载数据
    void updateFriendInfo(const Packege& friendUpdatePackage);                 // 更新好友信息
    void updateFriendOnlineStatus(const Packege& onlineStatusPackage);         // 更新好友在线状态
    void handleServerFriendListAsync(const Packege& friendInfoSyncPackage);    // 同步数据
    void addFriendItemOnApproval(const Packege& resend_Pkg);                   // 同意好友请求，增加新好友项
signals:
    void switchFriendPageRequested(int index);
    void sendFriendInfo(const QString& user_Info);
    void onAddNewFriendButtonClickded(const Account_Message& user_Info);
};

#endif // FRIENDLIST_H
