#include "DefaultWindow.h"
#include <QPixmap>
#include <QBoxLayout>

DefaultWindow::DefaultWindow(QWidget *parent)
    : QWidget{parent}
{
    imageLabel=new QLabel(this);
    //创建一个 QLabel 用于显示图片
    QPixmap pixmap(":/resource/image/dftImage.png");
    if(!pixmap.isNull())
    {
        // 设置图片适应标签大小
        pixmap = pixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding);
        imageLabel->setPixmap(pixmap);
        imageLabel->setAlignment(Qt::AlignCenter);
        // 设置标签覆盖整个窗口
        imageLabel->setGeometry(0, 0, this->width(), this->height());
    }
    QVBoxLayout* vLayout=new QVBoxLayout(this);
    vLayout->addWidget(imageLabel);
    this->setLayout(vLayout);
}
