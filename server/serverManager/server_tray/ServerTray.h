// ServerTray.h
#ifndef SERVERTRAY_H
#define SERVERTRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>

class ServerTray : public QObject
{
    Q_OBJECT
public:
    explicit ServerTray(QObject *parent = nullptr);
    ~ServerTray();

    // 公共接口
    void show();                         // 显示托盘图标
    void hide();                         // 隐藏托盘图标
    void setIcon(const QIcon &icon);     // 设置托盘图标
    void setToolTip(const QString &tip); // 设置提示信息

signals:
    void restoreRequested(); // 恢复主窗口请求
    void quitRequested();    // 退出程序请求

private slots:
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onRestoreAction();
    void onQuitAction();

private:
    void createActions();
    void createMenu();

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_restoreAction;
    QAction *m_quitAction;
};

#endif // SERVERTRAY_H
