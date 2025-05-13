#include "SpecificChatWindow.h"
#include "BubbleWidget.h"
#include <QApplication>
#include <QPushButton>
#include <QTextCursor>
#include <QRegularExpression>
#include <QAction>
#include <QTextEdit>
#include <QListView>
#include <QStandardItem>
#include <QFileDialog>
#include <QUuid>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDesktopServices>
#include <QTextBlock>

SpecificChatWindow::SpecificChatWindow(QWidget *parent,TcpClient* client,LocalDatabase* localBase)
    : QWidget{parent},
    client(client),
    localBase(localBase),
    sLayout(new QStackedLayout(this)),
    emojiMgr(new EmojiManager)
{
    // 图片的接收与显示
    connect(client,&TcpClient::messageReceived,this,&SpecificChatWindow::handleIncomingPeerRichTextMessage);
    connect(this,&SpecificChatWindow::imageUrlsGenerated,[this](const Packege& url_Pkg){
        if(url_Pkg.type==SEND_RICHTEXT_MESSAGE || url_Pkg.type==LOAD_CHATHISTORY) addMessage(url_Pkg);
    });

    connect(client,&TcpClient::messageReceived,this,&SpecificChatWindow::handleIncomingPeerFileMessage);
    connect(client,&TcpClient::messageReceived,this,&SpecificChatWindow::handleServerChatHistoryResp);
    initUi();
}

SpecificChatWindow::~SpecificChatWindow()
{
    delete emojiMgr;
}


/***********************************************************************************************
*                                 实现窗口整体布局                                               *
*                                 1.顶部菜单栏                                                  *
*                                 2.显示窗口与输入框                                             *
***********************************************************************************************/
void SpecificChatWindow::initUi()
{
    toolBar=topToolBar();
    onScreen_Widget = new QWidget(this); // 创建QStackWidget

    //创建布局
    vLayout=new QVBoxLayout(this);
    //添加窗口
    vLayout->addWidget(toolBar);
    vLayout->addWidget(onScreen_Widget);

    //中间窗口的添加stackedLayout布局
    onScreen_Widget->setLayout(sLayout);

    this->setLayout(vLayout);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

}


QToolBar *SpecificChatWindow::topToolBar()
{
    QToolBar* topNavigationToolBar =createTopToolBar();
    return topNavigationToolBar;
}


void SpecificChatWindow::OpenChatWidget(const QString &user)
{
    // 切换标题栏中的用户名
    username_Label->setText(user);
    if (userChatSplitters.contains(user))
    {
        // 如果窗口已存在，切换到该窗口
        int index = sLayout->indexOf(userChatSplitters[user]);
        sLayout->setCurrentIndex(index);
        return;
    }


    // 若没有该用户的窗口，则新建
    // 创建垂直分离器
    QSplitter* splitter=new QSplitter(Qt::Vertical);
    QListWidget* chatListWidget = new QListWidget;// 消息列表控件
    QWidget* inputWidget=inputWindow();

    // 列表控件属性设置
    chatListWidget->setVerticalScrollMode(QListWidget::ScrollPerPixel); // 平滑滚动
    chatListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 禁用水平滚动条

    // 向分离器中加窗口
    splitter->addWidget(chatListWidget);
    splitter->addWidget(inputWidget);

    // 设置Splitter的默认分配比例（上部分占60%，下部分占40%）
    splitter->setSizes({ static_cast<int>(height() * 0.6), static_cast<int>(height() * 0.4) });

    sLayout->addWidget(splitter);
    userChatSplitters[user] = splitter;

    // 加载聊天历史
    loadLocalChatHistory(user);
}

