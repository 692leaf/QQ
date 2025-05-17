#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QPoint>
#include <QPainter>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include "Tcpclient.h"
#include "LocalDatabase.h"
#include "MoveWidget.h"

class LoginDialog : public QDialog
{
    Q_OBJECT
    // logReg
public:
    LoginDialog(QWidget *parent = nullptr, TcpClient *client = nullptr, LocalDatabase *localBase = nullptr);
    ~LoginDialog();

public:
    enum class Operation
    {
        NONE,
        LOGIN,
        REGISTER
    };

protected:
    void paintEvent(QPaintEvent *ev) override;

public:
    void initUi();
    void setShadow();
    QWidget *logPage();
    QWidget *regPage();
    void setTipMsg(const QString &msg, int sec = 3000);
    QString configPath();

private:
    Operation currentOperation = Operation::NONE;
    // 客户端
    TcpClient *client;
    LocalDatabase *localBase;

    QRect leftRect;
    QRect midRect;
    QRect rightRect;

    // close
    QPushButton *closeBtn;

    QStackedWidget *sLayWidget;

    // logPage
    QLineEdit *lineAccount;
    QLineEdit *linePwd;
    QCheckBox *remPwd;
    QPushButton *toReg;
    QPushButton *logging;

    // regPage
    QLineEdit *m_LineAccount;
    QLineEdit *m_LinePwd;
    QLineEdit *m_ReEnterPwd;
    QPushButton *m_ToLog;
    QPushButton *m_Reg;

    QLabel *tipLab;
    QTimer *timer;

public slots:
    void handleServerResponse(const Packege &resend_Pkg);
    void handleLoginResponse(const Packege &resend_Pkg);
    void handRegisterResponse(const Packege &resend_Pkg);
    void onLogging();
    void onRegister();
};
#endif // LOGINDIALOG_H
