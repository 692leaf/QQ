#ifndef DEFAULTWINDOW_H
#define DEFAULTWINDOW_H

#include <QWidget>
#include <QLabel>

class DefaultWindow : public QWidget
{
    Q_OBJECT
public:
    explicit DefaultWindow(QWidget *parent = nullptr);

private:
    QLabel *imageLabel;
};

#endif // DEFAULTWINDOW_H
