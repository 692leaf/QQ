// ServerManager.cpp
#include "ServerManager.h"
#include <QApplication>
#include <QMessageBox>
#include <QTimer>

ServerManager::ServerManager(QObject *parent)
    : QObject{parent}
{
    // 初始化托盘
    serverTray = new ServerTray(this);
    serverTray->setIcon(QIcon(":/resource/Server.jpg"));
    serverTray->setToolTip(tr("服务器监控"));
    serverTray->show(); // 显示托盘

    // 网络模块初始化
    if (!init())
    {
        // 初始化失败处理
        QMessageBox::critical(nullptr, tr("错误"), tr("服务启动失败，请检查端口占用或数据库连接"));
        QTimer::singleShot(0, qApp, &QCoreApplication::quit); // 安全退出
        return;
    }
    // 连接信号槽
    connect(serverTray, &ServerTray::quitRequested,
            this, &ServerManager::onQuitRequested);
}

ServerManager::~ServerManager()
{
}

bool ServerManager::init()
{
    dbManager = new DatabaseManager(this);
    server = new TcpServer(this, dbManager);
    if (!server->init())
    {
        qCritical() << "TCP 服务器初始化失败";
        // 清理已经分配的资源
        cleanup();
        return false;
    }

    return true;
}

void ServerManager::cleanup()
{
    delete server;    // 会触发 TcpServer 析构
    delete dbManager; // 需确保 DatabaseManager 析构安全
    server = nullptr;
    dbManager = nullptr;
}

void ServerManager::onQuitRequested()
{
    serverTray->hide();
    QApplication::quit();
}
