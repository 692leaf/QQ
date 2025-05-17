#include "VideoSender.h"
#include <QVideoFrame>
#include <QImage>
#include <QBuffer>
#include <QMediaDevices>
#include <QApplication>

VideoSender::VideoSender(QObject *parent)
    : QObject{parent}
{
    udpSocket = new QUdpSocket(this);
    // 端口设为0表示自动分配
    if (udpSocket->bind(QHostAddress::Any, 0))
    {
        senderLocalPort = udpSocket->localPort();
    }
    else
    {
        qDebug() << "绑定失败，错误信息:" << udpSocket->errorString();
    }

    camera = new QCamera(QMediaDevices::defaultVideoInput());
    captureSession.setCamera(camera);
    QVideoSink *sink = new QVideoSink(this);
    captureSession.setVideoSink(sink);
    connect(sink, &QVideoSink::videoFrameChanged, this, &VideoSender::processFrame);
}

VideoSender::~VideoSender()
{
    stopCapture();

    // thread->quit();
    // delete thread;
}

void VideoSender::startCapture()
{
    camera->start();
}

void VideoSender::stopCapture()
{
    camera->stop();
}

void VideoSender::updateConnectedIpPort(const QString &IP, const quint64 &port)
{
    targetAddr = IP;
    targetPort = port;
}

void VideoSender::processFrame(const QVideoFrame &frame)
{
    QImage image = frame.toImage();
    if (image.isNull())
        return;

    // 转换为 RGB32 格式
    image = image.convertToFormat(QImage::Format_RGB32);

    // 转换为 JPEG 格式
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    if (image.save(&buffer, "JPEG", 30)) // 检查保存是否成功
    {
        udpSocket->writeDatagram(byteArray, QHostAddress(targetAddr), targetPort);
    }
}
