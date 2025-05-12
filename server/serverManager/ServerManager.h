// ServerManager.h
#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QObject>
#include <QWidget>
#include "servertray.h"
#include "DatabaseManager.h"
#include "TcpServer.h"

class ServerManager : public QObject
{
    Q_OBJECT
public:
    explicit ServerManager(QObject *parent = nullptr);
    ~ServerManager();

    bool init();
    void cleanup();
private slots:
    void onQuitRequested();        // 退出程序请求

private:
    ServerTray* serverTray;      // 系统托盘
    DatabaseManager* dbManager;  // 数据库
    TcpServer* server;           // 网络通讯
};

#endif // SERVERMANAGER_H
