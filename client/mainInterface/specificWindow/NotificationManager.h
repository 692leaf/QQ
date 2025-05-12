#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QWidget>
#include <QBoxLayout>
#include <QToolBar>
#include <QListView>
#include <QStandardItemModel>
#include "ButtonDelegate.h"
#include "Tcpclient.h"


class NotificationManager : public QWidget
{
    Q_OBJECT
public:
    explicit NotificationManager(QWidget *parent = nullptr,TcpClient* client=nullptr);
    void initUi();
    QToolBar* topToolBar();
public slots:
    void onRequestData();
    void addLocalFriendRequestEntry(const Account_Message& user_Info);
    void addRemoteFriendRequestEntry(const Packege& resend_Pkg);
    void handleNewFriendItemResponse(const Packege& resend_Pkg);
    void handleFriendBarDataResponse(const Packege &resend_Pkg);
    void onAgreeButtonClicked(const QModelIndex& index);
private:
    TcpClient* client;
    QListView* listView;
    QStandardItemModel* model;
    ButtonDelegate* delegate;
};

#endif // NOTIFICATIONMANAGER_H
