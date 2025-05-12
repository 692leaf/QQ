#ifndef MOVEWIDGET_H
#define MOVEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QMouseEvent>

class MoveWidget : public QObject
{
    Q_OBJECT
public:
    explicit MoveWidget(QWidget *target,QObject *parent = nullptr);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;//事件过滤器可以拦截多个对象的事件，所以 obj 也可能指向其他对象
private:
    QWidget *target;
    QPoint mPos;
signals:
};

#endif // MOVEWIDGET_H
