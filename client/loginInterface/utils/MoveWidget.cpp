#include "MoveWidget.h"
#include <QMouseEvent>
#include <QWidget>

MoveWidget::MoveWidget(QWidget *target, QObject *parent)
    : QObject(parent), target(target)
{
}

bool MoveWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == target)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            // 记录鼠标按下时的全局位置
            mPos = mouseEvent->pos();
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton)
            {
                // 当前窗口位置+偏移量
                target->move(target->pos() + mouseEvent->pos() - mPos);
            }
        }
    }
    return QObject::eventFilter(obj, event);
}
