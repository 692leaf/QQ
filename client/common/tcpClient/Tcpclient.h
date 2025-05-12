#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTimer>
#include <QTcpSocket>
#include "Packege.h"
#include "MessageType.h"

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    void initHeartBeat(int intervalMs);
    void heartbeatSent();
    void heartbeatTimeCheck(const Packege& reheart_Pkg);
public slots:
    void serverConnect();
    void readMessage();
    bool sendMessage(const Packege& sender_Pkg);
    void processPackage(Packege& resend_Pkg);
    //连接成功或失败
    void onConnected();
    void onConnectionError();
private:
    QTimer* heartbeatTimer;
    QTimer* timeoutTimer;
    int intervalMs;
    QTcpSocket* socket;
    QByteArray socketBuffer; // 接收缓冲区
signals:
    void heartbeatTimeout();
    void messageReceived(const Packege& resender_Pkg);
};

#endif // TCPCLIENT_H
