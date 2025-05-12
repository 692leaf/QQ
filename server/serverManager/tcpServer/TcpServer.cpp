#include "Tcpserver.h"
#include <QUuid>
#include <QFileInfo>
#include <QTextDocument>
#include <QTextBlock>

TcpServer::TcpServer(QObject *parent, DatabaseManager* dbManager)
    : QObject{parent},
    dbManager(dbManager)
{}


TcpServer::~TcpServer()
{
    server->close();
    QMap<QString, QTcpSocket*>::iterator it;
    for (it = account_Socket_Map.begin(); it != account_Socket_Map.end(); ) {
        QTcpSocket* socket = it.value();
        socket->close();
        delete socket;
        it = account_Socket_Map.erase(it); // 移除键值对，并更新迭代器
    }
    delete server;
}

bool TcpServer::init()
{
    //初始化
    server=new QTcpServer(this);
    if (!startListen())
    {
        // 检查监听是否成功
        delete server;
        server = nullptr;
        return false;
    }

    connect(server,&QTcpServer::newConnection,this,&TcpServer::clientConnect);
    return true;
}



bool TcpServer::startListen()
{
    int port=6000;
    if(!server->listen(QHostAddress::AnyIPv4,port))
    {
        qCritical() << "监听失败：" << server->errorString();
        return false;
    }
    return true;
}

void TcpServer::clientConnect()
{
    QTcpSocket* newSocket=server->nextPendingConnection();
    clientSockets.append(newSocket);
    connect(newSocket,&QTcpSocket::readyRead,this,&TcpServer::readMessage);
    connect(newSocket, &QTcpSocket::disconnected, [this, newSocket](){
        // 清理该socket关联的所有账户映射
        QMutableMapIterator<QString, QTcpSocket*> it(account_Socket_Map);
        while (it.hasNext())
        {
            it.next();
            if (it.value() == newSocket)
            {
                // 更新数据库中用户在线状态
                QString account = it.key();
                dbManager->userTable_Update_OnlineStatus(account, false);
                it.remove(); // 移除该socket关联的所有账户

                // 通知好友更改我的状态为在线
                sendOnlineStatusUpdateToFriends(account, 0);
            }
        }

        socketBuffers.remove(newSocket); // 清理缓冲区
        newSocket->deleteLater();
    });
}



void TcpServer::readMessage()
{
    QTcpSocket* sendSocket=qobject_cast<QTcpSocket*>(sender());
    if(!sendSocket) return;

    /*粘包处理*/
    // 维护每个socket的接收缓冲区
    QByteArray &buffer=socketBuffers[sendSocket];
    buffer+=sendSocket->readAll();

    //完整包的数据
    QByteArray pkgData;
    // 包处理循环
    while(true)
    {
        // 包头未接收完整
        if(buffer.size()<static_cast<int>(sizeof(quint32)))
            break;

        // 提取包体长度
        quint32 bodySize;
        QDataStream headStream(buffer);
        headStream>>bodySize;

        // 包体未接收完整
        if(buffer.size()<static_cast<int>(sizeof(quint32)+bodySize))
            break;

        // 提取完整数据包
        pkgData=buffer.mid(sizeof(quint32), bodySize);
        buffer.remove(0,sizeof(quint32)+bodySize);

        // 反序列化处理
        Packege send_Pkg;
        QDataStream in(&pkgData,QIODevice::ReadOnly);
        in.setVersion(QDataStream::Qt_6_5); // 必须与客户端一致
        // 用数据流的方式解析数据
        in >> send_Pkg;

        // 处理数据包
        processPackage(send_Pkg, sendSocket);
    }
}

bool TcpServer::sendMessage(const Packege& resend_Pkg)
{
    QTcpSocket* resendSocket=qobject_cast<QTcpSocket*>(sender());
    if (!resendSocket || resendSocket->state() != QTcpSocket::ConnectedState)
        return false;

    // 序列化数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5); // 必须与客户端一致

    // 先预留包头位置
    out << quint32(0);
    // 写入实际数据
    out << resend_Pkg;

    // 回到数据开头写入实际长度
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));

    return resendSocket->write(block)!=-1;
}

