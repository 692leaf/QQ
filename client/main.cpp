#pragma comment(linker, "/subsystem:\"windows\" /entry:\"WinMainCRTStartup\"")

#include "LoginDialog.h"
#include "MainPage.h"
#include "Tcpclient.h"
#include "LocalDatabase.h"

#include <QApplication>

int WinMain(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpClient* client = new TcpClient;
    LocalDatabase* localBase = new LocalDatabase;
    client->initHeartBeat(30);
    //登录
    {
        LoginDialog d(nullptr,client,localBase);
        if(d.exec()==QDialog::Rejected)
        {
            return 0;
        }
    }

    //登录成功进入主界面
    MainPage w(nullptr,client,localBase);
    w.show();
    return a.exec();
}
