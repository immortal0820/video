#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>
#include <QResizeEvent>
#include <QTcpSocket>

namespace Ui {
class Register;
}

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = 0);
    ~Register();

protected slots:
    void slotRegister();
    void resizeEvent(QResizeEvent *event);
    void slotConned();
    void slotRecv();

private:
    Ui::Register *ui;
    QTcpSocket *s;
};

#endif // REGISTER_H