bool TcpServer::sendMessage(const Packege& resend_Pkg,QTcpSocket *resendSocket)
{
    if (!resendSocket || resendSocket->state() != QTcpSocket::ConnectedState)
        return false;

    // 序列化数据
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_5); // 必须与客户端一致

    // 先预留包头位置
    out << quint32(0);
    // 写入实际数据
    out<<resend_Pkg;

    // 回到数据开头写入实际长度
    out.device()->seek(0);
    out << quint32(block.size() - sizeof(quint32));

    return resendSocket->write(block)!=-1;
}

void TcpServer::processPackage(Packege& send_Pkg,QTcpSocket* sendSocket)
{
    Packege resend_Pkg;
    switch(send_Pkg.type)
    {
    case HEARTBEAT_MONITORING://心跳监测
    {
        resend_Pkg=send_Pkg;
        sendMessage(resend_Pkg);
    }
    break;
    case LOGIN://登录
    {
        QString condition = QString("account='%1' AND password='%2'").arg(send_Pkg.sender).arg(send_Pkg.sender_Passwd);
        if (dbManager->userTable_Read_UserLogin(condition))
        {
            resend_Pkg.messageInfo.textOnly = "success";
        }
        else
        {
            resend_Pkg.messageInfo.textOnly = "failed";
        }

        resend_Pkg.type=LOGIN;
        sendMessage(resend_Pkg);
    }
    break;
    case REGISTER://注册
    {
        if(dbManager->userTable_Read_AccountExists(send_Pkg.sender))
        {
            resend_Pkg.messageInfo.textOnly = "exits";
        }
        else
        {
            QString uuid = dbManager->userTable_Create_AccountRecord(send_Pkg);
            resend_Pkg.messageInfo.textOnly = (uuid!="") ? "success" : "failed";
            // 注册成功时，获取版本号，并下载默认头像
            resend_Pkg.user_Info.account = send_Pkg.sender;
            resend_Pkg.user_Info.version = uuid;
            QString imagePath = ":/resource/默认头像框.png";
            resend_Pkg.user_Info.avatarData = fetchImageData(imagePath);
        }
        resend_Pkg.type=REGISTER;
        sendMessage(resend_Pkg);
    }
    break;
    case GET_FRIENDBAR_DATA:// 获取好友申请栏数据
    {
        // 四种可能(pending_Verification,disapproved,approved,to_Agree)

        // 预先分配四个空的一维 vector，作为二维 vector 的父元素
        resend_Pkg.friendListBarItemTextStates.resize(4);

        dbManager->frdTable_ReadMyListRecord(resend_Pkg.friendListBarItemTextStates,send_Pkg.sender);
        dbManager->frdTable_ReadOtherListRecord(resend_Pkg.friendListBarItemTextStates,send_Pkg.sender);

        for(int i=0;i<4;i++)
        {
            for(auto& friendInfo : resend_Pkg.friendListBarItemTextStates[i])
            {
                friendInfo.avatarData = fetchImageData(friendInfo.avatar_Path);
            }
        }

        resend_Pkg.type=GET_FRIENDBAR_DATA;
        sendMessage(resend_Pkg);
    }
    break;
    case FRIEND_REQUEST_SENT:// 好友申请
    {
        // 保存数据到数据库中
        dbManager->frdTable_Create_FriendListRecord(send_Pkg);

        // 直接发送数据给对方
        QTcpSocket* targetSocket = findSocketByAccount(send_Pkg.receiver);
        if (targetSocket)
        {
            Account_Message user_Info = dbManager->userTable_Read_UserMessage(send_Pkg.sender);
            user_Info.avatarData = fetchImageData(user_Info.avatar_Path);
            resend_Pkg.user_Info = user_Info;
            resend_Pkg.type = FRIEND_REQUEST_SENT;
            sendMessage(resend_Pkg,targetSocket);
        }
    }
    break;
    case HANDLE_FRIEND_REQUEST:// 处理好友申请
    {
        dbManager->frdTable_Update_FriendListRecord(send_Pkg);


        // 1.同意后更新本地好友列表
        resend_Pkg.user_Info = dbManager->userTable_Read_UserMessage(send_Pkg.sender);
        resend_Pkg.user_Info.avatarData = fetchImageData(resend_Pkg.user_Info.avatar_Path);
        resend_Pkg.type=HANDLE_FRIEND_REQUEST;
        sendMessage(resend_Pkg);

        // 2.同意后更新对方好友列表
        QTcpSocket* targetSocket = findSocketByAccount(send_Pkg.sender); // 根据用户名找到对应的 QTcpSocket

        if (targetSocket)//对方更新好友列表
        {
            //同意后对方好友列表更新
            Packege resend_Pkg_To_Freind;
            resend_Pkg_To_Freind.user_Info = dbManager->userTable_Read_UserMessage(send_Pkg.receiver);
            resend_Pkg_To_Freind.user_Info.avatarData = fetchImageData(resend_Pkg_To_Freind.user_Info.avatar_Path);
            resend_Pkg_To_Freind.type=HANDLE_FRIEND_REQUEST;
            sendMessage(resend_Pkg_To_Freind, targetSocket);
        }
    }
    break;
    case SEARCH_PAGE_DATA:// 获取搜索页面的数据
    {
        resend_Pkg.type = SEARCH_PAGE_DATA;
        QVector<Account_Message> search_Page_Data =
            dbManager->userTable_Read_AccountMessage(send_Pkg.messageInfo.textOnly);
        for(auto& user_Info : search_Page_Data)
        {
            user_Info.avatarData = fetchImageData(user_Info.avatar_Path);
        }

        resend_Pkg.search_Page_Data = search_Page_Data;
        sendMessage(resend_Pkg);
    }
    break;
    case UPDATE_ACCOUNT_INFO:// 更新账户信息
    {
        // 保存数据到数据库中
        dbManager->userTable_Update_Account_Info(send_Pkg);

        // 向在线好友同步自己的最新信息
        resend_Pkg = send_Pkg;
        QVector<Account_Message> friend_List_Data = dbManager->frdTable_ReadFriendRecord(send_Pkg.user_Info.account);
        for(auto& friend_Data:friend_List_Data)
        {
            // 根据用户名找到对应的 QTcpSocket
            QTcpSocket* targetSocket = findSocketByAccount(friend_Data.account);

            if (targetSocket)
            {
                sendMessage(resend_Pkg,targetSocket);
            }
        }
    }
    break;
    case SEND_RICHTEXT_MESSAGE:// 发送富文本消息
    {
        //发送给对方的包（拷贝一份）
        resend_Pkg=send_Pkg;

        // 先断开旧连接，再绑定新连接
        disconnect(this, &TcpServer::imageUrlsGenerated, nullptr, nullptr); // 断开所有旧连接
        connect(this,&TcpServer::imageUrlsGenerated,[this,resend_Pkg](const Packege& send_Pkg){
            //将数据保存到数据库中
            int recordId = dbManager->chrTable_Create_ChatRecord(send_Pkg);

            //直接发送给对方
            QTcpSocket* targetSocket = findSocketByAccount(resend_Pkg.receiver);
            if (targetSocket && targetSocket->state() == QTcpSocket::ConnectedState)
            {
                sendMessage(resend_Pkg,targetSocket);
                // 标记已同步
                dbManager->markMessageSynced(recordId);
            }
        });

        generateLocalImageUrls(send_Pkg);
    }
    break;
    case SEND_FILE_MESSAGE:// 发送文件消息
    {
        //是否接收完整
        bool isTransferComplete=0;
        const QString fileID=send_Pkg.messageInfo.file.fileID;
        if(send_Pkg.messageInfo.file.currentBlock==0)
        {
            activePackege[fileID]=send_Pkg;
            activePackege[fileID].messageInfo.file.receivedBlocks=0;
        }
        else
        {
            const int currentBlock=send_Pkg.messageInfo.file.currentBlock;
            activePackege[fileID].messageInfo.file.blocks[currentBlock]=
                send_Pkg.messageInfo.file.fileContent;
            int receivedBlocks=++activePackege[fileID].messageInfo.file.receivedBlocks;
            int totalBlocks=activePackege[fileID].messageInfo.file.totalBlocks;

            // 检查是否接收完成
            if(receivedBlocks==totalBlocks)
            {
                // 按序号合并块
                QByteArray fullData;
                for(int blockIndex=1;blockIndex<=totalBlocks;blockIndex++)
                {
                    QByteArray currentData=activePackege[fileID].messageInfo.file.blocks[blockIndex];
                    fullData+=currentData;
                }

                int totalSize=activePackege[fileID].messageInfo.file.fileSize;

                if(fullData.size()==totalSize)
                {
                    resend_Pkg=activePackege[fileID];
                    resend_Pkg.messageInfo.file.fileContent=fullData;
                    activePackege[fileID].messageInfo.file.blocks.clear();
                    activePackege.remove(fileID);
                    //资源释放
                    isTransferComplete=1;
                }
                else
                {
                    // 请求完整重传
                    // requestFullResend(fileID);
                }
            }
        }

        //文本或文件完整后放行
        if(isTransferComplete)
        {
            //将数据保存到数据库中
            int recordId = dbManager->chrTable_Create_ChatRecord(resend_Pkg);

            //将消息对方显示到对方聊天窗口上
            QTcpSocket* targetSocket = findSocketByAccount(send_Pkg.receiver);
            if (targetSocket&&targetSocket->state() == QTcpSocket::ConnectedState)
            {
                sendFileInfo(resend_Pkg,targetSocket);
                // 标记已同步
                dbManager->markMessageSynced(recordId);
            }
        }
    }
    break;
    case VIDEO_SIGNALING_MESSAGE_SEND://信令,用于在视频通讯中建立连接
    {
        send_Pkg.ip=dbManager->userTable_Read_IPv4(send_Pkg.sender);
        sendMessage(send_Pkg,findSocketByAccount(send_Pkg.receiver));
    }
    break;
    case VIDEO_SIGNALING_MESSAGE_RECEIVER:
    {
        send_Pkg.ip=dbManager->userTable_Read_IPv4(send_Pkg.receiver);
        sendMessage(send_Pkg,findSocketByAccount(send_Pkg.sender));
    }
    break;
    case AUDIO_SIGNALING_MESSAGE_SEND://信令,用于在语音通话中建立连接
    {
        send_Pkg.ip=dbManager->userTable_Read_IPv4(send_Pkg.sender);
        sendMessage(send_Pkg,findSocketByAccount(send_Pkg.receiver));
    }
    break;
    case AUDIO_SIGNALING_MESSAGE_RECEIVER:
    {
        send_Pkg.ip=dbManager->userTable_Read_IPv4(send_Pkg.receiver);
        sendMessage(send_Pkg,findSocketByAccount(send_Pkg.sender));
    }
    break;
    }

    // 特定用户的sendSocket改变了,则更新映射,并更新表中ip
    if(!account_Socket_Map.contains(send_Pkg.sender)||account_Socket_Map[send_Pkg.sender]!=sendSocket)
    {
        account_Socket_Map[send_Pkg.sender]=sendSocket;
        // 获取客户端的IP地址
        QHostAddress clientAddress=sendSocket->peerAddress();
        QString ipAddress=clientAddress.toString();
        dbManager->userTable_Update_IPv4(send_Pkg.sender,ipAddress);

        // 同步用户本地消息
        unreadNotificationAsync(send_Pkg.sender);

        // 通知好友更改我的状态为在线
        sendOnlineStatusUpdateToFriends(send_Pkg.sender, 1);
    }
}

