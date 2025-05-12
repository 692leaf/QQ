#ifndef SPECIFICCHATWINDOW_H
#define SPECIFICCHATWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QStackedLayout>
#include <QSplitter>
#include <QBoxLayout>
#include <QToolBar>
#include <QLabel>
#include <QMap>
#include "EmojiManager.h"
#include "VideoSender.h"
#include "VideoReceiver.h"
#include "AudioSender.h"
#include "AudioReceiver.h"
#include "BubbleWidget.h"
#include "Tcpclient.h"
#include "LocalDatabase.h"

class FileCache {
public:
    QByteArray data;
    QString fileName;
};

class SpecificChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit SpecificChatWindow(QWidget *parent = nullptr,TcpClient* client=nullptr,LocalDatabase* localBase=nullptr);
    ~SpecificChatWindow();

    void initUi();
    //一:窗口布局
    //上半部分工具栏
    QToolBar* topToolBar();

    //中间部分显示信息窗口
    void OpenChatWidget(const QString& user); // 打开聊天标签页方法

    // 下半部分输入数据窗口
    QWidget* inputWindow();

    // 二:外观调整
    //创建菜单栏
    QToolBar* createTopToolBar();
    QToolBar* createbottomToolBar();

    // 三:功能模块
    void sendFileFunction();
    void fileSendDialog(const QStringList& filePaths);

    // 发送图片模块
    void sendImageFunction();
    QVector<Image> extractImagesFromHtml(QString& html);
    // 接收图片模块
    void generateLocalImageUrls(const Packege &send_Pkg);
    QString generateRichTextInfo(const Packege &send_Pkg);


    void addMessage(const Packege& fullPkg);
    void processFilePackage(const Packege& unit_Pkg);

public slots:
    void handleIncomingPeerRichTextMessage(const Packege& resend_Pkg);
    void handleIncomingPeerFileMessage(const Packege& resend_Pkg);
    void requestLocalChatHistory(const QString& peerUser);
    void handleServerChatHistoryResp(const Packege& resend_Pkg);
    void handleDownloadRequest(const QString &fileID);
    void handleOpenFolder(const QString &fileID);
    void videoSignalingDialog(const Packege& send_Pkg);
    void voiceSignalingDialog(const Packege& send_Pkg);
    void videoCallDialog();
    void voiceCallDialog();
    // void deleteMessage(const QString& id);

private:
    TcpClient* client;
    LocalDatabase* localBase;
    QVBoxLayout* vLayout;
    QHash<QString, bool> messages;

    QToolBar* toolBar;
    QWidget* onScreen_Widget;
    QWidget* inputWidget;

    QLabel* username_Label;// 聊天窗口的用户标签
    QStackedLayout* sLayout;
    QMap<QString, QSplitter*> userChatSplitters; // 存储每个用户对应的聊天记录显示框


    Packege fullPkg;
    // 数据块
    QHash<QString,Packege> activePackege;
    // 文件存储
    QMap<QString, FileCache> receivedFiles; // key: fileID, value: fileData,fileName

    EmojiManager* emojiMgr;

    // 视频
    VideoSender* videoSender;
    VideoReceiver* videoReceiver;
    QDialog* videoSignalingDlg;
    QDialog* videoDlg;

    // 语音
    AudioSender* audioSender;
    AudioReceiver* audioReceiver;
    QDialog* voiceSignalingDlg;
    QDialog* audioDlg;

signals:
    void imageSelectionCompleted(const QStringList& imagePaths);
    void imageUrlsGenerated(const Packege& resend_Pkg);
    void chatListMessageUpdated(const QString& account);
};

#endif // SPECIFICCHATWINDOW_H
