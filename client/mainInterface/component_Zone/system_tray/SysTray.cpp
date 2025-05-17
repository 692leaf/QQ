// SysTray.cpp
#include "systray.h"
#include <QApplication>

SysTray::SysTray(QObject *parent) : QObject(parent)
{
    m_trayIcon = new QSystemTrayIcon(this);
    createActions();
    createMenu();

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &SysTray::onTrayActivated);
}

SysTray::~SysTray()
{
    delete m_trayMenu;
}

void SysTray::show()
{
    m_trayIcon->show();
}

void SysTray::hide()
{
    m_trayIcon->hide();
}

void SysTray::setIcon(const QIcon &icon)
{
    m_trayIcon->setIcon(icon);
}

void SysTray::setToolTip(const QString &tip)
{
    m_trayIcon->setToolTip(tip);
}

void SysTray::createActions()
{
    m_restoreAction = new QAction(tr("&Restore"), this);
    connect(m_restoreAction, &QAction::triggered,
            this, &SysTray::onRestoreAction);

    m_quitAction = new QAction(tr("&Exit"), this);
    connect(m_quitAction, &QAction::triggered,
            this, &SysTray::onQuitAction);
}

void SysTray::createMenu()
{
    m_trayMenu = new QMenu();
    m_trayMenu->addAction(m_restoreAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_quitAction);
    m_trayIcon->setContextMenu(m_trayMenu);
}

void SysTray::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick)
    {
        emit restoreRequested();
    }
}

void SysTray::onRestoreAction()
{
    emit restoreRequested();
}

void SysTray::onQuitAction()
{
    emit quitRequested();
}