QWidget *SpecificChatWindow::inputWindow()
{
    QWidget* w=new QWidget(this);
    QToolBar* bottomActionToolBar=createbottomToolBar();
    QTextEdit* textEdit=new QTextEdit(this);
    QPushButton* sendButton=new QPushButton("发送",this);

    // 文本设置
    textEdit->setAcceptRichText(true);                            // 允许富文本
    textEdit->setTextInteractionFlags(Qt::TextEditorInteraction); // 允许编辑

    // 加弹簧
    QWidget *spring = new QWidget(this);
    spring->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    // 对弹簧和控件水平布局
    QHBoxLayout* hLayout=new QHBoxLayout;
    hLayout->addWidget(spring);
    hLayout->addWidget(sendButton);

    // 再垂直布局
    QVBoxLayout* vLayout=new QVBoxLayout;
    vLayout->addWidget(bottomActionToolBar);
    vLayout->addWidget(textEdit);
    vLayout->addLayout(hLayout);

    w->setLayout(vLayout);

    /*======================= 添加表情包 =====================*/

    // 输入框中添加表情包
    connect(emojiMgr,&EmojiManager::emojiClicked,[this,textEdit](const QString& emojiPath){
        // 插入表情包到输入框
        QTextCursor cursor = textEdit->textCursor();
        QTextImageFormat imageFormat;

        // 加载表情包
        QPixmap pixmap(emojiPath);
        if (!pixmap.isNull())
        {
            imageFormat.setName(emojiPath);  // 直接设置表情包路径
            imageFormat.setWidth(pixmap.width());
            imageFormat.setHeight(pixmap.height());
            cursor.insertImage(imageFormat); // 插入自定义表情包对象
        }
    });

    /*======================= 添加图片 =====================*/

    // 输入框中添加图片
    connect(this,&SpecificChatWindow::imageSelectionCompleted,[this,textEdit](const QStringList& imagePaths){
        for(const QString& path:imagePaths)
        {
            // 插入图片到输入框
            QTextCursor cursor = textEdit->textCursor();
            QTextImageFormat imageFormat;

            // 加载图片并缩放（例如缩放到100x100）
            QPixmap pixmap(path);
            if (!pixmap.isNull())
            {
                pixmap = pixmap.scaled(100, 100, Qt::KeepAspectRatio);
                imageFormat.setName(path);  // 直接设置图像路径
                imageFormat.setWidth(pixmap.width());
                imageFormat.setHeight(pixmap.height());
                cursor.insertImage(imageFormat); // 插入自定义图像对象
            }
        }
    });

    /*==================== 上传图片 ===================*/

    // 点击按钮上传图片
    connect(sendButton,&QPushButton::clicked,[this,textEdit](){
        if(textEdit->document()->isEmpty()) return;

        // 1. 提取富文本中的图片资源和位置,并显示文本到本地窗口上
        QString html = textEdit->toHtml();
        QVector<Image> imageInfo = extractImagesFromHtml(html); // 返回<唯一标识符, 图片本地路径>

        // 2. 构建图片上传请求包
        Packege send_Pkg;
        send_Pkg.type = SEND_RICHTEXT_MESSAGE;
        send_Pkg.sender = qApp->property("username").toString();
        send_Pkg.receiver = username_Label->text();
        send_Pkg.messageInfo.sender_del = 0;
        send_Pkg.messageInfo.receiver_del = 0;
        send_Pkg.messageInfo.message_type=RICHTEXTCONTENT_TRANSFERS;
        send_Pkg.messageInfo.images = imageInfo; // 直接使用已解析的图片信息
        send_Pkg.messageInfo.richText = html;    // 存储处理后的HTML（带占位符）

        // 3. 填充图片二进制数据
        for(int i=0;i<imageInfo.size();i++)
        {
            QFileInfo fileInfo(imageInfo[i].localPath);

            send_Pkg.messageInfo.images[i].imageName = fileInfo.fileName(); // 文件名（含扩展名）
            send_Pkg.messageInfo.images[i].imageType = fileInfo.suffix().toLower(); // 文件类型（png等）

            // 安全读取图片数据
            QFile file(imageInfo[i].localPath);
            if (file.open(QIODevice::ReadOnly))
            {
                send_Pkg.messageInfo.images[i].imageData = file.readAll();
                file.close();
            }
            else
            {
                qWarning() << "无法打开图片文件:" << imageInfo[i].localPath;
                send_Pkg.messageInfo.images[i].imageData.clear(); // 清空无效数据
            }
        }

        // 4. 发送数据包
        client->sendMessage(send_Pkg);

        // 5.显示到本地窗口上
        Packege url_Pkg;
        url_Pkg=send_Pkg;
        url_Pkg.messageInfo.richText=textEdit->toHtml();
        generateLocalImageUrls(url_Pkg);

        textEdit->clear();
    });

    return w;
}


