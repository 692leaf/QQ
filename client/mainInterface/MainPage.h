#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QBoxLayout>
#include <QStackedLayout>
#include "SysTray.h"
#include "ToolBar.h"
#include "ChatList.h"
#include "FriendList.h"
#include "DefaultWindow.h"
#include "NotificationManager.h"
#include "SpecificChatWindow.h"
#include "FriendDetailWidget.h"


class MainPage : public QWidget
{
    Q_OBJECT
public:
    explicit MainPage(QWidget *parent = nullptr,TcpClient* client=nullptr,LocalDatabase* localBase=nullptr);
    ~MainPage();
    void initUi();
private:
    enum class Operation
    {
        NONE,
        CHATLIST, //聊天界面
        BUDDYLIST, //好友列表界面
    };
protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    // 托盘设置
    void onRestoreRequested();
    void onQuitRequested();
    // 界面切换
    void switchSecondColumnPage(int index);
    void switchSpecificWindow(int index);

private:
    TcpClient* client;
    LocalDatabase* localBase;
    // 声明托盘变量
    SysTray* m_sysTray;
    //声明布局
    QStackedLayout* sLayout_Sec;
    QStackedLayout* sLayout_SpecificWindow;
    QHBoxLayout* hLayout;

    ToolBar* tBar;

    ChatList* ctList;
    FriendList* fdList;

    DefaultWindow* dftWindow;
    NotificationManager* notifManager;
    SpecificChatWindow* chatWindow;
    FriendDetailWidget* frdDetailWidget;

signals:
    void askFriendListDataRequest();
    void chatListDataRequest();
    void friendListDataRequest();
};

#endif // MAINPAGE_H
