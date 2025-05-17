#include "LoginDialog.h"
#include <QRect>
#include <QPaintEvent>
#include <QGraphicsDropShadowEffect>
#include <QStaticText>
#include <QPixmap>
#include <QErrorMessage>
#include <QImageReader>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QFile>
#include <QTimer>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent, TcpClient *client, LocalDatabase *localBase)
    : QDialog(parent),
      client(client),
      localBase(localBase)
{
    connect(client, &TcpClient::messageReceived, this, &LoginDialog::handleServerResponse);
    initUi();
}

LoginDialog::~LoginDialog() {}

void LoginDialog::paintEvent(QPaintEvent *ev)
{
    // 设置渐变色
    QLinearGradient linearGrad(QPoint(0, 0), QPoint(370, height()));
    linearGrad.setColorAt(0, QColor(0xa8edea));
    linearGrad.setColorAt(1, QColor(0xfed6e3));

    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    painter.setBrush(Qt::white);
    painter.drawRect(rightRect);

    // 设置和弦
    painter.setBrush(linearGrad);
    painter.drawRect(leftRect);
    painter.drawChord(midRect, -90 * 16, 180 * 16);

    // 设置文本
    painter.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    painter.setFont(QFont("微软雅黑", 30));
    painter.drawStaticText(48, 60, QStaticText("Welcome"));
    painter.setFont(QFont("微软雅黑", 20));
    painter.drawStaticText(52, 120, QStaticText("QQ界面"));
    // 设置图片
    painter.drawPixmap(QRect(60, 200, 200, 200), QPixmap(":/resource/image/beauty.jpg"));
    painter.drawPixmap(QRect(390, 70, 300, 100), QPixmap(":/resource/image/title1.png"));
}

void LoginDialog::initUi()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize(740, 470);

    // 定义三个矩形模块
    leftRect = QRect(QPoint(5, 5), QPoint(190, height() - 10));
    midRect = QRect(QPoint(5, 5), QPoint(370, height() - 10));
    rightRect = QRect(QPoint(5, 60), QPoint(width() - 10, height() - 70));

    // 设置阴影
    setShadow();

    // close
    closeBtn = new QPushButton(this);
    closeBtn->setObjectName("closeBtn");
    // closeBtn->setIcon(QIcon(":/resource/image/close.png"));
    closeBtn->move(width() - 46, 70);
    // closeBtn->setFixedSize(32,32);
    // 取消按钮焦点设置
    closeBtn->setFocusPolicy(Qt::NoFocus);

    sLayWidget = new QStackedWidget(this);
    // 设置登录界面
    QWidget *createLoginPage = logPage();
    QWidget *createRegPage = regPage();
    sLayWidget->addWidget(createLoginPage);
    sLayWidget->addWidget(createRegPage);
    sLayWidget->setCurrentIndex(0);
    sLayWidget->move(430, 145);

    // 安装事件过滤器实现窗口移动
    this->installEventFilter(new MoveWidget(this));

    // 设置样式
    QFile file(":/resource/qss/StyleSheet.css");
    if (file.open(QIODevice::ReadOnly))
    {
        setStyleSheet(file.readAll());
    }

    // 设置提示词
    tipLab = new QLabel(this);
    tipLab->setGeometry(434, 350, 200, 30);
    tipLab->setAlignment(Qt::AlignCenter);
    // tipLab->setText("12,232232324张家界");
    tipLab->setStyleSheet("color:red");
    // 提示词3秒后自动消失
    timer = new QTimer(this);
    timer->callOnTimeout([this]()
                         {
        tipLab->clear();
        timer->stop(); });

    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
}

void LoginDialog::setShadow()
{
    auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setColor(Qt::gray);
    shadowEffect->setOffset(0);
    shadowEffect->setBlurRadius(10);
    this->setGraphicsEffect(shadowEffect);
}

QWidget *LoginDialog::logPage()
{
    QWidget *w = new QWidget;
    QHBoxLayout *hLayout = new QHBoxLayout;
    QVBoxLayout *vLayout = new QVBoxLayout;

    // 初始化控件
    lineAccount = new QLineEdit;
    linePwd = new QLineEdit;
    remPwd = new QCheckBox("记住密码");
    toReg = new QPushButton("没有账号? 去注册");
    logging = new QPushButton("登录");

    toReg->setObjectName("toReg");
    logging->setObjectName("logging");

    lineAccount->setPlaceholderText("用户名");
    linePwd->setPlaceholderText("密码");
    linePwd->setEchoMode(QLineEdit::Password);
    // 焦点设置
    toReg->setFocusPolicy(Qt::NoFocus);
    logging->setFocusPolicy(Qt::StrongFocus);

    // 添加控件到水平布局
    hLayout->addWidget(remPwd);
    hLayout->addWidget(toReg);
    // 添加控件到垂直布局
    vLayout->addWidget(lineAccount);
    vLayout->addWidget(linePwd);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(logging);

    // 布局调整
    vLayout->setSpacing(10);

    w->setLayout(vLayout);

    connect(toReg, &QPushButton::clicked, [this]()
            { sLayWidget->setCurrentIndex(1); });
    connect(logging, &QPushButton::clicked, this, &LoginDialog::onLogging);

    // 设置配置文件,将账号密码保存到本地(记住密码复选框)
    QSettings settings(configPath(), QSettings::Format::IniFormat);
    // 从配置文件中获取账号密码
    remPwd->setChecked(settings.value("remPwd", false).toBool());
    lineAccount->setText(settings.value("username", "").toString());
    if (remPwd->isChecked())
    {
        linePwd->setText(settings.value("password", "").toString());
    }

    return w;
}