/***********************************************************************************************
*                                 实现菜单栏外观                                                 *
*                                 1.顶部菜单外观                                                 *
*                                 2.输入框菜单外观                                               *
***********************************************************************************************/
QToolBar *SpecificChatWindow::createTopToolBar()
{
    QToolBar* topNavigationToolBar=new QToolBar(this);
    // Voice Call, Video Call, Screen Sharing, Invite to Join the Group, Group Settings
    QAction* voiceCall=new QAction(QIcon(":/resource/image/voiceCall.png"),"",this);
    QAction* videoCall=new QAction(QIcon(":/resource/image/videoCall.png"),"",this);
    QAction* screenSharing=new QAction(QIcon(":/resource/image/screenSharing.png"),"",this);
    QAction* joinGroup=new QAction(QIcon(":/resource/image/joinGroup.png"),"",this);
    QAction* groupSettings=new QAction(QIcon(":/resource/image/groupSettings.png"),"",this);

    //向tBar中添加组件
    username_Label=new QLabel("",this);
    topNavigationToolBar->addWidget(username_Label);

    //加弹簧
    QWidget *spring = new QWidget(this);
    spring->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    topNavigationToolBar->addWidget(spring);

    topNavigationToolBar->addAction(voiceCall);
    topNavigationToolBar->addAction(videoCall);
    topNavigationToolBar->addAction(screenSharing);
    topNavigationToolBar->addAction(joinGroup);
    topNavigationToolBar->addAction(groupSettings);

    //设置提示词
    voiceCall->setToolTip("语音通话");
    videoCall->setToolTip("视频通话");
    screenSharing->setToolTip("屏幕共享");
    joinGroup->setToolTip("发起群聊");
    groupSettings->setToolTip("设置");


    //发送方发出视频请求
    connect(videoCall,&QAction::triggered,[this](){
        // 先创建 VideoReceiver 和 VideoSender
        videoReceiver = new VideoReceiver(this);
        videoSender = new VideoSender(this);

        audioReceiver = new AudioReceiver(this);
        audioSender = new AudioSender(this);


        // 再显示视频对话框
        videoCallDialog();

        QString targetUser = username_Label->text();
        //发出视频通话请求包
        {
            // 1.信令,通过tcp发送请求
            Packege send_Pkg;
            send_Pkg.type=VIDEO_SIGNALING_MESSAGE_SEND;
            send_Pkg.sender=qApp->property("username").toString();
            send_Pkg.receiver=targetUser;
            send_Pkg.videoPort=videoReceiver->getLocalBindPort();
            send_Pkg.audioPort=audioReceiver->getLocalBindPort();
            client->sendMessage(send_Pkg);

            // 2.等待对方同意后才初始化,发送视频和语音
            connect(client,&TcpClient::messageReceived,[this](const Packege& resend_Pkg){
                if(resend_Pkg.type!=VIDEO_SIGNALING_MESSAGE_RECEIVER) return;

                //对方接受通话
                if(resend_Pkg.videoPort)
                {
                    //更新目标ip和端口，并打开摄像头
                    videoSender->updateConnectedIpPort(resend_Pkg.ip,resend_Pkg.videoPort);
                    videoSender->startCapture();

                    //更新目标ip和端口，并打开麦克风
                    audioSender->updateConnectedIpPort(resend_Pkg.ip,resend_Pkg.audioPort);
                    audioSender->startBroadcast();
                }
                else//对方拒绝通话
                {
                    if (videoDlg)
                    {
                        videoDlg->close();
                        videoDlg->deleteLater();
                        videoDlg = nullptr;
                    }
                }
            });
        }
    });
    //接收方处理视频请求
    connect(client,&TcpClient::messageReceived,[this](const Packege& send_Pkg){
        if(send_Pkg.type!=VIDEO_SIGNALING_MESSAGE_SEND) return;
        //弹出信令窗口
        videoSignalingDialog(send_Pkg);
    });



    //发送方发出语音请求
    connect(voiceCall,&QAction::triggered,[this](){
        voiceCallDialog();
        audioReceiver = new AudioReceiver(this);
        audioSender = new AudioSender(this);

        QString targetUser = username_Label->text();
        //发出语音通话请求包
        {
            // 1.信令,通过tcp发送请求
            Packege send_Pkg;
            send_Pkg.type=AUDIO_SIGNALING_MESSAGE_SEND;
            send_Pkg.sender=qApp->property("username").toString();
            send_Pkg.receiver=targetUser;
            send_Pkg.audioPort=audioReceiver->getLocalBindPort();
            client->sendMessage(send_Pkg);

            // 2.等待对方同意后才初始化,发送视频和语音
            connect(client,&TcpClient::messageReceived,[this](const Packege& resend_Pkg){
                if(resend_Pkg.type!=AUDIO_SIGNALING_MESSAGE_RECEIVER) return;

                // 对方接受通话
                if(resend_Pkg.audioPort)
                {
                    // 更新目标ip和端口，并打开麦克风
                    audioSender->updateConnectedIpPort(resend_Pkg.ip,resend_Pkg.audioPort);
                    audioSender->startBroadcast();
                }
                else // 对方拒绝通话
                {
                    if(audioDlg)
                    {
                        audioDlg->close();
                        audioDlg->deleteLater();
                        audioDlg = nullptr;
                    }
                }
            });
        }
    });

    //接收方处理语音请求
    connect(client,&TcpClient::messageReceived,[this](const Packege& send_Pkg){
        if(send_Pkg.type!=AUDIO_SIGNALING_MESSAGE_SEND) return;
        //弹出信令窗口
        voiceSignalingDialog(send_Pkg);
    });

    return topNavigationToolBar;
}

