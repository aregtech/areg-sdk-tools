#include "sioverview.hpp"
#include "ui_sioverview.h"

sioverview::sioverview(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::sioverview)
{
    ui->setupUi(this);
}

sioverview::~sioverview()
{
    delete ui;
}
