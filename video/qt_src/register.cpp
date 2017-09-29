#include "register.h"
#include "ui_register.h"
#include <header.h>
#include <info.h>
#include <QHostAddress>
#include <strings.h>

Register::Register(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Register)
{
    ui->setupUi(this);

    //设置窗口标题
    this->setWindowTitle("注册账户");

    //设置窗口图标
    QIcon icon(":/pic/clock.jpg");
    this->setWindowIcon(icon);
    this->show();

    //设置按钮状态
    ui->reg->setDisabled(true);

    //设置用户密码为密码形式
    ui->reg_password->setEchoMode(QLineEdit::Password);

    //网络通信
    s = new QTcpSocket(this);
    s->connectToHost(QHostAddress("192.168.3.10"),6666);

    connect(s, SIGNAL(connected()),
            this, SLOT(slotConned()));
}

Register::~Register()
{
    delete ui;
}

void Register::resizeEvent(QResizeEvent *event)
{
    //添加背景图片
    QWidget::resizeEvent(event);
    QPalette pa;
    QPixmap pix(":/pic/yang.jpg");

    //图片自适应放大和缩小
    pa.setBrush(QPalette::Window, QBrush(pix.scaled(event->size(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
    setPalette(pa);

}

void Register::slotConned()
{
    ui->reg->setEnabled(true);

    connect(ui->reg, SIGNAL(clicked(bool)),
            this, SLOT(slotRegister()));
}

void Register::slotRecv()
{
    USE user;

    memset(&user, 0, sizeof(USE));
    s->read((char *)&user, sizeof(USE));
    qDebug() << user.username<< endl ;

    if(0 == strncmp(user.username, "R", 1)) {
        Info *info = new Info();
        info->show();
    }
}

void Register::slotRegister()
{
    USE user;

    memset(&user, 0, sizeof(USE));
    //把LineEdit中的内容另存到数组中
    QString str1 = ui->reg_username->text();
    QString str2 = ui->reg_password->text();

    QByteArray arr1 = str1.toLatin1();
    strcpy(user.username, arr1.data());
    QByteArray arr2 = str2.toLatin1();
    strcpy(user.password, arr2.data());
    user.flag = 2;

    //发送数据
    s->write((char *)&user, sizeof(USE));

    connect(s, SIGNAL(readyRead()),
            this, SLOT(slotRecv()));
}

