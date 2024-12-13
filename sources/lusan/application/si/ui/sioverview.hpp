#ifndef SIOVERVIEW_HPP
#define SIOVERVIEW_HPP

#include <QWidget>

namespace Ui {
class sioverview;
}

class sioverview : public QWidget
{
    Q_OBJECT

public:
    explicit sioverview(QWidget *parent = nullptr);
    ~sioverview();

private:
    Ui::sioverview *ui;
};

#endif // SIOVERVIEW_HPP
