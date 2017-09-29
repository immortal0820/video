#ifndef LOGININFOR_H
#define LOGININFOR_H

#include <QWidget>
#include <QResizeEvent>

namespace Ui {
class LoginInfor;
}

class LoginInfor : public QWidget
{
    Q_OBJECT

public:
    explicit LoginInfor(QWidget *parent = 0);
    ~LoginInfor();

protected slots:
    void resizeEvent(QResizeEvent *event);

private:
    Ui::LoginInfor *ui;
};

#endif // LOGININFOR_H
