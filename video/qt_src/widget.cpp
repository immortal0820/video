#include "widget.h"
#include "ui_widget.h"
#include "logininfor.h"
#include "videoplay.h"
#include <QPalette>
#include <QPixmap>
#include <QHostAddress>
#include <QDebug>
#include <header.h>
#include <register.h>
#include <strings.h>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //设置窗口标题
    this->setWindowTitle("视频监控系统");


    //设置窗口图标
    QIcon icon(":/pic/bird.jpg");
    this->setWindowIcon(icon);
    this->show();

    //设置按钮的大小
    ui->login->setFixedSize(QSize(80, 20));
    ui->register_2->setFixedSize(QSize(80, 20));

    //设置按钮状态
    ui->login->setDisabled(true);
    ui->register_2->setDisabled(true);

    //设置用户密码为密码形式
    ui->log_password->setEchoMode(QLineEdit::Password);

    connect(ui->register_2, SIGNAL(clicked(bool)),
            this, SLOT(slotRegister()));
    connect(ui->accept, SIGNAL(clicked(bool)),
            this, SLOT(slotAccept()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::resizeEvent(QResizeEvent *event)
{
    //添加背景图片
    QWidget::resizeEvent(event);
    QPalette pa;
    QPixmap pix(":/pic/liushishi.jpg");

    //图片自适应放大和缩小
    pa.setBrush(QPalette::Window, QBrush(pix.scaled(event->size(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
    setPalette(pa);

}

void Widget::slotConned()
{
    ui->login->setEnabled(true);
    ui->register_2->setEnabled(true);

    connect(ui->login, SIGNAL(clicked(bool)),
            this, SLOT(slotLogin()));
}

void Widget::slotAccept()
{
    //网络通信
    QString strip = ui->serip->text();
    s = new QTcpSocket(this);
    s->connectToHost(QHostAddress(strip), 6666);

    connect(s, SIGNAL(connected()),
            this, SLOT(slotConned()));

}

void Widget::slotRecv()
{
    USE user;

    memset(&user, 0, sizeof(USE));

    s->read((char *)&user, sizeof(USE));

    qDebug() << user.username << endl ;

    if(0 == strncmp(user.username, "L", 1)) {
        VideoPlay *viplay = new VideoPlay();
        viplay->show();
    } else if(0 == strncmp(user.username, "N", 1)){
        LoginInfor *log = new LoginInfor();
        log->show();
    }
}

void Widget::slotLogin()
{
    USE user;

    memset(&user, 0, sizeof(USE));
    //把LineEdit中的内容另存到数组中
    QString str1 = ui->log_username->text();
    QString str2 = ui->log_password->text();

    QByteArray arr1 = str1.toLatin1();
    strcpy(user.username, arr1.data());
    QByteArray arr2 = str2.toLatin1();
    strcpy(user.password, arr2.data());
    user.flag = 1;

    //发送数据
    s->write((char *)&user, sizeof(USE));

    connect(s, SIGNAL(readyRead()),
            this, SLOT(slotRecv()));
}

void Widget::slotRegister()
{
    Register *reg = new Register();
    reg->show();
}
