#pragma comment(linker, "/subsystem:\"windows\" /entry:\"WinMainCRTStartup\"")

#include <QApplication>
#include "Tcpserver.h"
#include "ServerManager.h"

int WinMain(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TcpServer server;
    ServerManager serverManager;

    return a.exec();
}
