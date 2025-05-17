#ifndef AUDIORECEIVER_H
#define AUDIORECEIVER_H

#include <QObject>
#include <QUdpSocket>
#include <QAudioSink>
#include <QMediaDevices>

class AudioReceiver : public QObject
{
    Q_OBJECT
public:
    explicit AudioReceiver(QObject *parent = nullptr);
    ~AudioReceiver();

    void startListening();
    void stopListening();

    quint16 getLocalBindPort();

private slots:
    void handleDatagrams();

private:
    QUdpSocket *udpSocket;
    QAudioSink *audioSink = nullptr;
    QIODevice *audioDevice = nullptr;
    quint16 localListenPort = 0;
};

#endif // AUDIORECEIVER_H