void TcpServer::generateLocalImageUrls(Packege &send_Pkg)
{
    // 将图片保存到数据库,返回url
    send_Pkg.messageInfo.images=dbManager->mediaTable_Create_MediaRecord(send_Pkg);
    // 生成富文本
    send_Pkg.messageInfo.richText=generateRichTextInfo(send_Pkg);
    emit imageUrlsGenerated(send_Pkg);
}

QString TcpServer::generateRichTextInfo(const Packege &send_Pkg)
{
    // 获取当前富文本（带占位符）
    QString html = send_Pkg.messageInfo.richText;
    QVector<Image> images=send_Pkg.messageInfo.images;
    for (int i = 0; i < images.size(); ++i) {
        QString placeholder = QString("IMG_PLACEHOLDER_%1")
        .arg(images[i].uniqueId);

        html.replace(placeholder, images[i].url);
    }

    return html;
}

void TcpServer::sendImageInfo(Packege &resend_Pkg, QTcpSocket *targetSocket)
{
    // 1. 提取富文本中的图片资源和位置
    QString html = resend_Pkg.messageInfo.richText;
    QVector<Image> imageInfo = extractImagesFromHtml(html); // 返回<唯一标识符, 图片本地路径>

    // 2. 构建图片同步包
    resend_Pkg.messageInfo.images = imageInfo; // 直接使用已解析的图片信息
    resend_Pkg.messageInfo.richText = html;    // 存储处理后的HTML（带占位符）

    // 3. 填充图片二进制数据
    for(int i=0;i<imageInfo.size();i++)
    {
        QFileInfo fileInfo(imageInfo[i].localPath);

        resend_Pkg.messageInfo.images[i].imageName = dbManager->mediaTable_Select_Media_Name(imageInfo[i].localPath); // 文件名（含扩展名）
        resend_Pkg.messageInfo.images[i].imageType = fileInfo.suffix().toLower();                                     // 文件类型（png等）

        resend_Pkg.messageInfo.images[i].imageData = fetchImageData(imageInfo[i].localPath);
    }

    // 4. 发送同步包
    sendMessage(resend_Pkg,targetSocket);
}