QToolBar *SpecificChatWindow::createbottomToolBar()
{
    QToolBar* bottomActionToolBar=new QToolBar;
    //Emoji, Screenshot, Send File, Send Image, Voice Message
    //设置组件并添加
    QAction* emoji = new QAction(QIcon(":/resource/image/emoji.png"),"",this);
    QAction* screenShot = new QAction(QIcon(":/resource/image/screenShot.png"),"",this);
    QAction* sendFile = new QAction(QIcon(":/resource/image/sendFile.png"),"",this);
    QAction* sendImage = new QAction(QIcon(":/resource/image/sendImage.png"),"",this);
    QAction* voiceMessage = new QAction(QIcon(":/resource/image/voiceMessage.png"),"",this);
    QAction* chatRecord = new QAction(QIcon(":/resource/image/chatRecord.png"),"",this);

    bottomActionToolBar->addAction(emoji);
    bottomActionToolBar->addAction(screenShot);
    bottomActionToolBar->addAction(sendFile);
    bottomActionToolBar->addAction(sendImage);
    bottomActionToolBar->addAction(voiceMessage);

    //加弹簧
    QWidget *spring = new QWidget(this);
    spring->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bottomActionToolBar->addWidget(spring);

    bottomActionToolBar->addAction(chatRecord);

    //设置提示词
    emoji->setToolTip("表情");
    screenShot->setToolTip("截图");
    sendFile->setToolTip("发送文件");
    sendImage->setToolTip("发送图片");
    voiceMessage->setToolTip("语音消息");
    chatRecord->setToolTip("聊天记录");

    // SpecificChatWindow.cpp - inputWindow() 函数中的表情按钮连接
    connect(emoji,&QAction::triggered,[this, emoji](){
        if (emojiMgr->isVisible())
        {
            emojiMgr->hide();
        }
        else
        {
            // 获取当前聊天窗口的splitter
            QString currentUser = username_Label->text();
            if (!userChatSplitters.contains(currentUser)) return;

            QSplitter* splitter = userChatSplitters[currentUser];
            QWidget* chatListWidget = splitter->widget(0);
            QWidget* inputArea = splitter->widget(1);

            // 计算表情窗口的位置
            QPoint listLeft = chatListWidget->mapToGlobal(QPoint(0, 0));
            QPoint inputTop = inputArea->mapToGlobal(QPoint(0, 0));

            // 设置位置：左侧对齐聊天列表，顶部贴输入框上方
            emojiMgr->move(listLeft.x(), inputTop.y() - emojiMgr->height());
            emojiMgr->show();
        }
    });
    connect(sendFile,&QAction::triggered,this,&SpecificChatWindow::sendFileFunction);
    connect(sendImage,&QAction::triggered,this,&SpecificChatWindow::sendImageFunction);

    return bottomActionToolBar;
}


/***********************************************************************************************
*                               小组件功能模块的实现                                              *
*                                 1.文件传输                                                    *
*                                 2.图片功能                                                    *
*                                 2.文本图片传输                                                 *
*                                 3.文件接收与存储                                               *
*                                 4.视频通话                                                    *
*                                 5.语音通话                                                    *
***********************************************************************************************/
void SpecificChatWindow::sendFileFunction()
{
    //获取文件路径
    QStringList filePaths=QFileDialog::getOpenFileNames(this,"选择文件","D:/","所有文件(*.*)");
    if(!filePaths.isEmpty())
    {
        //弹出窗口
        fileSendDialog(filePaths);
    }
    else
    {
        qDebug()<<"未选择文件";
    }
}

void SpecificChatWindow::fileSendDialog(const QStringList& filePaths)
{
    QDialog* dlog=new QDialog(this);


    //创建布局
    auto hLayout=new QHBoxLayout;
    auto sendBtn=new QPushButton("发送",dlog);
    auto cancelBtn=new QPushButton("取消",dlog);
    hLayout->addWidget(sendBtn);
    hLayout->addWidget(cancelBtn);

    //创建布局
    auto vLayout=new QVBoxLayout;
    //创建标题栏
    QLabel* titleLabel=new QLabel("发送给醉酒梦月",dlog);
    //创建视图列表
    auto listView=new QListView(dlog);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto model=new QStandardItemModel(dlog);
    listView->setModel(model);

    for(const QString& path:filePaths)
    {
        QFileInfo fileInfo(path);
        QString fileName=fileInfo.fileName();
        QString fileType=fileInfo.suffix().toLower();
        QStandardItem* item=new QStandardItem(
            QIcon(":/resource/image/fileType/"+fileType+".png"),//添加文件图标
            fileName+"\n"+
            QString::number(fileInfo.size()/1024)+" KB");//显示文件大小
        model->appendRow(item);
    }

    //将视图添加进布局
    vLayout->addWidget(titleLabel);
    vLayout->addWidget(listView);
    vLayout->addLayout(hLayout);

    dlog->setLayout(vLayout);
    dlog->show();

    //发送消息
    connect(sendBtn,&QPushButton::clicked,[this,dlog,filePaths](){ 
        // 预处理：收集零大小文件
        QStringList zeroSizeFiles;
        for(const QString& path:filePaths)
        {
            //发送文件
            QFile file(path);
            if(file.open(QIODevice::ReadOnly))
            {
                // 生成唯一文件ID（示例使用QUuid）
                QString fileID = QUuid::createUuid().toString();

                const int blockSize=1024*1024; //每次读取1MB
                // 发送元数据包
                Packege metaPkg;
                /*元信息*/
                metaPkg.type=SEND_FILE_MESSAGE;
                metaPkg.sender=qApp->property("username").toString();
                metaPkg.receiver=username_Label->text();

                QFileInfo fileInfo(path);
                /*0大小文件特殊处理*/
                if(fileInfo.size()==0)
                {
                    zeroSizeFiles.push_back(fileInfo.fileName());
                    continue; // 跳过当前文件，继续处理下一个
                }
                metaPkg.messageInfo.message_type=BLOCK_FILE_TRANSFERS;
                metaPkg.messageInfo.file.fileName=fileInfo.fileName();
                metaPkg.messageInfo.file.fileSize=fileInfo.size();
                metaPkg.messageInfo.file.fileType=fileInfo.suffix().toLower();
                metaPkg.messageInfo.sender_del=0;
                metaPkg.messageInfo.receiver_del=0;


                //该文件唯一标识符
                metaPkg.messageInfo.file.fileID=fileID;
                //0表示元数据包
                metaPkg.messageInfo.file.currentBlock=0;
                //总块数计算
                metaPkg.messageInfo.file.totalBlocks=ceil((double)fileInfo.size()/blockSize);
                //发送元消息
                client->sendMessage(metaPkg);

                // 1.构建完整包,存储消息到本地数据库中
                processFilePackage(metaPkg);

                int blockIndex=1;
                while(!file.atEnd())
                {
                    Packege dataPkg;  // 每次创建新对象
                    dataPkg = metaPkg; // 继承元数据
                    dataPkg.messageInfo.file.currentBlock=blockIndex++;
                    dataPkg.messageInfo.file.fileContent=file.read(blockSize);
                    //发送块信息
                    client->sendMessage(dataPkg);

                    // 2.构建完整包,存储消息到本地数据库中
                    processFilePackage(dataPkg);
                }
                file.close();
            }
        }
        dlog->accept();

        // 如果有零大小文件，统一弹窗提示
        if (!zeroSizeFiles.isEmpty())
        {
            QString warningMsg = "以下文件大小为零，已跳过：\n";
            warningMsg += zeroSizeFiles.join("\n"); // 合并所有文件名
            QMessageBox::warning(this, "警告", warningMsg);
        }
    });
    //取消,关闭对话框
    connect(cancelBtn,&QPushButton::clicked,dlog,&QDialog::reject);
}

