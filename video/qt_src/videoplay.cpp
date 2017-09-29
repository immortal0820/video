#include "videoplay.h"
#include "ui_videoplay.h"
#include <QHostAddress>
#include <QPixmap>
#include <header.h>
#include <QDebug>
#include <QGuiApplication>
#include <QTime>
#include <QDate>

VideoPlay::VideoPlay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoPlay)
{
    ui->setupUi(this);

    //设置窗口标题
    this->setWindowTitle("video play");

    //设置窗口图标
    QIcon icon(":/pic/youku.jpg");
    this->setWindowIcon(icon);

    //设置按钮状态
    ui->startbtn->setDisabled(true);
    ui->stopbtn->setDisabled(true);

    //设置按钮大小
    ui->startbtn->setFixedSize(QSize(30, 25));
    ui->stopbtn->setFixedSize(QSize(30, 25));

    //设置按钮图标
    ui->startbtn->setIcon(QIcon(":/pic/start.jpg"));
    ui->startbtn->setIconSize(QSize(30, 20));
    ui->stopbtn->setIcon(QIcon(":/pic/stop.jpg"));
    ui->stopbtn->setIconSize(QSize(30, 20));

    //获取窗口的大小
    int w = this->width();
    int h = this->height();

    //设置标签自适应窗口大小
    ui->label->setScaledContents(true);

    //设置标签的大小
    ui->label->resize(w-10, h-10);
    ui->datetime->resize(100, 10);

    //向lable中添加图片
    QPixmap pix(":/pic/heise.jpg");
    ui->label->setPixmap(pix);

    //设置窗口大小
    this->setFixedSize(600, 480);

    //时间
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()),
            this, SLOT(slotQtime()));
    timer->start(1000);

    //网络通信
    s = new QTcpSocket(this);
    s->connectToHost(QHostAddress("192.168.3.10"), 6666);


    connect(s, SIGNAL(connected()),
            this, SLOT(slotConned()));
}


VideoPlay::~VideoPlay()
{
    delete ui;
}

void VideoPlay::slotConned()
{
    ui->startbtn->setEnabled(true);
    ui->stopbtn->setEnabled(true);

    connect(ui->startbtn, SIGNAL(clicked(bool)),
            this, SLOT(slotStart()));
    connect(ui->stopbtn, SIGNAL(clicked(bool)),
            this, SLOT(slotStop()));
}


void VideoPlay::slotStart()
{
    thr = new MyThr;

    connect(thr, SIGNAL(sigvideoplay(QPixmap)),
            ui->label, SLOT(setPixmap(QPixmap)));

    thr->start();//子线程启动
}

void VideoPlay::slotStop()
{
    USE user;

    memset(&user, 0, sizeof(USE));
    user.flag = 4;

    s->write((char *)&user, sizeof(USE));

    disconnect(thr, SIGNAL(sigvideoplay(QPixmap)),
              ui->label, SLOT(setPixmap(QPixmap)));

    connect(ui->stopbtn, SIGNAL(sigclos()),
            thr, SLOT(slotclosesocket()));
    emit sigclos();

    thr->terminate(); //关闭子线程
}

void VideoPlay::slotQtime()
{
    QTime qtimeObj = QTime::currentTime();  //获取当前时间
    QString strtime = qtimeObj.toString("hh:mm::ss"); //设置时间格式
    strtime.prepend(" Time is: "); //在源字符串的开头位置插入另一个字符串
    QDate qdata = QDate::currentDate();  //获取当前日期
    QString strdate = qdata.toString("yyyy MM d ddd"); //设置日期显示格式
    strdate.prepend("Today is: ");
    strdate.append(strtime);

    ui->datetime->setText(strdate);
}

MyThr::MyThr(QObject *parent):
    QThread(parent)
{

}

MyThr::~MyThr()
{

}

void MyThr::slotclosesocket()
{
    ss->close();
    delete ss;
}

void MyThr::run()
{
    USE user;

    memset(&user, 0, sizeof(USE));
    user.flag = 3;

    ss = new QTcpSocket;
    ss->connectToHost(QHostAddress("192.168.3.10"), 6666);
    if(!ss->waitForConnected(30000)) {
         qDebug () << "MyThr connect failed!" << endl;
         return;
    }

    ss->write((char *)&user, sizeof(USE));
    char picsize[20];
    char picbuf[60*1024];
    int readlen = 0, ret = 0;
    unsigned long len = 0;

    while(1) {
        ret = 0;
        readlen = 0;
        len = 0;
        memset(picsize, 0, sizeof(picsize));

        ss->waitForReadyRead(120);
        ss->read(picsize, sizeof(picsize));
        len = atoi(picsize);
        while(readlen < len) {
            ret = ss->read(picbuf+readlen, len-ret);
            if(ret < 0) {
                ss->close();
                return;
            } else if(ret == 0) {
                ss->waitForReadyRead(120);
                continue;
            } else {
                readlen = readlen + ret;
            }
        }

        QPixmap pix;
        pix.loadFromData((uchar *)picbuf, len, "JPEG");
        emit sigvideoplay(pix);
        memset(picbuf, 0, 60*1024);
        QCoreApplication::processEvents();
    }
}

