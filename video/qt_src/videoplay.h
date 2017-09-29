#ifndef VIDEOPLAY_H
#define VIDEOPLAY_H

#include <QWidget>
#include <QTcpSocket>
#include <QThread>
#include <QPixmap>
#include <QTimer>

class MyThr : public QThread{
    Q_OBJECT
public:
    explicit MyThr(QObject *parent = Q_NULLPTR);
    ~MyThr();

signals:
    void sigvideoplay(QPixmap);

protected slots:
    void slotclosesocket();

protected:
    void run();

private:
    QTcpSocket *ss;
};

namespace Ui {
class VideoPlay;
}

class VideoPlay : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlay(QWidget *parent = 0);
    ~VideoPlay();

signals:
    void sigclos();

protected slots:
    void slotStart();
    void slotStop();
    void slotConned();
    void slotQtime();

private:
    Ui::VideoPlay *ui;
    QTcpSocket *s;
    MyThr *thr;
    QTimer *timer;
};

#endif // VIDEOPLAY_H