void SpecificChatWindow::sendImageFunction()
{
    //获取图片路径
    QStringList imagePaths=QFileDialog::getOpenFileNames(
        this,"选择图片","D:/","图片(*gif;*png;*jpg;*jpeg;*.webp;*avif;*.bmp;*sharpp;*.apng)");
    if(!imagePaths.isEmpty())
    {
        emit imageSelectionCompleted(imagePaths);
    }
    else
    {
        qDebug()<<"未选择图片";
    }
}

// 提取图片并生成唯一占位符
QVector<Image> SpecificChatWindow::extractImagesFromHtml(QString &html)
{
    QVector<Image> images;
    QTextDocument doc;
    doc.setHtml(html);

    QTextBlock block = doc.begin();
    while (block.isValid()) {
        QTextBlock::iterator it;
        for (it = block.begin(); !it.atEnd(); ++it) {
            QTextFragment fragment = it.fragment();
            if (fragment.charFormat().isImageFormat()) {
                QTextImageFormat format = fragment.charFormat().toImageFormat();
                QString imagePath = format.name();

                if (imagePath.startsWith(":/resource/image/emojis/")) {
                    continue; // 不处理表情包
                }

                // 生成唯一ID和占位符（用UUID）
                Image img;
                img.localPath = format.name(); // 通过 name() 获取路径
                // 替换为占位符
                img.uniqueId = QUuid::createUuid().toString(QUuid::WithoutBraces);
                QString placeholder = QString("IMG_PLACEHOLDER_%1").arg(img.uniqueId);
                format.setName(placeholder);
                QTextCursor cursor(&doc);

                // 更新文档格式
                cursor.setPosition(fragment.position());
                cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
                cursor.mergeCharFormat(format);

                images.append(img);
            }
        }
        block = block.next();
    }

    html = doc.toHtml();
    return images;
}

void SpecificChatWindow::generateLocalImageUrls(const Packege &resend_Pkg)
{
    // 将图片保存到本地,返回url
    Packege url_Pkg=resend_Pkg;
    url_Pkg.messageInfo.images=localBase->local_MediaTable_Create_MediaRecord(resend_Pkg); // 将图片保存到本地数据库
    // 生成富文本
    url_Pkg.messageInfo.richText=generateRichTextInfo(url_Pkg);
    localBase->local_ChrTable_Create_ChatRecord(url_Pkg); // 将富文本保存到本地数据库

    // 更新最新消息
    QString account = fullPkg.sender != qApp->property("username").toString() ?
                          fullPkg.sender : fullPkg.receiver;
    emit chatListMessageUpdated(account);

    emit imageUrlsGenerated(url_Pkg);
}

QString SpecificChatWindow::generateRichTextInfo(const Packege &url_Pkg)
{
    // 获取当前富文本（带占位符）
    QString html = url_Pkg.messageInfo.richText;
    QVector<Image> images=url_Pkg.messageInfo.images;
    for (int i = 0; i < images.size(); ++i) {
        QString placeholder = QString("IMG_PLACEHOLDER_%1")
        .arg(images[i].uniqueId);

        html.replace(placeholder, images[i].url);
    }

    return html;
}


