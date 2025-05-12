#include "AudioSender.h"
#include <QAudioDevice>
#include <QMediaDevices>

AudioSender::AudioSender(QObject *parent)
    : QObject{parent}
{
    udpSocket = new QUdpSocket(this);
}

AudioSender::~AudioSender()
{
    stopBroadcast();
}

void AudioSender::updateConnectedIpPort(const QString &ip, quint16 port)
{
    targetIp = ip;
    targetPort = port;
}

void AudioSender::startBroadcast()
{
    // 配置音频参数
    QAudioFormat format;
    format.setSampleRate(16000);      // 16kHz采样率
    format.setChannelCount(1);        // 单声道
    format.setSampleFormat(QAudioFormat::UInt8); // 8位无符号

    // 获取音频设备
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (!inputDevice.isFormatSupported(format)) {
        qWarning() << "Format not supported, using preferred format";
        format = inputDevice.preferredFormat();
    }

    // 初始化音频源
    audioSource = new QAudioSource(inputDevice, format, this);
    audioDevice = audioSource->start();

    // 连接数据可用信号
    connect(audioDevice, &QIODevice::readyRead,
            this, &AudioSender::handleAudioDataReady);
}

void AudioSender::stopBroadcast()
{
    if (audioSource) {
        audioSource->stop();
        delete audioSource;
        audioSource = nullptr;
    }
    udpSocket->close();
}

void AudioSender::handleAudioDataReady()
{
    // 读取音频数据并发送
    QByteArray audioData = audioDevice->readAll();
    if(!audioData.isEmpty())
    {
        udpSocket->writeDatagram(audioData, QHostAddress(targetIp), targetPort);
    }
}
