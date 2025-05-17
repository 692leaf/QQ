#include "AddFriendOrGroupDialog.h"
#include <QApplication>
#include <QStandardItem>

AddFriendOrGroupDialog::AddFriendOrGroupDialog(QWidget *parent, TcpClient *client)
    : QDialog{parent},
      client(client)
{
    initUi();
}

AddFriendOrGroupDialog::~AddFriendOrGroupDialog()
{
}

void AddFriendOrGroupDialog::initUi()
{
    setWindowTitle("全网搜索");

    // 创建布局
    QVBoxLayout *vLayout = new QVBoxLayout(this);

    QLineEdit *searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("请输入搜索关键词");

    // 创建QListView实例
    listView = new QListView;
    // 创建数据模型
    model = new QStandardItemModel;
    // 将模型设置给listView
    listView->setModel(model);

    // 设置自定义委托
    delegate = new ButtonDelegate(this);
    listView->setItemDelegate(delegate);

    // 连接text信号到槽函数
    connect(searchLineEdit, &QLineEdit::textChanged, this, &AddFriendOrGroupDialog::onSearchTextChanged);
    // 连接 client 的 messageReceived 信号到槽函数
    connect(client, &TcpClient::messageReceived, this, &AddFriendOrGroupDialog::handleSearchPageDataResponse);
    // 连接按钮(“加入”)信号到槽函数
    connect(delegate, &ButtonDelegate::buttonClicked, this, &AddFriendOrGroupDialog::onAddButtonClicked);

    Packege send_Pkg;
    send_Pkg.sender = qApp->property("username").toString();
    send_Pkg.messageInfo.textOnly = "";
    send_Pkg.type = SEARCH_PAGE_DATA;

    // 默认显示前六个用户
    client->sendMessage(send_Pkg);

    vLayout->addWidget(searchLineEdit);
    vLayout->addWidget(listView);

    this->setLayout(vLayout);
}

void AddFriendOrGroupDialog::onSearchTextChanged(const QString &text)
{
    Packege send_Pkg;
    send_Pkg.messageInfo.textOnly = text;
    send_Pkg.type = SEARCH_PAGE_DATA;

    client->sendMessage(send_Pkg);
}

/*搜索栏页面数据获取*/
void AddFriendOrGroupDialog::handleSearchPageDataResponse(const Packege &resend_Pkg)
{
    if (resend_Pkg.type != SEARCH_PAGE_DATA)
        return;

    model->clear(); // 清空数据模型

    for (auto user_Info : resend_Pkg.search_Page_Data)
    {
        // 从databasemanager数据库中获取用户信息
        // 增加项目栏
        QStandardItem *item = new QStandardItem;

        // 头像加载
        QPixmap avatarPixmap;
        if (!avatarPixmap.loadFromData(user_Info.avatarData))
        {
            // 直接从字节数组加载图像
            qWarning() << "Failed to load pixmap from byte array";
            // 加载失败时设置默认头像
            avatarPixmap = QPixmap(":/resource/image/avatar1.jpg");
        }

        item->setData(QVariant::fromValue(avatarPixmap), Qt::DecorationRole); // 头像
        item->setData(user_Info.account, Qt::UserRole);                       // QQ号
        item->setData(user_Info.nickname, Qt::DisplayRole);                   // 昵称

        // 清除可编辑标志,保留其他默认标志
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);

        QModelIndex index = model->indexFromItem(item);
        delegate->setButtonEnabled(index, true);

        model->appendRow(item);
    }
}

void AddFriendOrGroupDialog::onAddButtonClicked(const QModelIndex &index)
{
    Packege send_Pkg;
    send_Pkg.sender = qApp->property("username").toString();
    send_Pkg.receiver = model->itemFromIndex(index)->data(Qt::UserRole).toString(); // QQ号
    send_Pkg.type = FRIEND_REQUEST_SENT;

    client->sendMessage(send_Pkg);
    delegate->setButtonText(index, "已申请");

    // 发送信号
    Account_Message user_Info;
    user_Info.account = model->itemFromIndex(index)->data(Qt::UserRole).toString();     // QQ号
    user_Info.nickname = model->itemFromIndex(index)->data(Qt::DisplayRole).toString(); // 昵称

    // ========================================== 图片 ==========================================
    // 从QStandardItem获取头像数据
    QVariant decorationData = model->itemFromIndex(index)->data(Qt::DecorationRole);

    // 将QVariant转换为QPixmap
    QPixmap avatarPixmap;
    if (decorationData.isValid() && decorationData.canConvert<QPixmap>())
    {
        avatarPixmap = decorationData.value<QPixmap>();
    }
    else
    {
        // 处理转换失败的情况
        qWarning() << "Failed to get avatar pixmap from item data";
        avatarPixmap = QPixmap(":/resource/image/avatar1.jpg");
    }

    // 将QPixmap转换为QByteArray
    QByteArray &avatarData = user_Info.avatarData;
    if (!avatarPixmap.isNull())
    {
        // 将QPixmap转为QImage
        QImage avatarImage = avatarPixmap.toImage();

        // 使用QBuffer将图像数据写入QByteArray
        QBuffer buffer(&avatarData);
        buffer.open(QIODevice::WriteOnly);

        // 保存为PNG格式（可根据需要改为JPEG等）
        if (avatarImage.save(&buffer, "PNG"))
        {
            qDebug() << "Avatar data saved successfully, size:" << avatarData.size();
        }
        else
        {
            qWarning() << "Failed to save avatar image data";
            avatarData.clear();
        }
    }
    else
    {
        qWarning() << "Avatar pixmap is null";
    }

    emit onAddNewFriendButtonClickded(user_Info);
}
