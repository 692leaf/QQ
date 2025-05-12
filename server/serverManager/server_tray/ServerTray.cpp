// ServerTray.cpp
#include "ServerTray.h"
#include <QApplication>

ServerTray::ServerTray(QObject *parent) : QObject(parent)
{
    m_trayIcon = new QSystemTrayIcon(this);
    createActions();
    createMenu();

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &ServerTray::onTrayActivated);
}

ServerTray::~ServerTray()
{}

void ServerTray::show()
{
    m_trayIcon->show();
}

void ServerTray::hide()
{
    m_trayIcon->hide();
}

void ServerTray::setIcon(const QIcon &icon)
{
    m_trayIcon->setIcon(icon);
}

void ServerTray::setToolTip(const QString &tip)
{
    m_trayIcon->setToolTip(tip);
}

void ServerTray::createActions()
{
    m_restoreAction = new QAction(tr("&Restore"), this);
    connect(m_restoreAction, &QAction::triggered,
            this, &ServerTray::onRestoreAction);

    m_quitAction = new QAction(tr("&Exit"), this);
    connect(m_quitAction, &QAction::triggered,
            this, &ServerTray::onQuitAction);
}

void ServerTray::createMenu()
{
    m_trayMenu = new QMenu();
    m_trayMenu->addAction(m_restoreAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_quitAction);
    m_trayIcon->setContextMenu(m_trayMenu);
}

void ServerTray::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick) {
        emit restoreRequested();
    }
}

void ServerTray::onRestoreAction()
{
    emit restoreRequested();
}

void ServerTray::onQuitAction()
{
    emit quitRequested();
}