QByteArray TcpServer::fetchImageData(const QString& imagePath)
{
    QByteArray imageData;
    // 安全读取图片数据
    QFile image(imagePath);
    if (image.open(QIODevice::ReadOnly))
    {
        imageData = image.readAll();
        image.close();
    }
    else
    {
        qWarning() << "无法打开图片:" << imagePath;
        imageData.clear(); // 清空无效数据
    }

    return imageData;
}

QVector<Image> TcpServer::extractImagesFromHtml(QString &html)
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

void TcpServer::sendFileInfo(Packege& resend_Pkg,QTcpSocket* targetSocket)
{
    const int blockSize=1024*1024; //每次读取1MB

    QByteArray fileData=resend_Pkg.messageInfo.file.fileContent;
    const int totalSize=resend_Pkg.messageInfo.file.fileSize;
    const int totalBlocks=qCeil(totalSize / (double)blockSize);
    const QString fileID = QUuid::createUuid().toString(); // 新生成传输ID

    // 发送元数据包
    Packege metaPkg;
    metaPkg = resend_Pkg; // 继承原有信息
    metaPkg.messageInfo.file.currentBlock=0; // 元数据标识
    metaPkg.messageInfo.file.totalBlocks=totalBlocks;
    metaPkg.messageInfo.file.fileID=fileID;
    metaPkg.messageInfo.file.fileSize=totalSize;
    metaPkg.messageInfo.file.blocks.clear();
    sendMessage(metaPkg, targetSocket);


    // 分块发送数据
    for(int blockIndex=1;blockIndex<=totalBlocks;blockIndex++)
    {
        //计算块范围
        const int startPos=(blockIndex-1)*blockSize;
        const int endPos=qMin(startPos+blockSize,totalSize);
        const int chunkSize=endPos-startPos;

        //准备数据包
        Packege dataPkg;
        dataPkg=resend_Pkg; //继承基础信息
        dataPkg.messageInfo.file.currentBlock=blockIndex;
        dataPkg.messageInfo.file.totalBlocks=totalBlocks;
        dataPkg.messageInfo.file.fileID=fileID;
        dataPkg.messageInfo.file.fileContent=fileData.mid(startPos,chunkSize);

        // 添加CRC校验（使用qChecksum）
        /* const quint32 crc = qChecksum(dataPkg.messageInfo.file.fileContent.constData
                                        dataPkg.messageInfo.file.fileContent.size());
                                        dataPkg.messageInfo.file.blockCRCs[blockIndex] = crc;*/

        // 发送数据块
        sendMessage(dataPkg, targetSocket);

        // 流量控制：可根据网络状况添加延时
        // QThread::usleep(100);
    }
}

