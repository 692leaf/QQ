#include "MainPage.h"
#include <QCloseEvent>
#include <QMessageBox>

MainPage::MainPage(QWidget *parent,TcpClient* client,LocalDatabase* localBase)
    : QWidget{parent},
    client(client),
    localBase(localBase)
{
    // 设置主窗口标题
    this->setWindowTitle("QQ");

    // 初始化托盘
    m_sysTray = new SysTray(this);
    m_sysTray->setIcon(QIcon(":/resource/image/QQ.jpg"));
    m_sysTray->setToolTip(tr("QQ"));
    m_sysTray->show(); // 显示托盘

    tBar=new ToolBar(this,client,localBase);

    ctList=new ChatList(this,client,localBase);
    fdList=new FriendList(this,client,localBase);

    dftWindow=new DefaultWindow;
    notifManager=new NotificationManager(this,client);
    chatWindow=new SpecificChatWindow(this,client,localBase);
    frdDetailWidget=new FriendDetailWidget(this,localBase);

    initUi();
}

MainPage::~MainPage()
{
    delete sLayout_Sec;
}

void MainPage::initUi()
{
    //secondColumn的界面
    sLayout_Sec=new QStackedLayout;
    sLayout_Sec->addWidget(ctList);
    sLayout_Sec->addWidget(fdList);
    sLayout_Sec->setCurrentIndex(0);


    //sLayout_SpecificChatWindow
    sLayout_SpecificWindow=new QStackedLayout;
    sLayout_SpecificWindow->addWidget(dftWindow);       //0
    sLayout_SpecificWindow->addWidget(chatWindow);      //1
    sLayout_SpecificWindow->addWidget(notifManager);    //2
    sLayout_SpecificWindow->addWidget(frdDetailWidget); //3
    //设置初始界面为默认界面
    sLayout_SpecificWindow->setCurrentIndex(DEFAULT_PAGE);


    hLayout=new QHBoxLayout;
    hLayout->addWidget(tBar->tBar);
    hLayout->addLayout(sLayout_Sec);
    hLayout->addLayout(sLayout_SpecificWindow);
    setLayout(hLayout);

    /*托盘点击事件*/
    connect(m_sysTray, &SysTray::restoreRequested,this, &MainPage::onRestoreRequested);
    connect(m_sysTray, &SysTray::quitRequested,this, &MainPage::onQuitRequested);
    /*数据传递信号*/
    // 接收信号，好友框中获取account和nickname
    connect(fdList,&FriendList::sendFriendInfo,frdDetailWidget,&FriendDetailWidget::receiveFriendInfo);

    /*请求数据信号*/
    // 同步好友列表
    connect(client,&TcpClient::messageReceived,fdList,&FriendList::handleServerFriendListAsync);
    // 接收信号，向服务器请求聊天列表数据
    connect(this,&MainPage::chatListDataRequest,ctList,&ChatList::load_Local_chatList_Data);
    // 接收信号，向服务器请求好友列表数据
    connect(this,&MainPage::friendListDataRequest,fdList,&FriendList::load_Local_FriendList_Data);
    // 接收信号，向服务器请求好友申请列表数据
    connect(this,&MainPage::askFriendListDataRequest,notifManager,&NotificationManager::onRequestData);
    // 接收信号，更新好友信息
    connect(client,&TcpClient::messageReceived,fdList,&FriendList::updateFriendInfo);
    // 更新好友在线状态
    connect(client,&TcpClient::messageReceived,fdList,&FriendList::updateFriendOnlineStatus);
    // 更新最新消息提示词
    connect(chatWindow,&SpecificChatWindow::chatListMessageUpdated,ctList,&ChatList::load_Local_chatList_Data);

    /*切换界面信号*/
    // 接收信号，切换switchTabPageRequested
    connect(ctList,&ChatList::switchSpecificPageRequested,chatWindow,&SpecificChatWindow::OpenChatWidget);
    // 接收信号，切换secondColumn的界面
    connect(tBar,&ToolBar::switchSecondPageRequested,this,&MainPage::switchSecondColumnPage);
    // 接收信号，切换specificChatWindow
    connect(ctList,&ChatList::switchChatPageRequested,this,&MainPage::switchSpecificWindow);
    connect(fdList,&FriendList::switchFriendPageRequested,this,&MainPage::switchSpecificWindow);
    // 接收信号，好友消息通知中添加新通知
    connect(fdList,&FriendList::onAddNewFriendButtonClickded,notifManager,&NotificationManager::addLocalFriendRequestEntry);
}

void MainPage::closeEvent(QCloseEvent *event)
{
    // 最小化到托盘（无论是否已显示）
    hide();
    m_sysTray->show(); // 确保托盘可见
    event->ignore();   // 阻止真正关闭
}

void MainPage::onRestoreRequested()
{
    showNormal();
    activateWindow();
}

void MainPage::onQuitRequested()
{
    m_sysTray->hide();
    QApplication::quit();
}


void MainPage::switchSecondColumnPage(int index)
{
    if(sLayout_Sec)
    {
        sLayout_Sec->setCurrentIndex(index);
        //主界面中转信号
        if(index==CHATLIST_PAGE)
        {
            //当前在聊天列表界面
            emit chatListDataRequest();
        }
        if(index==FRIENDLIST_PAGE)
        {
            //当前在好友列表界面
            emit friendListDataRequest();
        }

        /*secondPage切换时,specificChatWindow切换为企鹅界面*/
        if(sLayout_SpecificWindow)
        {
            sLayout_SpecificWindow->setCurrentIndex(DEFAULT_PAGE);
        }
    }
}


void MainPage::switchSpecificWindow(int index)
{
    if(sLayout_SpecificWindow)
    {
        sLayout_SpecificWindow->setCurrentIndex(index);
        //主界面中转信号
        if(index==NOTIFICATION_PAGE)
        {
            emit askFriendListDataRequest();
        }
    }
}


