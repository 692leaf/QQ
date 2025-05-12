#ifndef VIDEOSENDER_H
#define VIDEOSENDER_H

#include <QObject>
#include <QUdpSocket>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>

class VideoSender : public QObject
{
    Q_OBJECT
public:
    explicit VideoSender(QObject *parent = nullptr);
    ~VideoSender();

    void startCapture();
    void stopCapture();

    void updateConnectedIpPort(const QString& IP,const quint64& port);

private slots:
    void processFrame(const QVideoFrame &frame);

private:
    QUdpSocket *udpSocket;
    QCamera *camera;
    QMediaCaptureSession captureSession;
public:
    quint16 senderLocalPort = 0;
    QString targetAddr = "127.0.0.1";
    quint16 targetPort = 12345;
};

#endif // VIDEOSENDER_H
