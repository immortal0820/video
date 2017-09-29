#ifndef INFO_H
#define INFO_H

#include <QWidget>
#include <QResizeEvent>

namespace Ui {
class Info;
}

class Info : public QWidget
{
    Q_OBJECT

public:
    explicit Info(QWidget *parent = 0);
    ~Info();

protected slots:
    void resizeEvent(QResizeEvent *event);

private:
    Ui::Info *ui;
};

#endif // INFO_H