QWidget *LoginDialog::regPage()
{
    QWidget *w = new QWidget;
    auto hLayout = new QHBoxLayout;
    auto vLayout = new QVBoxLayout;

    // 初始化控件
    m_LineAccount = new QLineEdit;
    m_LinePwd = new QLineEdit;
    m_ReEnterPwd = new QLineEdit;
    m_Reg = new QPushButton("注册");
    m_ToLog = new QPushButton("去登录");

    m_Reg->setObjectName("m_Reg");
    m_ToLog->setObjectName("m_ToLog");

    m_LineAccount->setPlaceholderText("用户名");
    m_LinePwd->setPlaceholderText("密码");
    m_ReEnterPwd->setPlaceholderText("确认密码");
    m_LinePwd->setEchoMode(QLineEdit::Password);
    m_ReEnterPwd->setEchoMode(QLineEdit::Password);

    // 添加控件到水平布局
    hLayout->addWidget(m_Reg);
    hLayout->addWidget(m_ToLog);

    hLayout->setSpacing(0);

    // 添加控件到垂直布局
    vLayout->addWidget(m_LineAccount);
    vLayout->addWidget(m_LinePwd);
    vLayout->addWidget(m_ReEnterPwd);
    vLayout->addLayout(hLayout);

    // 布局调整
    vLayout->setSpacing(10);

    w->setLayout(vLayout);

    connect(m_ToLog, &QPushButton::clicked, [this]()
            { sLayWidget->setCurrentIndex(0); });
    connect(m_Reg, &QPushButton::clicked, this, &LoginDialog::onRegister);

    return w;
}

void LoginDialog::setTipMsg(const QString &msg, int sec)
{
    tipLab->setText(msg);
    if (timer->isActive())
    {
        timer->stop();
    }
    timer->start(sec);
}

QString LoginDialog::configPath()
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QDir dir(path);
    return dir.path() + "/" + "hello.ini";
}

void LoginDialog::handleServerResponse(const Packege &resend_Pkg)
{
    switch (currentOperation)
    {
    case Operation::LOGIN:
        // 处理登录响应
        handleLoginResponse(resend_Pkg);
        break;
    case Operation::REGISTER:
        // 处理注册响应
        handRegisterResponse(resend_Pkg);
        break;
    default:
        qWarning() << "未定义操作类型";
        break;
    }
    currentOperation = Operation::NONE;
}

void LoginDialog::handleLoginResponse(const Packege &resend_Pkg)
{
    if (resend_Pkg.messageInfo.textOnly.trimmed() == "success")
    {
        auto username = lineAccount->text();
        auto password = linePwd->text();
        qApp->setProperty("username", username);
        qApp->setProperty("password", password);
        // 记住密码
        QSettings settings(configPath(), QSettings::Format::IniFormat);
        if (remPwd->isChecked())
        {
            settings.setValue("remPwd", remPwd->isChecked());
            settings.setValue("username", username);
            settings.setValue("password", password);
        }
        accept();
    }
    else
    {
        setTipMsg("登录失败");
    }
}

void LoginDialog::handRegisterResponse(const Packege &resend_Pkg)
{
    if (resend_Pkg.messageInfo.textOnly.trimmed() == "success")
    {
        setTipMsg("注册成功!");
        localBase->userProfile_Table_Create_UserInfo(resend_Pkg.user_Info);
    }
    else if (resend_Pkg.messageInfo.textOnly.trimmed() == "exits")
    {
        setTipMsg("用户已经存在!");
    }
    else
    {
        setTipMsg("注册失败!");
    }
}

void LoginDialog::onLogging()
{
    // 检查合法性
    auto username = lineAccount->text();
    auto password = linePwd->text();
    if (username.isEmpty() || password.isEmpty())
    {
        setTipMsg("账号或密码为空");
        return;
    }
    if (username.size() < 3 || password.size() < 3)
    {
        setTipMsg("账号或密码长度不得低于3位");
        return;
    }
    if (username.size() > 15 || password.size() > 15)
    {
        setTipMsg("账号或密码长度不得超过15位");
        return;
    }

    /*服务器认证登录*/

    // 标记当前是登录操作
    currentOperation = Operation::LOGIN;
    Packege send_pkg;
    // 打包数据包
    {
        send_pkg.sender = username;
        send_pkg.sender_Passwd = password;
        send_pkg.type = LOGIN;
    }

    if (client->sendMessage(send_pkg))
    {
        setTipMsg("正在登录...");
    }
    else
    {
        setTipMsg("发送请求失败");
    }
}

void LoginDialog::onRegister()
{
    // 检查合法性
    auto username = m_LineAccount->text();
    auto password = m_LinePwd->text();
    auto rePassword = m_ReEnterPwd->text();

    if (username.isEmpty() || password.isEmpty())
    {
        setTipMsg("账号或密码为空");
        return;
    }
    if (username.size() < 3 || password.size() < 3 || rePassword.size() < 3)
    {
        setTipMsg("账号或密码长度不得低于3位");
        return;
    }
    if (username.size() > 15 || password.size() > 15 || rePassword.size() > 15)
    {
        setTipMsg("账号或密码长度不得超过15位");
        return;
    }
    if (password != rePassword)
    {
        setTipMsg("密码不一致！");
        return;
    }

    /*服务器认证注册*/

    // 标记当前是注册操作
    currentOperation = Operation::REGISTER;
    /* QString message=username+" "+password+"1";*/
    Packege send_pkg;
    // 打包数据包
    {
        send_pkg.sender = username;
        send_pkg.sender_Passwd = password;
        send_pkg.type = REGISTER;
    }

    if (client->sendMessage(send_pkg))
    {
        setTipMsg("正在注册...");
    }
    else
    {
        setTipMsg("注册失败");
    }
}
