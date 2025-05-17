#ifndef VIDEORECEIVER_H
#define VIDEORECEIVER_H

#include <QObject>
#include <QUdpSocket>

class VideoReceiver : public QObject
{
    Q_OBJECT
public:
    explicit VideoReceiver(QObject *parent = nullptr);
    ~VideoReceiver();
    void startListening();

    quint16 getLocalBindPort();

signals:
    void frameReceived(const QImage &image);

private slots:
    void readPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    quint16 localListenPort = 0; // 初始化为0，动态分配
};

#endif // VIDEORECEIVER_H
