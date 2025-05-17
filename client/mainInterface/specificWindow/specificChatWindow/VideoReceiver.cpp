#include "VideoReceiver.h"
#include <QImage>

VideoReceiver::VideoReceiver(QObject *parent)
    : QObject{parent}
{
    udpSocket = new QUdpSocket(this);
}

VideoReceiver::~VideoReceiver()
{
    delete udpSocket;
}

void VideoReceiver::startListening()
{
    // 已开启监听
    if (udpSocket->state() == QUdpSocket::BoundState)
    {
        return;
    }

    // 端口设为0表示自动分配
    if (udpSocket->bind(QHostAddress::Any, 0))
    {
        localListenPort = udpSocket->localPort();
    }
    else
    {
        qDebug() << "绑定失败，错误信息:" << udpSocket->errorString();
    }
    connect(udpSocket, &QUdpSocket::readyRead, this, &VideoReceiver::readPendingDatagrams);
}

quint16 VideoReceiver::getLocalBindPort()
{
    startListening();
    return localListenPort;
}

void VideoReceiver::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        // 检查数据有效性
        if (datagram.isEmpty())
            continue;

        QImage image;
        if (image.loadFromData(datagram, "JPEG") && !image.isNull())
        {
            emit frameReceived(image);
        }
    }
}