void TcpServer::unreadNotificationAsync(const QString& account)
{
    unreadFriendListInfosAsync(account);
    unreadTextFileNotificationsAsync(account);
}

void TcpServer::unreadFriendListInfosAsync(const QString& account)
{
    Packege friendInfoSyncPackage;
    friendInfoSyncPackage.type = ASYNC_FETCH_FRIEND_LIST_INFOS;
    friendInfoSyncPackage.sender = account;
    friendInfoSyncPackage.friend_List_Data = dbManager->frdTable_ReadFriendRecord(account);

    // 根据用户名找到对应的 QTcpSocket
    QTcpSocket* targetSocket = findSocketByAccount(account);
    if(!targetSocket) return;

    for(auto& friend_Data:friendInfoSyncPackage.friend_List_Data)
    {
        // 获取图片数据
        friend_Data.avatarData = fetchImageData(friend_Data.avatar_Path);

        // 同步自己的最新信息
        sendMessage(friendInfoSyncPackage,targetSocket);
    }
}

void TcpServer::unreadTextFileNotificationsAsync(const QString& account)
{
    QVector<Packege> chatHistory_Pkgs=dbManager->chrTable_Read_ChatHistory(account);

    QTcpSocket* targetSocket = findSocketByAccount(account);
    for(int i=0;i<chatHistory_Pkgs.size();i++)
    {
        if(chatHistory_Pkgs[i].messageInfo.message_type==RICHTEXTCONTENT_TRANSFERS)
        {
            sendImageInfo(chatHistory_Pkgs[i],targetSocket);
        }
        else if(chatHistory_Pkgs[i].messageInfo.message_type==BLOCK_FILE_TRANSFERS)// 文件分块发送
        {
            sendFileInfo(chatHistory_Pkgs[i],targetSocket);
        }
    }
}

void TcpServer::sendOnlineStatusUpdateToFriends(const QString &account, bool isOnline)
{
    // 通知好友更改我的状态为在线
    Packege onlineStatusPackage;
    onlineStatusPackage.type = UPDATE_ONLINE_STATUS;
    onlineStatusPackage.user_Info.account = account;
    onlineStatusPackage.user_Info.status = isOnline;
    QVector<Account_Message> friend_List_Data = dbManager->frdTable_ReadFriendRecord(account);

    for(auto& friend_Data:friend_List_Data)
    {
        // 根据用户名找到对应的 QTcpSocket
        QTcpSocket* targetSocket = findSocketByAccount(friend_Data.account);

        if (targetSocket)
        {
            sendMessage(onlineStatusPackage,targetSocket);
        }
    }
}

QTcpSocket *TcpServer::findSocketByAccount(const QString &account)
{
    if(account_Socket_Map.contains(account))
    {
        return account_Socket_Map[account];
    }
    return nullptr;
}





