#include "logininfor.h"
#include "ui_logininfor.h"
#include <QPalette>
#include <QPixmap>

LoginInfor::LoginInfor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginInfor)
{
    ui->setupUi(this);

    //设置窗口标题
    this->setWindowTitle("login failed");

    //设置窗口图标
    QIcon icon(":/pic/qiu.jpg");
    this->setWindowIcon(icon);
    this->show();
}

LoginInfor::~LoginInfor()
{
    delete ui;
}

void LoginInfor::resizeEvent(QResizeEvent *event)
{
    //设置窗口背景图片
    QWidget::resizeEvent(event);
    QPalette pa;
    QPixmap pix(":/pic/gaoyuanyuan.jpg");
    //背景图片自适应窗口大小
    pa.setBrush(QPalette::Window, QBrush(pix.scaled(event->size(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
    setPalette(pa);
}
