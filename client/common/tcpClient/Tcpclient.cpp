#include "Tcpclient.h"
#include <QApplication>
#include <QDateTime>
#include <QMessageBox>

TcpClient::TcpClient(QObject *parent)
    : QObject{parent}
{
    socket=new QTcpSocket;

    // 连接信号
    connect(socket,&QTcpSocket::connected,this,&TcpClient::onConnected);
    connect(socket,&QTcpSocket::errorOccurred,this,&TcpClient::onConnectionError);

    serverConnect();

    connect(this,&TcpClient::heartbeatTimeout,[this](){
        if (socket->state() == QAbstractSocket::ConnectedState)
        {
            socket->disconnectFromHost();
        }
        serverConnect();  // 重新连接
    });
}


TcpClient::~TcpClient()
{
    heartbeatTimer->stop();
    timeoutTimer->stop();

    socket->close();
    delete socket;
}

void TcpClient::initHeartBeat(int intervalMs)
{
    // 将传入的参数赋给类的成员变量
    this->intervalMs=intervalMs;
    heartbeatTimer=new QTimer(this);
    heartbeatTimer->setInterval(intervalMs);
    timeoutTimer=new QTimer(this);
    timeoutTimer->setInterval(0.5*intervalMs);
    // 启动超时监测
    heartbeatTimer->start();
    connect(this,&TcpClient::messageReceived,this,&TcpClient::heartbeatTimeCheck);
}

void TcpClient::heartbeatSent()
{
    connect(heartbeatTimer, &QTimer::timeout, this, [this]{
        if (socket->state() != QAbstractSocket::ConnectedState) return;

        // 发送心跳包
        Packege heartbeat_Pkg;
        heartbeat_Pkg.type=HEARTBEAT_MONITORING;
        heartbeat_Pkg.sender=qApp->property("username").toString();
        heartbeat_Pkg.timeStamp= QDateTime::currentMSecsSinceEpoch();
        sendMessage(heartbeat_Pkg);
        // 启动心跳超时检测
        timeoutTimer->start();
    });

    //超时处理
    connect(timeoutTimer, &QTimer::timeout, [this] (){
        emit heartbeatTimeout();
        timeoutTimer->stop();
    });
}

void TcpClient::heartbeatTimeCheck(const Packege& resend_Pkg)
{
    if(resend_Pkg.type!=HEARTBEAT_MONITORING) return;
    // 收到心跳响应，停止超时检测
    if(QDateTime::currentMSecsSinceEpoch()-resend_Pkg.timeStamp<intervalMs)
    {
        timeoutTimer->stop();
    }
}


void TcpClient::serverConnect()
{
    QString ip="192.168.43.144";
    QString port="6000";
    socket->connectToHost(QHostAddress(ip),port.toUShort());
}



void TcpClient::readMessage()
{
    if(!socket) return;

    /*新增粘包处理*/
    // 维护每个socket的接收缓冲区
    QByteArray &buffer = socketBuffer;
    buffer += socket->readAll();

    //完整包的数据
    QByteArray pkgData;
    // 包处理循环
    while (true)
    {
        // 包头未接收完整
        if (buffer.size() < static_cast<int>(sizeof(quint32)))
            break;

        // 提取包体长度
        quint32 bodySize;
        QDataStream headStream(buffer);
        headStream >> bodySize;

        // 包体未接收完整
        if (buffer.size() < static_cast<int>(sizeof(quint32) + bodySize))
            break;

        // 提取完整数据包
        pkgData = buffer.mid(sizeof(quint32), bodySize);
        buffer.remove(0, sizeof(quint32) + bodySize);

        // 反序列化处理
        Packege resend_Pkg;
        QDataStream in(pkgData);
        in.setVersion(QDataStream::Qt_6_5); // 必须与客户端一致
        in >> resend_Pkg;

        // 处理数据包
        processPackage(resend_Pkg);
    }
}



bool TcpClient::sendMessage(const Packege& send_Pkg)
{
    /*qDebug()<<"message_type:"<<send_Pkg.messageInfo.message_type;
    qDebug()<<"fileContent:"<<send_Pkg.messageInfo.file.fileContent;*/

    if (!socket || socket->state() != QTcpSocket::ConnectedState)
    {
        return false;
    }


    //序列化数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5); // 必须与服务器端一致

    // 先预留包头位置
    out << quint32(0);
    // 写入实际数据
    out << send_Pkg;

    // 回到数据开头写入实际长度
    out.device()->seek(0);
    out << quint32(block.size()-sizeof(quint32));


    // 发送数据
    return socket->write(block)!=-1;
}

void TcpClient::processPackage(Packege& resend_Pkg)
{
    emit messageReceived(resend_Pkg);
}


void TcpClient::onConnected()
{
    connect(socket,&QTcpSocket::readyRead,this,&TcpClient::readMessage);
}

void TcpClient::onConnectionError()
{
    if(socket)
    {
        QMessageBox::information(nullptr,"连接提示","连接服务器失败:"+socket->errorString());
    }

}




