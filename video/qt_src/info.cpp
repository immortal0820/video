#include "info.h"
#include "ui_info.h"
#include <QPixmap>
#include <QPalette>

Info::Info(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Info)
{
    ui->setupUi(this);

    //设置窗口标题
    this->setWindowTitle("register success");

    //设置窗口图标
    QIcon icon(":/pic/qiu.jpg");
    this->setWindowIcon(icon);
    this->show();
}

Info::~Info()
{
    delete ui;
}

void Info::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    QPalette pa;
    QPixmap pix(":/pic/taile.jpg");
    pa.setBrush(QPalette::Window, QBrush(pix.scaled(event->size(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
    setPalette(pa);
}