void SpecificChatWindow::addMessage(const Packege& fullPkg)
{
    QString account=fullPkg.sender==qApp->property("username").toString()?fullPkg.receiver:fullPkg.sender;
    if (!userChatSplitters.contains(account))
    {
        OpenChatWidget(account);
    }


    // 从 splitter 中获取 QListWidget
    QSplitter* splitter = userChatSplitters[account];
    QListWidget* chatListWidget = qobject_cast<QListWidget*>(splitter->widget(0));


    QListWidgetItem* item=new QListWidgetItem(chatListWidget);
    BubbleWidget* bubble=new BubbleWidget(fullPkg);

    // 存储文件内容和文件名到本地
    int message_Type=fullPkg.messageInfo.message_type;
    if(message_Type==BLOCK_FILE_TRANSFERS)
    {
        QString fileID=fullPkg.messageInfo.file.fileID;
        QByteArray fullData=fullPkg.messageInfo.file.fileContent;
        QString fileName=fullPkg.messageInfo.file.fileName;
        receivedFiles[fileID]={fullData,fileName};
    }

    // 设置列表项大小
    item->setSizeHint(bubble->sizeHint());

    // 添加项到列表
    chatListWidget->addItem(item);
    chatListWidget->setItemWidget(item,bubble);

    //自动滚动到底部
    chatListWidget->scrollToBottom();

    // 文件下载与打开
    connect(bubble, &BubbleWidget::downloadRequested,
            this, &SpecificChatWindow::handleDownloadRequest);
    connect(bubble, &BubbleWidget::openFolderRequested,
            this, &SpecificChatWindow::handleOpenFolder);
}

void SpecificChatWindow::processFilePackage(const Packege &unit_Pkg)
{
    //是否接收完整
    bool isTransferComplete=0;

    const QString fileID=unit_Pkg.messageInfo.file.fileID;
    if(unit_Pkg.messageInfo.file.currentBlock==0)
    {
        activePackege[unit_Pkg.messageInfo.file.fileID]=unit_Pkg;
        activePackege[fileID].messageInfo.file.receivedBlocks=0;
        activePackege[fileID].messageInfo.file.blocks.clear(); // 显式初始化
        activePackege[fileID].messageInfo.sender_del=unit_Pkg.messageInfo.sender_del;
        activePackege[fileID].messageInfo.receiver_del=unit_Pkg.messageInfo.receiver_del;
    }
    else
    {
        const int currentBlock=unit_Pkg.messageInfo.file.currentBlock;
        activePackege[fileID].messageInfo.file.blocks[currentBlock]=
            unit_Pkg.messageInfo.file.fileContent;
        int receiveBlocks=++activePackege[fileID].messageInfo.file.receivedBlocks;
        int totalBlocks=activePackege[fileID].messageInfo.file.totalBlocks;

        if(receiveBlocks==totalBlocks)
        {
            // 按序号合并块
            QByteArray fullData;
            for(int blockIndex=1;blockIndex<=totalBlocks;blockIndex++)
            {
                QByteArray currentData=activePackege[fileID].messageInfo.file.blocks[blockIndex];
                fullData+=currentData;
            }

            QString fileName=activePackege[fileID].messageInfo.file.fileName;
            int totalSize=activePackege[fileID].messageInfo.file.fileSize;

            if(fullData.size()==totalSize)
            {
                fullPkg=activePackege[fileID];
                fullPkg.messageInfo.file.fileContent=fullData;
                // 资源释放
                activePackege[fileID].messageInfo.file.blocks.clear();
                activePackege.remove(fileID);
                isTransferComplete=1;
            }
            else
            {
                // 请求完整重传
                // requestFullResend(fileID);
            }
        }
    }

    //文件完整后放行
    if(isTransferComplete)
    {
        if(fullPkg.type==SEND_FILE_MESSAGE||fullPkg.type==LOAD_CHATHISTORY)
        {
            addMessage(fullPkg);
        }
        if(fullPkg.type==SEND_FILE_MESSAGE||fullPkg.type==ASYNC_FETCH_CHATHISTORY)
        {
            // 存储文件消息到本地数据库
            localBase->local_ChrTable_Create_ChatRecord(fullPkg);

            // 更新最新消息
            QString account = fullPkg.sender != qApp->property("username").toString() ?
                                  fullPkg.sender : fullPkg.receiver;
            emit chatListMessageUpdated(account);
        }
    }
}

void SpecificChatWindow::handleIncomingPeerRichTextMessage(const Packege &resend_Pkg)
{
    if(resend_Pkg.type!=SEND_RICHTEXT_MESSAGE) return;

    generateLocalImageUrls(resend_Pkg);
}

// 处理对方发来的消息
void SpecificChatWindow::handleIncomingPeerFileMessage(const Packege& resend_Pkg)
{
    if(resend_Pkg.type!=SEND_FILE_MESSAGE) return;

    processFilePackage(resend_Pkg);
}

void SpecificChatWindow::loadLocalChatHistory(const QString& peerUser)
{
    QVector<Packege> chatHistory_Pkgs=localBase->local_ChrTable_Load_ChatHistory(peerUser);
    for(auto chatHistory_Pkg:chatHistory_Pkgs)
    {
        if(chatHistory_Pkg.messageInfo.message_type==RICHTEXTCONTENT_TRANSFERS)
        {
            // 显示图片
            addMessage(chatHistory_Pkg);
        }
        else if(chatHistory_Pkg.messageInfo.message_type==BLOCK_FILE_TRANSFERS)
        {
            // 合并块，并显示
            processFilePackage(chatHistory_Pkg);
        }
    }
}

