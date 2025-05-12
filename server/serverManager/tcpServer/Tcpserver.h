#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "DatabaseManager.h"
#include "Packege.h"
#include "MessageType.h"

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr, DatabaseManager* dbManager = nullptr);
    ~TcpServer();
    // 初始化
    bool init();

    bool startListen();
public slots:
    void clientConnect();
    void readMessage();
    bool sendMessage(const Packege& resend_Pkg);
    bool sendMessage(const Packege& resend_Pkg,QTcpSocket* sendSocket);

    void processPackage(Packege& send_Pkg,QTcpSocket* sendSocket);

    void generateLocalImageUrls(Packege& send_Pkg);
    QString generateRichTextInfo(const Packege& send_Pkg);

    void sendImageInfo(Packege& resend_Pkg,QTcpSocket* targetSocket);
    QByteArray fetchImageData(const QString& imagePath);
    QVector<Image> extractImagesFromHtml(QString& html);
    void sendFileInfo(Packege& resend_Pkg,QTcpSocket* targetSocket);

    // 用户登录时,未读消息同步
    void unreadNotificationAsync(const QString& account);
    void unreadFriendListInfosAsync(const QString& account);
    void unreadTextFileNotificationsAsync(const QString& account);

    // 通知好友更新我的在线状态
    void sendOnlineStatusUpdateToFriends(const QString& account, bool isOnline);
private:
    QTcpServer* server;
    QList<QTcpSocket*> clientSockets;
    QHash<QTcpSocket*, QByteArray> socketBuffers; // 接收缓冲区

    DatabaseManager* dbManager; // 数据库
    QMap<QString,QTcpSocket*> account_Socket_Map; // 数据块
    QHash<QString,Packege> activePackege;

    QTcpSocket* findSocketByAccount(const QString& account);
signals:
    void imageUrlsGenerated(const Packege& send_Pkg);
};

#endif // TCPSERVER_H
