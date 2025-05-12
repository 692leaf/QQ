#include "AudioReceiver.h"
#include <QAudioDevice>

AudioReceiver::AudioReceiver(QObject *parent)
    : QObject{parent}
{
    udpSocket = new QUdpSocket(this);
}

AudioReceiver::~AudioReceiver()
{
    stopListening();
}

void AudioReceiver::startListening()
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

    // 配置音频参数（需与发送端一致）
    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::UInt8);

    // 获取输出设备
    QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
    if (!outputDevice.isFormatSupported(format))
    {
        qWarning() << "Output format not supported";
        return;
    }

    // 初始化音频播放
    audioSink = new QAudioSink(outputDevice, format, this);
    audioDevice = audioSink->start();

    // 连接数据接收信号
    connect(udpSocket, &QUdpSocket::readyRead,this, &AudioReceiver::handleDatagrams);
}

void AudioReceiver::stopListening()
{
    if (audioSink)
    {
        audioSink->stop();
        delete audioSink;
        audioSink = nullptr;
    }
    udpSocket->close();
}

quint16 AudioReceiver::getLocalBindPort()
{
    startListening();
    return localListenPort;
}

void AudioReceiver::handleDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        if (audioDevice)
        {
            audioDevice->write(datagram); // 直接写入播放设备
        }
    }
}