void SpecificChatWindow::handleServerChatHistoryResp(const Packege &chatSyncPkg)
{
    if(chatSyncPkg.type!=ASYNC_FETCH_CHATHISTORY) return;

    int message_type=chatSyncPkg.messageInfo.message_type;
    switch(message_type)
    {
    case RICHTEXTCONTENT_TRANSFERS:
        generateLocalImageUrls(chatSyncPkg);
    break;
    case BLOCK_FILE_TRANSFERS:
        processFilePackage(chatSyncPkg);
    break;
    default:
        qDebug()<<"message_type on handleServerChatHistoryResp is None!";
    break;
    }
}

void SpecificChatWindow::handleDownloadRequest(const QString &fileID)
{
    if (!receivedFiles.contains(fileID)) {
        qDebug() << "文件不存在或已过期";
        return;
    }

    // 获取系统的下载目录
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    if (!downloadPath.isEmpty())
    {
        // 获取文件名
        QString fileName = receivedFiles[fileID].fileName;
        //拼凑完整路径
        QString filePath = QDir(downloadPath).filePath(fileName);
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(receivedFiles[fileID].data);
            file.close();
            QMessageBox::information(this, "成功", "文件已保存至：" + filePath);
        }
        else
        {
            QMessageBox::warning(this, "错误", "无法创建文件");
        }
    }
}

void SpecificChatWindow::handleOpenFolder(const QString &fileID)
{
    if (!receivedFiles.contains(fileID))
    {
        QMessageBox::warning(this, "错误", "文件未找到");
        return;
    }

    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    // 拼接完整路径
    QString filePath = QDir(downloadPath).filePath(receivedFiles[fileID].fileName); //  receivedFiles 的存储的对象fileName

    // 检查文件是否存在
    if (!QFile::exists(filePath))
    {
        QMessageBox::warning(this, "错误", "文件路径可能已改变或文件已被删除");
        return;
    }

    // 使用 QDesktopServices 打开文件所在目录
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists())
    {
        // 跨平台打开文件夹
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
    }
    else
    {
        QMessageBox::warning(this, "错误", "文件路径不存在: " + filePath);
    }
}

void SpecificChatWindow::videoSignalingDialog(const Packege& send_Pkg)
{
    videoSignalingDlg=new QDialog(this);

    QHBoxLayout* hLayout = new QHBoxLayout(videoSignalingDlg);
    QLabel* call_Display_Label = new QLabel("来电");
    QPushButton* acceptButton = new QPushButton(QIcon(),"接受");
    QPushButton* closeButton = new QPushButton(QIcon(),"拒绝");

    hLayout->addWidget(call_Display_Label);
    hLayout->addWidget(acceptButton);
    hLayout->addWidget(closeButton);

    videoSignalingDlg->setLayout(hLayout);
    videoSignalingDlg->show();


    connect(acceptButton,&QPushButton::clicked,[this,send_Pkg](){
        videoSignalingDlg->accept();

        // 先初始化
        videoReceiver = new VideoReceiver(this);
        videoSender = new VideoSender(this);

        audioReceiver = new AudioReceiver(this);
        audioSender = new AudioSender(this);

        // 再显示视频对话框
        videoCallDialog();

        {
            // 1. 通过信令返回网络信息
            Packege resend_Pkg;
            resend_Pkg.type=VIDEO_SIGNALING_MESSAGE_RECEIVER;
            resend_Pkg.sender=send_Pkg.sender;
            resend_Pkg.receiver=send_Pkg.receiver;
            resend_Pkg.videoPort=videoReceiver->getLocalBindPort();
            resend_Pkg.audioPort=audioReceiver->getLocalBindPort();

            client->sendMessage(resend_Pkg);

            // 2. 启动本地摄像头和麦克风，给对方发送udp数据包
            videoSender->updateConnectedIpPort(send_Pkg.ip, send_Pkg.videoPort);
            videoSender->startCapture();

            audioSender->updateConnectedIpPort(send_Pkg.ip, send_Pkg.audioPort);
            audioSender->startBroadcast();
        }

    });

    connect(closeButton,&QPushButton::clicked,[this,send_Pkg](){
        videoSignalingDlg->close();

        // 拒绝视频对话,关闭发送方的聊天窗口
        Packege resend_Pkg;
        resend_Pkg.type=VIDEO_SIGNALING_MESSAGE_RECEIVER;
        resend_Pkg.sender=send_Pkg.sender;
        resend_Pkg.receiver=send_Pkg.receiver;
        resend_Pkg.videoPort=0;
        resend_Pkg.audioPort=0;

        client->sendMessage(resend_Pkg);
    });
}

