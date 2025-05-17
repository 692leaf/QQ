#ifndef AUDIOSENDER_H
#define AUDIOSENDER_H

#include <QObject>
#include <QUdpSocket>
#include <QAudioSource>

class AudioSender : public QObject
{
    Q_OBJECT
public:
    explicit AudioSender(QObject *parent = nullptr);
    ~AudioSender();

    void updateConnectedIpPort(const QString &targetIp, quint16 targetPort);
    void startBroadcast();
    void stopBroadcast();

private slots:
    void handleAudioDataReady();

private:
    QUdpSocket *udpSocket;
    QAudioSource *audioSource = nullptr;
    QIODevice *audioDevice = nullptr;
    QString targetIp;
    quint16 targetPort;
};

#endif // AUDIOSENDER_H
