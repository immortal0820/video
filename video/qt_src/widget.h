#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QResizeEvent>
#include <QTcpSocket>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

protected slots:
    void slotLogin();
    void slotRegister();
    void slotRecv();
    void resizeEvent(QResizeEvent *event);
    void slotConned();
    void slotAccept();

private:
    Ui::Widget *ui;
    QTcpSocket *s;
};

#endif // WIDGET_H