void SpecificChatWindow::voiceSignalingDialog(const Packege &send_Pkg)
{
    voiceSignalingDlg=new QDialog(this);

    QHBoxLayout* hLayout = new QHBoxLayout(voiceSignalingDlg);
    QLabel* call_Display_Label = new QLabel("来电");
    QPushButton* acceptButton = new QPushButton(QIcon(),"接受");
    QPushButton* closeButton = new QPushButton(QIcon(),"拒绝");

    hLayout->addWidget(call_Display_Label);
    hLayout->addWidget(acceptButton);
    hLayout->addWidget(closeButton);

    voiceSignalingDlg->setLayout(hLayout);
    voiceSignalingDlg->show();


    connect(acceptButton,&QPushButton::clicked,[this,send_Pkg](){
        voiceSignalingDlg->accept();
        voiceCallDialog();

        //初始化
        audioReceiver = new AudioReceiver(this);
        audioSender = new AudioSender(this);

        {
            // 1. 通过信令返回网络信息
            Packege resend_Pkg;
            resend_Pkg.type=AUDIO_SIGNALING_MESSAGE_RECEIVER;
            resend_Pkg.sender=send_Pkg.sender;
            resend_Pkg.receiver=send_Pkg.receiver;
            resend_Pkg.audioPort=audioReceiver->getLocalBindPort();

            client->sendMessage(resend_Pkg);

            // 2. 启动本地麦克风，给对方发送udp数据包
            audioSender->updateConnectedIpPort(send_Pkg.ip, send_Pkg.audioPort);
            audioSender->startBroadcast();
        }

    });

    connect(closeButton,&QPushButton::clicked,[this,send_Pkg](){
        voiceSignalingDlg->close();

        // 拒绝语音对话,关闭发送方的聊天窗口
        Packege resend_Pkg;
        resend_Pkg.type=AUDIO_SIGNALING_MESSAGE_RECEIVER;
        resend_Pkg.sender=send_Pkg.sender;
        resend_Pkg.receiver=send_Pkg.receiver;
        resend_Pkg.audioPort=0;

        client->sendMessage(resend_Pkg);
    });
}

void SpecificChatWindow::videoCallDialog()
{
    // 创建并显示视频聊天对话框
    videoDlg = new QDialog;
    videoDlg->show();

    //布局
    QVBoxLayout* vedioVlayout = new QVBoxLayout(videoDlg);

    QLabel* videoLabel=new QLabel;

    QHBoxLayout* hLayout = new QHBoxLayout;
    QPushButton* recordButton = new QPushButton("录音");
    QPushButton* acceptButton = new QPushButton("同意");
    QPushButton* closeButton = new QPushButton("拒绝");
    hLayout->addWidget(recordButton);
    hLayout->addWidget(acceptButton);
    hLayout->addWidget(closeButton);

    vedioVlayout->addWidget(videoLabel);
    vedioVlayout->addLayout(hLayout);
    videoDlg->setLayout(vedioVlayout);

    // 连接信号槽（跨线程安全）
    connect(videoReceiver, &VideoReceiver::frameReceived, this, [=](const QImage &image) {
        // 转换图像格式为 RGB32
        QImage rgbImage = image.convertToFormat(QImage::Format_RGB32);
        videoLabel->setPixmap(QPixmap::fromImage(rgbImage.scaled(640, 480)));
    }, Qt::QueuedConnection); // 确保在主线程更新 UI
}

void SpecificChatWindow::voiceCallDialog()
{
    // 创建并显示语音聊天对话框
    audioDlg = new QDialog;
    audioDlg->show();

    //布局
    QVBoxLayout* audio_Vlayout = new QVBoxLayout(this);

    //设置占位图片
    QLabel* audio_Label = new QLabel;
    const QString placeholderPath = ".png";
    if(QFile::exists(placeholderPath))
    {
        audio_Label->setPixmap(QPixmap(placeholderPath));
    }

    QHBoxLayout* audio_Hlayout = new QHBoxLayout;
    QPushButton* recordButton = new QPushButton(QIcon(),"录音");
    QPushButton* closeButton = new QPushButton(QIcon(),"关闭");
    audio_Hlayout->addWidget(recordButton);
    audio_Hlayout->addWidget(closeButton);

    audio_Vlayout->addWidget(audio_Label);
    audio_Vlayout->addLayout(audio_Hlayout);
    audioDlg->setLayout(audio_Vlayout);

    connect(closeButton,&QPushButton::clicked,audioDlg,&QDialog::rejected);
}



/*
void SpecificChatWindow::deleteMessage(const QString &id)
{
    if (!messages.contains(id))
    {
        return;
    }

    QTextDocument* doc=textBrowser->document();
    QTextBlock block=doc->begin();

    while(block.isValid())
    {
        MessageBlockData* data=static_cast<MessageBlockData*>(block.userData());
        if(data&&data->id==id)
        {
            QTextCursor cursor(block);

            //选中整块内容
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::EndOfBlock,QTextCursor::KeepAnchor);

            // 删除内容及块后的换行符
            cursor.removeSelectedText();
            cursor.deleteChar();

            messages.remove(id);
            break;
        }
        block=block.next();
    }
}
*/

